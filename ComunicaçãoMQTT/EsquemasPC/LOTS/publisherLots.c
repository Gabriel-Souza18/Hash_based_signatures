#include "Utils/IO.h"
#include "../../../LOTS/keys.h"
#include "../../../SHA256/sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <sodium.h>
#include <signal.h>

#define BROKER_ADDRESS "localhost"
#define BROKER_PORT 1883
#define TOPIC_PC_TO_ESP "lots"
#define TOPIC_ESP_TO_PC "lots/esp_to_pc"

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
        fprintf(stderr, "Erro: Falha ao desempacotar pacote binario LOTS.\n");
        return;
    }

    printf("Mensagem recebida: \"%s\"\n", msg ? msg : "(vazia)");
    printf("Tamanho Chave Publica: %zu bytes (esperado: %zu)\n", tamanho_pkey, sizeof(PublicKeys));
    printf("Tamanho Assinatura: %zu bytes (esperado: %d)\n", tamanho_sig, 256 * KEY_SIZE);

    if (tamanho_pkey != sizeof(PublicKeys) || tamanho_sig != 256 * KEY_SIZE || msg == NULL) {
        fprintf(stderr, "Erro: Tamanho de chave publica ou assinatura invalido para LOTS.\n");
        if (msg) free(msg);
        if (pkey_bytes) free(pkey_bytes);
        if (sig_bytes) free(sig_bytes);
        return;
    }

    // 1. Gera hash da mensagem
    uint8_t msgHash[32];
    sha256_bytes(msg, strlen(msg), msgHash);

    // 2. Converte ponteiros para os tipos do LOTS
    PublicKeys *pKeys = (PublicKeys*)pkey_bytes;
    uint8_t (*assinatura)[KEY_SIZE] = (uint8_t (*)[KEY_SIZE])sig_bytes;

    // 3. Verifica assinatura
    sha256_reset_counter();
    clock_t t0 = clock();
    bool valida = verificarMSG(msgHash, pKeys, assinatura);
    clock_t t1 = clock();
    double tempo_verif_s = (double)(t1 - t0) / CLOCKS_PER_SEC;
    uint32_t tempo_verif_us = (uint32_t)(tempo_verif_s * 1000000.0);
    unsigned long long hashes = sha256_get_counter();

    printf("\n[LOTS] Resultado da Verificacao no PC: %s\n", valida ? ">>> ASSINATURA VALIDA! <<<" : ">>> ASSINATURA INVALIDA! <<<");
    printf("[LOTS] Tempo de verificacao: %.6f s (%u us)\n", tempo_verif_s, tempo_verif_us);
    printf("[LOTS] Hashes calculados na verificacao: %llu\n", hashes);
    printf("======================================================\n\n");
    fflush(stdout);

    // 4. Publica as métricas do verificador PC no tópico "estat/pc"
    char metricas_json[256];
    snprintf(metricas_json, sizeof(metricas_json),
             "{\"dispositivo\":\"PC\",\"algoritmo\":\"LOTS\",\"operacao\":\"verificacao\","
             "\"tempo_verificacao_us\":%u,\"numero_hashes\":%llu,\"resultado\":%d}",
             tempo_verif_us, hashes, (valida ? 1 : 0));

    int rc = mosquitto_publish(mosq, NULL, "estat/pc", strlen(metricas_json), metricas_json, 0, false);
    if (rc == MOSQ_ERR_SUCCESS) {
        printf("[LOTS] Metricas enviadas para 'estat/pc': %s\n", metricas_json);
    } else {
        fprintf(stderr, "[LOTS] Erro ao enviar metricas para 'estat/pc': %s\n", mosquitto_strerror(rc));
    }
    fflush(stdout);

    // Liberação
    if (msg) free(msg);
    if (pkey_bytes) free(pkey_bytes);
    if (sig_bytes) free(sig_bytes);
}

