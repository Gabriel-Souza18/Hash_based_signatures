#include "Arvore.h"
#include "../../../SHA256/sha256.h"
#include "../../Utils/IO.h"

#include <mosquitto.h>
#include <sodium.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BROKER_ADDRESS "localhost"
#define BROKER_PORT 1883
#define TOPIC_PC_TO_ESP "mss"
#define TOPIC_ESP_TO_PC "mss/esp_to_pc"
#define NUM_FOLHAS 16
#define MSS_WIRE_SIZE (4 * sizeof(uint32_t) + 32 + 32 * 32 + 32 + 64 + sizeof(PublicKeys) + sizeof(Assinatura))

static No *mssTree(Folha *folhas) {
    No *nivel[NUM_FOLHAS];
    int quantidade = NUM_FOLHAS / 2;

    for (int i = 0; i < quantidade; i++) {
        nivel[i] = alocarNo();
        conectarFolhasAoNo(nivel[i], &folhas[2 * i], &folhas[2 * i + 1]);
    }

    while (quantidade > 1) {
        int nova_quantidade = quantidade / 2;
        No **novo_nivel = (No **)malloc((size_t)nova_quantidade * sizeof(No *));
        if (!novo_nivel) return NULL;

        for (int i = 0; i < nova_quantidade; i++) {
            novo_nivel[i] = alocarNo();
            novo_nivel[i]->filho_esq = nivel[2 * i];
            novo_nivel[i]->filho_dir = nivel[2 * i + 1];
            novo_nivel[i]->tipo_filho_esq = TIPO_NO;
            novo_nivel[i]->tipo_filho_dir = TIPO_NO;
            criarPai(novo_nivel[i]);
        }

        memcpy(nivel, novo_nivel, (size_t)nova_quantidade * sizeof(No *));
        free(novo_nivel);
        quantidade = nova_quantidade;
    }

    return nivel[0];
}

static void liberarContextoMss(No *raiz, Folha *folhas) {
    limparArvore(raiz);
    for (int i = 0; i < NUM_FOLHAS; i++) {
        free(folhas[i].Skeys);
        free(folhas[i].Pkeys);
    }
    free(folhas);
}

static bool serializarAssinatura(const AssinaturaMSS *assinatura, uint8_t *saida, size_t capacidade) {
    if (!assinatura || !saida || capacidade < MSS_WIRE_SIZE ||
        assinatura->tamanhoCaminho < 0 || assinatura->tamanhoCaminho > 32 ||
        !assinatura->wotsSignature || !assinatura->folhaPkeys) return false;

    uint8_t *p = saida;
    uint32_t campos[4] = {
        (uint32_t)assinatura->alturaArvore,
        (uint32_t)assinatura->totalFolhas,
        (uint32_t)assinatura->indiceFolha,
        (uint32_t)assinatura->tamanhoCaminho
    };
    memcpy(p, campos, sizeof(campos)); p += sizeof(campos);
    memcpy(p, assinatura->hashFolha, 32); p += 32;
    memcpy(p, assinatura->caminho, 32 * 32); p += 32 * 32;
    memcpy(p, assinatura->caminhoDirecao, 32); p += 32;
    memcpy(p, assinatura->leaf_PK_seed, 32); p += 32;
    memcpy(p, assinatura->leaf_SK_seed, 32); p += 32;
    memcpy(p, assinatura->folhaPkeys, sizeof(PublicKeys)); p += sizeof(PublicKeys);
    memcpy(p, assinatura->wotsSignature, sizeof(Assinatura));
    return true;
}

