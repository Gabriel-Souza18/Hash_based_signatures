#include "keys.h"
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
#define TOPIC_PC_TO_ESP "hors"
#define TOPIC_ESP_TO_PC "hors/esp_to_pc"
#define HORS_PKEY_SIZE ((size_t)HORS_T * HORS_N)
#define HORS_SIGNATURE_SIZE ((size_t)HORS_K * HORS_N)

static void publicarMetricas(struct mosquitto *mosq, const char *operacao,
                             uint32_t tempo_us, unsigned long long hashes,
                             int resultado) {
    char json[320];
    snprintf(json, sizeof(json),
             "{\"dispositivo\":\"PC\",\"algoritmo\":\"HORS\",\"operacao\":\"%s\","
             "\"tempo_verificacao_us\":%u,\"numero_hashes\":%llu,\"resultado\":%d}",
             operacao, tempo_us, hashes, resultado);
    int rc = mosquitto_publish(mosq, NULL, "estat/pc", (int)strlen(json), json, 1, false);
    if (rc != MOSQ_ERR_SUCCESS)
        fprintf(stderr, "[HORS] Erro ao publicar metricas: %s\n", mosquitto_strerror(rc));
}

static void on_message(struct mosquitto *mosq, void *userdata,
                       const struct mosquitto_message *message) {
    (void)userdata;
    char *mensagem = NULL;
    uint8_t *pkey = NULL, *sig_bytes = NULL;
    size_t tamanho_pkey = 0, tamanho_sig = 0;

    bool pacote_valido = desempacotarDados(
        (const uint8_t *)message->payload, (size_t)message->payloadlen,
        &mensagem, &pkey, &tamanho_pkey, &sig_bytes, &tamanho_sig);
    if (!pacote_valido || !mensagem || !pkey || !sig_bytes ||
        tamanho_pkey != HORS_PKEY_SIZE || tamanho_sig != HORS_SIGNATURE_SIZE) {
        fprintf(stderr, "[HORS] Pacote recebido invalido: pkey=%zu, assinatura=%zu.\n",
                tamanho_pkey, tamanho_sig);
        free(mensagem); free(pkey); free(sig_bytes);
        return;
    }

    Keys *keys = (Keys *)calloc(1, sizeof(Keys));
    Assinatura assinatura;
    if (!keys) {
        fprintf(stderr, "[HORS] Falha ao alocar chaves para verificacao.\n");
        free(mensagem); free(pkey); free(sig_bytes);
        return;
    }
    memcpy(keys->PKeys, pkey, HORS_PKEY_SIZE);
    memcpy(&assinatura, sig_bytes, HORS_SIGNATURE_SIZE);

    sha256_reset_counter();
    clock_t inicio = clock();
    int valida = verificarAssinatura(mensagem, (int)strlen(mensagem),
                                     &assinatura, keys->PKeys);
    clock_t fim = clock();
    uint32_t tempo_us = (uint32_t)(((double)(fim - inicio) / CLOCKS_PER_SEC) * 1000000.0);
    unsigned long long hashes = sha256_get_counter();

    printf("[HORS] Mensagem: \"%s\" | verificacao: %s | %u us | %llu hashes\n",
           mensagem, valida ? "valida" : "invalida", tempo_us, hashes);
    publicarMetricas(mosq, "verificacao", tempo_us, hashes, valida);

    free(keys);
    free(mensagem); free(pkey); free(sig_bytes);
}

static int executarReceptor(struct mosquitto *mosq) {
    int rc = mosquitto_subscribe(mosq, NULL, TOPIC_ESP_TO_PC, 1);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "[HORS] Falha ao assinar %s: %s\n",
                TOPIC_ESP_TO_PC, mosquitto_strerror(rc));
        return rc;
    }
    printf("[HORS] Aguardando mensagens da ESP32.\n");
    return mosquitto_loop_forever(mosq, -1, 1);
}