// Modo Receptor: escuta mensagens vindas da ESP32 e verifica
int executar_receptor(struct mosquitto *mosq) {
    printf("[Modo Receptor LOTS] Inscrevendo no topico '%s'...\n", TOPIC_ESP_TO_PC);
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
    printf("[LOTS] Gerando par de chaves LOTS para o publicador...\n");
    SecretKeys *sKeys = malloc_Skeys();
    PublicKeys *pKeys = malloc_Pkeys();
    generateSecretKeys(sKeys);
    generatePublicKeys(pKeys, sKeys);

    const char *caminhoPkey = "publicKeys.txt";
    FILE *arquivoPkey = fopen(caminhoPkey, "wb");
    if (arquivoPkey) {
        fwrite(pKeys->PK0, sizeof(uint8_t), 256 * KEY_SIZE, arquivoPkey);
        fwrite(pKeys->PK1, sizeof(uint8_t), 256 * KEY_SIZE, arquivoPkey);
        fclose(arquivoPkey);
        printf("[LOTS] Chave pública salva localmente em '%s'.\n\n", caminhoPkey);
    }

    printf("Digite as mensagens para assinar e enviar para a placa (pressione Enter para enviar, Ctrl+D para sair):\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        char *mensagem = lerMensagemStream(stdin);
        if (mensagem == NULL) {
            break;
        }

        if (strlen(mensagem) > 0) {
            printf("\nProcessando Mensagem: '%s'\n", mensagem);

            uint8_t msgHash[32];
            sha256_bytes(mensagem, strlen(mensagem), msgHash);

            uint8_t assinatura[256][KEY_SIZE];
            assinarMSG(msgHash, sKeys, assinatura);
            printf("[LOTS] Mensagem assinada! (Assinatura: %d bytes)\n", 256 * KEY_SIZE);

            size_t tamanho_pacote = 0;
            uint8_t *pacote = empacotarDados(mensagem, (uint8_t*)pKeys, sizeof(PublicKeys), (uint8_t*)assinatura, 256 * KEY_SIZE, &tamanho_pacote);

            if (pacote != NULL) {
                int rc = mosquitto_publish(mosq, NULL, TOPIC_PC_TO_ESP, tamanho_pacote, pacote, 0, false);
                if (rc == MOSQ_ERR_SUCCESS) {
                    printf("[MQTT] Pacote publicado com sucesso em '%s'! (%zu bytes)\n", TOPIC_PC_TO_ESP, tamanho_pacote);
                } else {
                    fprintf(stderr, "[MQTT] Erro ao publicar pacote: %s\n", mosquitto_strerror(rc));
                }
                free(pacote);
            } else {
                fprintf(stderr, "Erro ao criar pacote para envio.\n");
            }
            printf("\n");
        }
        free(mensagem);
    }

    freeKeys(pKeys, sKeys);
    return 0;
}

int main(int argc, char *argv[]) {
    bool modo_receptor = true; // Por padrao agora escuta a ESP32!

    // Se o usuario passar --send ou -s, ativa o modo transmissor interativo original
    if (argc > 1 && (strcmp(argv[1], "--send") == 0 || strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "send") == 0)) {
        modo_receptor = false;
    }

    if (sodium_init() < 0) {
        fprintf(stderr, "Erro: falha ao inicializar libsodium.\n");
        return 1;
    }

    mosquitto_lib_init();

    struct mosquitto *mosq = mosquitto_new(modo_receptor ? "C_Receiver_Lots" : "C_Publisher_Lots", true, NULL);
    if (!mosq) {
        fprintf(stderr, "Erro: Não foi possível criar a instância do Mosquitto.\n");
        mosquitto_lib_cleanup();
        return 1;
    }

    if (modo_receptor) {
        mosquitto_message_callback_set(mosq, on_message);
    }

    printf("Conectando ao broker MQTT em %s:%d...\n", BROKER_ADDRESS, BROKER_PORT);
    int rc = mosquitto_connect(mosq, BROKER_ADDRESS, BROKER_PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Erro ao conectar ao broker: %s\n", mosquitto_strerror(rc));
        fprintf(stderr, "Certifique-se de que o broker Mosquitto está rodando e acessível.\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }
    printf("Conectado com sucesso ao broker!\n\n");

    if (modo_receptor) {
        printf("--- MODO RECEPTOR / VERIFICADOR ATIVADO (padrao) ---\n");
        printf("(Para transmitir mensagens do PC para a ESP32 use: %s --send)\n\n", argv[0]);
        executar_receptor(mosq);
    } else {
        printf("--- MODO TRANSMISSOR PC ATIVADO ---\n\n");
        executar_transmissor(mosq);
    }

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    printf("\nPrograma LOTS encerrado.\n");
    return 0;
}