static bool desempacotarMss(const uint8_t *dados, size_t tamanho, AssinaturaMSS *assinatura) {
    if (!dados || !assinatura || tamanho != MSS_WIRE_SIZE) return false;
    if (!assinatura->folhaPkeys) {
        assinatura->folhaPkeys = mallocPkeys();
        if (!assinatura->folhaPkeys) return false;
    }
    if (!assinatura->wotsSignature) {
        assinatura->wotsSignature = mallocAssinatura();
        if (!assinatura->wotsSignature) return false;
    }

    const uint8_t *p = dados;
    uint32_t campos[4];
    memcpy(campos, p, sizeof(campos)); p += sizeof(campos);
    if (campos[3] > 32 || campos[1] != NUM_FOLHAS || campos[2] >= NUM_FOLHAS) return false;

    assinatura->alturaArvore = (int)campos[0];
    assinatura->totalFolhas = (int)campos[1];
    assinatura->indiceFolha = (int)campos[2];
    assinatura->tamanhoCaminho = (int)campos[3];
    memcpy(assinatura->hashFolha, p, 32); p += 32;
    memcpy(assinatura->caminho, p, 32 * 32); p += 32 * 32;
    memcpy(assinatura->caminhoDirecao, p, 32); p += 32;
    memcpy(assinatura->leaf_PK_seed, p, 32); p += 32;
    memcpy(assinatura->leaf_SK_seed, p, 32); p += 32;
    memcpy(assinatura->folhaPkeys, p, sizeof(PublicKeys)); p += sizeof(PublicKeys);
    memcpy(assinatura->wotsSignature, p, sizeof(Assinatura));
    return true;
}

static void publicarMetricas(struct mosquitto *mosq, bool valida, uint32_t tempo_us,
                             unsigned long long hashes) {
    char json[256];
    snprintf(json, sizeof(json),
             "{\"dispositivo\":\"PC\",\"algoritmo\":\"MSS\",\"operacao\":\"verificacao\","
             "\"tempo_verificacao_us\":%u,\"numero_hashes\":%llu,\"resultado\":%d}",
             tempo_us, hashes, valida ? 1 : 0);
    int rc = mosquitto_publish(mosq, NULL, "estat/pc", (int)strlen(json), json, 1, false);
    if (rc != MOSQ_ERR_SUCCESS)
        fprintf(stderr, "[MSS] Erro ao publicar metricas: %s\n", mosquitto_strerror(rc));
}

static void on_message(struct mosquitto *mosq, void *userdata,
                       const struct mosquitto_message *message) {
    (void)userdata;
    char *mensagem = NULL;
    uint8_t *pkey = NULL, *sig_wire = NULL;
    size_t tamanho_pkey = 0, tamanho_sig = 0;
    AssinaturaMSS *assinatura = NULL;

    if (!desempacotarDados((const uint8_t *)message->payload, (size_t)message->payloadlen,
                           &mensagem, &pkey, &tamanho_pkey, &sig_wire, &tamanho_sig) ||
        !mensagem || tamanho_pkey != 32 || tamanho_sig != MSS_WIRE_SIZE) {
        fprintf(stderr, "[MSS] Pacote recebido invalido.\n");
        goto cleanup;
    }

    assinatura = alocarAssinatura();
    strncpy(assinatura->mensagem, mensagem, sizeof(assinatura->mensagem) - 1);
    if (!desempacotarMss(sig_wire, tamanho_sig, assinatura)) {
        fprintf(stderr, "[MSS] Assinatura MSS serializada invalida.\n");
        goto cleanup;
    }

    sha256_reset_counter();
    clock_t inicio = clock();
    int valida = verificarAssinatura(assinatura, pkey, assinatura->folhaPkeys);
    clock_t fim = clock();
    uint32_t tempo_us = (uint32_t)(((double)(fim - inicio) / CLOCKS_PER_SEC) * 1000000.0);
    unsigned long long hashes = sha256_get_counter();

    printf("[MSS] Mensagem: \"%s\" | verificacao: %s | %u us | %llu hashes\n",
           mensagem, valida ? "valida" : "invalida", tempo_us, hashes);
    publicarMetricas(mosq, valida, tempo_us, hashes);

cleanup:
    if (assinatura) liberarAssinatura(assinatura);
    free(mensagem);
    free(pkey);
    free(sig_wire);
}

