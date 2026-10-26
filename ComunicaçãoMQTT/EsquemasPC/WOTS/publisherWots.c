/*
 * publisherWots.c — Comunicação WOTS+ para PC via MQTT
 *
 * Suporta dois modos:
 *   1. Modo Receptor (Padrão):
 *      - Conecta ao broker e assina o tópico "wots/esp_to_pc"
 *      - Recebe pacotes binários enviados pelo ESP32:
 *          [4B tam_msg][msg][4B tam_pkey][pkey][4B tam_sig][sig]
 *      - Desempacota os dados e verifica a assinatura WOTS+
 *
 *   2. Modo Transmissor (--send ou -s):
 *      - Gera seeds e chaves WOTS+
 *      - Lê mensagens do terminal, assina e envia para a ESP32 no tópico "wots"
 *
 * Compilar:  make
 * Executar modo receptor:       ./publisherWots
 * Executar modo transmissor:    ./publisherWots --send
 */

#include "../../../WOTS/keys.h"
#include "../../../WOTS/prf.h"
#include "../../../SHA256/sha256.h"
#include "../../Utils/IO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mosquitto.h>
#include <sodium.h>
#include <signal.h>
#include <stdbool.h>

#define BROKER_ADDRESS  "localhost"
#define BROKER_PORT     1883
#define TOPIC_PC_TO_ESP "wots"
#define TOPIC_ESP_TO_PC "wots/esp_to_pc"

static volatile bool executando = true;

static void tratar_sigint(int sig) {
    (void)sig;
    executando = false;
}

// Callback do Mosquitto para quando uma mensagem for recebida (ESP -> PC)
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    (void)mosq;
    (void)userdata;

    printf("\n======================================================\n");
    printf("[MQTT] Pacote recebido no topico [%s] (%d bytes)\n", message->topic, message->payloadlen);
    printf("======================================================\n");

    char *msg = NULL;
    uint8_t *pkey_bytes = NULL;
    size_t tamanho_pkey = 0;
    uint8_t *sig_bytes = NULL;
    size_t tamanho_sig = 0;

    bool sucesso = desempacotarDados(
        (const uint8_t*)message->payload,
        (size_t)message->payloadlen,
        &msg,
        &pkey_bytes, &tamanho_pkey,
        &sig_bytes, &tamanho_sig
    );

    if (!sucesso) {
        fprintf(stderr, "Erro: Falha ao desempacotar pacote binario WOTS+.\n");
        return;
    }

    printf("Mensagem recebida: \"%s\"\n", msg ? msg : "(vazia)");
    printf("Tamanho Chave Publica: %zu bytes (PublicKeys=%zu, Bundle com Seeds=%zu)\n", tamanho_pkey, sizeof(PublicKeys), 2 * WOTS_N + sizeof(PublicKeys));
    printf("Tamanho Assinatura: %zu bytes (esperado: %zu)\n", tamanho_sig, sizeof(Assinatura));

    if ((tamanho_pkey != sizeof(PublicKeys) && tamanho_pkey != (2 * WOTS_N + sizeof(PublicKeys))) ||
        tamanho_sig != sizeof(Assinatura) || msg == NULL) {
        fprintf(stderr, "Erro: Tamanho de chave publica ou assinatura invalido para WOTS+.\n");
        if (msg) free(msg);
        if (pkey_bytes) free(pkey_bytes);
        if (sig_bytes) free(sig_bytes);
        return;
    }

    // Se o pacote incluiu o PK_seed e SK_seed (formato bundle: [PK_seed (32B)][SK_seed (32B)][PublicKeys (2144B)])
    PublicKeys *pKeys = NULL;
    if (tamanho_pkey == 2 * WOTS_N + sizeof(PublicKeys)) {
        memcpy(PK_seed, pkey_bytes, WOTS_N);
        memcpy(SK_seed, pkey_bytes + WOTS_N, WOTS_N);
        pKeys = (PublicKeys*)(pkey_bytes + 2 * WOTS_N);
        printf("[WOTS+] PK_seed e SK_seed restauradas com sucesso do pacote recebido.\n");
    } else {
        pKeys = (PublicKeys*)pkey_bytes;
    }

    // 1. Gera hash da mensagem
    unsigned char msgHash[WOTS_N];
    sha256_bytes((const uint8_t*)msg, strlen(msg), msgHash);

    // 2. Converte ponteiros para os tipos do WOTS+
    Assinatura *assinatura = (Assinatura*)sig_bytes;

    // 3. Verifica assinatura
    sha256_reset_counter();
    clock_t t0 = clock();
    int valida = verificarMensagem(msgHash, assinatura, pKeys);
    clock_t t1 = clock();
    unsigned long long hashes = sha256_get_counter();

    printf("\n[WOTS+] Resultado da Verificacao no PC: %s\n", valida ? ">>> ASSINATURA VALIDA! <<<" : ">>> ASSINATURA INVALIDA! <<<");
    printf("[WOTS+] Tempo de verificacao: %.4f s\n", (double)(t1 - t0) / CLOCKS_PER_SEC);
    printf("[WOTS+] Hashes calculados na verificacao: %llu\n", hashes);
    printf("======================================================\n\n");
    fflush(stdout);

    // 4. Publica as métricas do verificador PC no tópico "estat/pc"
    double tempo_verif_s = (double)(t1 - t0) / CLOCKS_PER_SEC;
    uint32_t tempo_verif_us = (uint32_t)(tempo_verif_s * 1000000.0);

    char metricas_json[256];
    snprintf(metricas_json, sizeof(metricas_json),
             "{\"dispositivo\":\"PC\",\"algoritmo\":\"WOTS\",\"operacao\":\"verificacao\","
             "\"tempo_verificacao_us\":%u,\"numero_hashes\":%llu,\"resultado\":%d}",
             tempo_verif_us, hashes, (valida ? 1 : 0));

    int rc = mosquitto_publish(mosq, NULL, "estat/pc", strlen(metricas_json), metricas_json, 0, false);
    if (rc == MOSQ_ERR_SUCCESS) {
        printf("[WOTS+] Metricas enviadas para 'estat/pc': %s\n", metricas_json);
    } else {
        fprintf(stderr, "[WOTS+] Erro ao enviar metricas para 'estat/pc': %s\n", mosquitto_strerror(rc));
    }
    fflush(stdout);

    // Liberação
    if (msg) free(msg);
    if (pkey_bytes) free(pkey_bytes);
    if (sig_bytes) free(sig_bytes);
}