static int executarTransmissor(struct mosquitto *mosq) {
    Keys *keys = (Keys *)malloc(sizeof(Keys));
    Assinatura *assinatura = (Assinatura *)malloc(sizeof(Assinatura));
    if (!keys || !assinatura) {
        fprintf(stderr, "[HORS] Falha ao alocar chaves ou assinatura.\n");
        free(keys); free(assinatura);
        return 1;
    }

    printf("[HORS] Gerando chaves (%zu bytes de chave publica)...\n", HORS_PKEY_SIZE);
    sha256_reset_counter();
    clock_t inicio_keygen = clock();
    gerarKeys(keys);
    clock_t fim_keygen = clock();
    uint32_t tempo_keygen_us = (uint32_t)(((double)(fim_keygen - inicio_keygen) /
                                           CLOCKS_PER_SEC) * 1000000.0);
    unsigned long long hashes_keygen = sha256_get_counter();

    printf("Digite a mensagem para assinar (Ctrl+D para sair):\n");
    for (;;) {
        char *mensagem = lerMensagemStream(stdin);
        if (!mensagem) break;
        if (strlen(mensagem) == 0) { free(mensagem); continue; }

        sha256_reset_counter();
        clock_t inicio = clock();
        assinarMensagem(mensagem, (int)strlen(mensagem), assinatura, keys->SKeys);
        clock_t fim = clock();
        uint32_t tempo_us = (uint32_t)(((double)(fim - inicio) / CLOCKS_PER_SEC) * 1000000.0);
        unsigned long long hashes = sha256_get_counter();

        size_t pacote_size = 0;
        uint8_t *pacote = empacotarDados(
            mensagem, (const uint8_t *)keys->PKeys, HORS_PKEY_SIZE,
            (const uint8_t *)assinatura, HORS_SIGNATURE_SIZE, &pacote_size);
        if (!pacote) {
            fprintf(stderr, "[HORS] Falha ao empacotar assinatura.\n");
        } else {
            int rc = mosquitto_publish(mosq, NULL, TOPIC_PC_TO_ESP,
                                        (int)pacote_size, pacote, 1, false);
            if (rc != MOSQ_ERR_SUCCESS)
                fprintf(stderr, "[HORS] Falha ao publicar: %s\n", mosquitto_strerror(rc));
            else
                printf("[HORS] Pacote publicado em %s (%zu bytes).\n",
                       TOPIC_PC_TO_ESP, pacote_size);
        }

        char json[320];
        snprintf(json, sizeof(json),
                 "{\"dispositivo\":\"PC\",\"algoritmo\":\"HORS\",\"operacao\":\"assinatura\","
                 "\"tempo_keygen_us\":%u,\"tempo_assinatura_us\":%u,"
                 "\"numero_hashes\":%llu,\"tamanho_pkey_bytes\":%zu,"
                 "\"tamanho_assinatura_bytes\":%zu}",
                 tempo_keygen_us, tempo_us, hashes + hashes_keygen,
                 HORS_PKEY_SIZE, HORS_SIGNATURE_SIZE);
        if (mosquitto_publish(mosq, NULL, "estat/pc", (int)strlen(json), json, 1, false)
            != MOSQ_ERR_SUCCESS)
            fprintf(stderr, "[HORS] Falha ao publicar metricas.\n");

        free(pacote);
        free(mensagem);
    }

    free(assinatura);
    free(keys);
    return 0;
}

int main(int argc, char **argv) {
    bool receptor = !(argc > 1 && (!strcmp(argv[1], "--send") ||
                                   !strcmp(argv[1], "-s") || !strcmp(argv[1], "send")));
    if (sodium_init() < 0) return 1;

    mosquitto_lib_init();
    struct mosquitto *mosq = mosquitto_new(
        receptor ? "C_Receiver_HORS" : "C_Publisher_HORS", true, NULL);
    if (!mosq) {
        mosquitto_lib_cleanup();
        return 1;
    }
    if (receptor) mosquitto_message_callback_set(mosq, on_message);

    int rc = mosquitto_connect(mosq, BROKER_ADDRESS, BROKER_PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "[HORS] Falha ao conectar: %s\n", mosquitto_strerror(rc));
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }
    rc = receptor ? executarReceptor(mosq) : executarTransmissor(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return rc == MOSQ_ERR_SUCCESS || rc == MOSQ_ERR_CONN_LOST ? 0 : rc;
}