static int executarReceptor(struct mosquitto *mosq) {
    int rc = mosquitto_subscribe(mosq, NULL, TOPIC_ESP_TO_PC, 1);
    if (rc != MOSQ_ERR_SUCCESS) return rc;
    return mosquitto_loop_forever(mosq, -1, 1);
}

static int executarTransmissor(struct mosquitto *mosq) {
    printf("Digite a mensagem para assinar (Ctrl+D para sair):\n");
    for (;;) {
        char *mensagem = lerMensagemStream(stdin);
        if (!mensagem) break;
        if (strlen(mensagem) == 0) { free(mensagem); continue; }

        Folha *folhas = (Folha *)calloc(NUM_FOLHAS, sizeof(Folha));
        AssinaturaMSS *assinatura = folhas ? alocarAssinatura() : NULL;
        No *raiz = NULL;
        uint8_t *wire = NULL, *pacote = NULL;
        if (!folhas || !assinatura) {
            fprintf(stderr, "[MSS] Falha ao alocar contexto.\n");
            free(folhas); free(assinatura); free(mensagem); continue;
        }
        for (int i = 0; i < NUM_FOLHAS; i++) {
            folhas[i].Skeys = mallocSkeys();
            folhas[i].Pkeys = mallocPkeys();
        }
        criarFolhas(folhas, NUM_FOLHAS);
        raiz = mssTree(folhas);
        if (!raiz) {
            fprintf(stderr, "[MSS] Falha ao gerar arvore.\n");
            liberarAssinatura(assinatura); liberarContextoMss(NULL, folhas);
            free(mensagem); continue;
        }
        criarAssinatura(assinatura, raiz, &folhas[0], 0, NUM_FOLHAS, mensagem);
        wire = (uint8_t *)malloc(MSS_WIRE_SIZE);
        size_t pacote_tamanho = 0;
        if (wire && serializarAssinatura(assinatura, wire, MSS_WIRE_SIZE))
            pacote = empacotarDados(mensagem, raiz->hash, 32, wire, MSS_WIRE_SIZE, &pacote_tamanho);
        if (!pacote) {
            fprintf(stderr, "[MSS] Falha ao empacotar assinatura.\n");
        } else {
            int rc = mosquitto_publish(mosq, NULL, TOPIC_PC_TO_ESP, (int)pacote_tamanho, pacote, 1, false);
            if (rc != MOSQ_ERR_SUCCESS)
                fprintf(stderr, "[MSS] Falha ao publicar: %s\n", mosquitto_strerror(rc));
        }
        free(pacote); free(wire);
        liberarAssinatura(assinatura);
        liberarContextoMss(raiz, folhas);
        free(mensagem);
    }
    return 0;
}

int main(int argc, char **argv) {
    bool receptor = !(argc > 1 && (!strcmp(argv[1], "--send") ||
                                   !strcmp(argv[1], "-s") || !strcmp(argv[1], "send")));
    if (sodium_init() < 0) return 1;
    mosquitto_lib_init();
    struct mosquitto *mosq = mosquitto_new(receptor ? "C_Receiver_MSS" : "C_Publisher_MSS", true, NULL);
    if (!mosq) return 1;
    if (receptor) mosquitto_message_callback_set(mosq, on_message);
    int rc = mosquitto_connect(mosq, BROKER_ADDRESS, BROKER_PORT, 60);
    if (rc == MOSQ_ERR_SUCCESS)
        rc = receptor ? executarReceptor(mosq) : executarTransmissor(mosq);
    else fprintf(stderr, "[MSS] Falha ao conectar: %s\n", mosquitto_strerror(rc));
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return rc == MOSQ_ERR_SUCCESS || rc == MOSQ_ERR_CONN_LOST ? 0 : 1;
}