// Modo Receptor: escuta mensagens vindas da ESP32 e verifica
int executar_receptor(struct mosquitto *mosq) {
    printf("[Modo Receptor WOTS+] Inscrevendo no topico '%s'...\n", TOPIC_ESP_TO_PC);
    int rc = mosquitto_subscribe(mosq, NULL, TOPIC_ESP_TO_PC, 1);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Erro ao assinar topico %s: %s\n", TOPIC_ESP_TO_PC, mosquitto_strerror(rc));
        return 1;
    }

    printf("Aguardando mensagens da ESP32... (Pressione Ctrl+C para sair)\n\n");
    fflush(stdout);

    rc = mosquitto_loop_forever(mosq, -1, 1);
    if (rc != MOSQ_ERR_SUCCESS && rc != MOSQ_ERR_CONN_LOST) {
        fprintf(stderr, "mosquitto_loop_forever: %s\n", mosquitto_strerror(rc));
    }
    return 0;
}

// Modo Transmissor: lê mensagens do teclado, assina e envia para a ESP32
int executar_transmissor(struct mosquitto *mosq) {
    printf("[WOTS] Inicializando seeds...\n");
    initializeSeeds();

    printf("[WOTS] Gerando chaves secretas (SK)...\n");
    SecretKeys *sKeys = mallocSkeys();
    generateSKeys(sKeys);

    printf("[WOTS] Gerando chaves publicas (PK)...\n");
    PublicKeys *pKeys = mallocPkeys();
    generatePKeys(pKeys, sKeys);

    printf("Digite as mensagens para assinar e enviar para a placa (Enter = enviar, Ctrl+D = sair):\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        char *mensagem = lerMensagemStream(stdin);
        if (mensagem == NULL) break;

        if (strlen(mensagem) == 0) {
            free(mensagem);
            continue;
        }

        printf("\nProcessando: '%s'\n", mensagem);

        unsigned char msgHash[WOTS_N];
        sha256_bytes(mensagem, strlen(mensagem), msgHash);

        Assinatura *assinatura = mallocAssinatura();
        assinarMensagem(msgHash, assinatura, sKeys);
        printf("[WOTS] Assinado com sucesso!\n");

        size_t tamanho_pkey_bundle = 2 * WOTS_N + sizeof(PublicKeys);
        uint8_t *pkey_bundle = (uint8_t*)malloc(tamanho_pkey_bundle);
        if (!pkey_bundle) {
            fprintf(stderr, "Erro ao alocar memoria para pkey_bundle.\n");
            free(assinatura);
            free(mensagem);
            continue;
        }
        memcpy(pkey_bundle, PK_seed, WOTS_N);
        memcpy(pkey_bundle + WOTS_N, SK_seed, WOTS_N);
        memcpy(pkey_bundle + 2 * WOTS_N, pKeys, sizeof(PublicKeys));

        size_t tamanho_pacote = 0;
        uint8_t *pacote = empacotarDados(
            mensagem,
            pkey_bundle, tamanho_pkey_bundle,
            (const uint8_t *)assinatura, sizeof(Assinatura),
            &tamanho_pacote
        );
        free(pkey_bundle);

        if (pacote != NULL) {
            int rc = mosquitto_publish(mosq, NULL, TOPIC_PC_TO_ESP, (int)tamanho_pacote, pacote, 1, false);
            if (rc == MOSQ_ERR_SUCCESS) {
                printf("[MQTT] Pacote publicado em '%s' (%zu B)\n", TOPIC_PC_TO_ESP, tamanho_pacote);
            } else {
                fprintf(stderr, "[MQTT] Erro ao publicar: %s\n", mosquitto_strerror(rc));
            }
            free(pacote);
        } else {
            fprintf(stderr, "Erro ao empacotar dados.\n");
        }

        free(assinatura);
        free(mensagem);
        printf("\n");
    }

    free(sKeys);
    free(pKeys);
    return 0;
}

