#include "Utils/IO.h"
#include "../../../LOTS/keys.h"
#include "../../../SHA256/sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <sodium.h>

#define BROKER_ADDRESS "localhost"
#define BROKER_PORT 1883
#define MQTT_TOPIC "lots"
int main() {
    struct mosquitto *mosq = NULL;
    int rc;

    // Inicializa a biblioteca libsodium (necessária para geração de chaves em LOTS/keys.c)
    if (sodium_init() < 0) {
        fprintf(stderr, "Erro: falha ao inicializar libsodium.\n");
        return 1;
    }

    // Inicializa a biblioteca mosquitto
    mosquitto_lib_init();

    // Cria uma nova instância de cliente MQTT
    mosq = mosquitto_new("C_Publisher_Lots", true, NULL);
    if (!mosq) {
        fprintf(stderr, "Erro: Não foi possível criar a instância do Mosquitto.\n");
        mosquitto_lib_cleanup();
        return 1;
    }

    // Conecta ao broker MQTT
    printf("Conectando ao broker MQTT em %s:%d...\n", BROKER_ADDRESS, BROKER_PORT);
    rc = mosquitto_connect(mosq, BROKER_ADDRESS, BROKER_PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Erro ao conectar ao broker: %s\n", mosquitto_strerror(rc));
        fprintf(stderr, "Certifique-se de que o broker Mosquitto está rodando e acessível.\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }
    printf("Conectado com sucesso ao broker!\n\n");

    // Inicia o loop de rede em segundo plano para manter a conexão ativa (keep-alive)
    mosquitto_loop_start(mosq);

    // Geração do par de chaves LOTS
    printf("[LOTS] Gerando par de chaves LOTS para o publicador...\n");
    SecretKeys *sKeys = malloc_Skeys();
    PublicKeys *pKeys = malloc_Pkeys();
    generateSecretKeys(sKeys);
    generatePublicKeys(pKeys, sKeys);
    
    // Salva a chave pública localmente para posterior verificação
    const char *caminhoPkey = "publicKeys.txt";
    FILE *arquivoPkey = fopen(caminhoPkey, "wb");
    if (arquivoPkey) {
        fwrite(pKeys->PK0, sizeof(uint8_t), 256 * KEY_SIZE, arquivoPkey);
        fwrite(pKeys->PK1, sizeof(uint8_t), 256 * KEY_SIZE, arquivoPkey);
        fclose(arquivoPkey);
        printf("[LOTS] Chave pública salva localmente em '%s'.\n\n", caminhoPkey);
    } else {
        printf("Aviso: Não foi possível salvar a chave pública localmente.\n\n");
    }

    printf("Digite as mensagens para assinar e enviar para a placa (pressione Enter para enviar, Ctrl+D para sair):\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        // Lê a mensagem dinamicamente usando lerMensagemStream de IO.c
        char *mensagem = lerMensagemStream(stdin);
        
        // Se retornar NULL (fim de arquivo/EOF ou erro)
        if (mensagem == NULL) {
            break;
        }

        // Se a mensagem não estiver vazia, assina e envia
        if (strlen(mensagem) > 0) {
            printf("\nProcessando Mensagem: '%s'\n", mensagem);
            
            // 1. Gera o hash SHA-256 da mensagem
            uint8_t msgHash[32];
            sha256_bytes(mensagem, strlen(mensagem), msgHash);
            
            // 2. Assina o hash usando as chaves secretas do LOTS
            uint8_t assinatura[256][KEY_SIZE];
            assinarMSG(msgHash, sKeys, assinatura);
            printf("[LOTS] Mensagem assinada! (Assinatura: %d bytes)\n", 256 * KEY_SIZE);
            
            // 3. Empacota a mensagem, a chave pública e a assinatura em um único buffer binário
            size_t tamanho_pacote = 0;
            uint8_t *pacote = empacotarDados(mensagem, (uint8_t*)pKeys, sizeof(PublicKeys), (uint8_t*)assinatura, 256 * KEY_SIZE, &tamanho_pacote);
            
            if (pacote != NULL) {
                // 4. Publica o pacote binário no broker MQTT
                rc = mosquitto_publish(mosq, NULL, MQTT_TOPIC, tamanho_pacote, pacote, 1, false);
                if (rc == MOSQ_ERR_SUCCESS) {
                    printf("[MQTT] Pacote (Mensagem + Chave Pública + Assinatura) publicado com sucesso! (%zu bytes)\n", tamanho_pacote);
                } else {
                    fprintf(stderr, "[MQTT] Erro ao publicar pacote: %s\n", mosquitto_strerror(rc));
                }
                
                // Libera o pacote alocado dinamicamente
                free(pacote);
            } else {
                fprintf(stderr, "Erro ao criar pacote para envio.\n");
            }
            printf("\n");
        }

        // Libera a memória alocada dinamicamente pelo lerMensagemStream
        free(mensagem);
    }

    // Libera a memória das chaves LOTS
    freeKeys(pKeys, sKeys);

    // Desconecta e limpa recursos do Mosquitto
    mosquitto_loop_stop(mosq, true);
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    printf("\nPublisher LOTS encerrado.\n");
    return 0;
}