int main(int argc, char *argv[]) {
    bool modo_receptor = true; // Por padrao escuta mensagens da ESP32

    if (argc > 1 && (strcmp(argv[1], "--send") == 0 || strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "send") == 0)) {
        modo_receptor = false;
    }

    if (sodium_init() < 0) {
        fprintf(stderr, "Erro: falha ao inicializar libsodium.\n");
        return 1;
    }

    mosquitto_lib_init();

    struct mosquitto *mosq = mosquitto_new(modo_receptor ? "C_Receiver_Wots" : "C_Publisher_Wots", true, NULL);
    if (!mosq) {
        fprintf(stderr, "Erro: nao foi possivel criar instancia do Mosquitto.\n");
        mosquitto_lib_cleanup();
        return 1;
    }

    if (modo_receptor) {
        mosquitto_message_callback_set(mosq, on_message);
    }

    printf("Conectando ao broker MQTT em %s:%d...\n", BROKER_ADDRESS, BROKER_PORT);
    int rc = mosquitto_connect(mosq, BROKER_ADDRESS, BROKER_PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Erro ao conectar: %s\n", mosquitto_strerror(rc));
        fprintf(stderr, "Verifique se o broker Mosquitto esta rodando.\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }
    printf("Conectado com sucesso!\n\n");

    if (modo_receptor) {
        printf("--- MODO RECEPTOR / VERIFICADOR WOTS+ ATIVADO (padrao) ---\n");
        printf("(Para transmitir mensagens do PC para a ESP32 use: %s --send)\n\n", argv[0]);
        executar_receptor(mosq);
    } else {
        printf("--- MODO TRANSMISSOR PC WOTS+ ATIVADO ---\n\n");
        executar_transmissor(mosq);
    }

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    printf("\nPrograma WOTS encerrado.\n");
    return 0;
}
