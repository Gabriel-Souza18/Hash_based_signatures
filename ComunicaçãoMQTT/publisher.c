#include "Utils/IO.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>

#define BROKER_ADDRESS "localhost"
#define BROKER_PORT 1883
#define MQTT_TOPIC "teste/esp"

int main() {
    struct mosquitto *mosq = NULL;
    int rc;

    // Inicializa a biblioteca mosquitto
    mosquitto_lib_init();

    // Cria uma nova instância de cliente MQTT
    mosq = mosquitto_new("C_Publisher", true, NULL);
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

    printf("Digite as mensagens a enviar para a placa (pressione Enter para enviar, Ctrl+D ou Ctrl+C para sair):\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        // Lê a mensagem dinamicamente usando a função implementada em IO.c
        char *mensagem = lerMensagemStream(stdin);
        
        // Se retornar NULL (fim de arquivo/EOF ou erro)
        if (mensagem == NULL) {
            break;
        }

        // Se a mensagem não estiver vazia, publica no broker MQTT
        if (strlen(mensagem) > 0) {
            rc = mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(mensagem), mensagem, 0, false);
            if (rc == MOSQ_ERR_SUCCESS) {
                printf("[MQTT] Enviado com sucesso: %s\n", mensagem);
            } else {
                fprintf(stderr, "[MQTT] Erro ao publicar mensagem: %s\n", mosquitto_strerror(rc));
            }
        }

        // Libera a memória alocada dinamicamente pelo lerMensagemStream para evitar vazamentos de memória
        free(mensagem);
    }

    // Desconecta e limpa recursos
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    printf("\nPublisher encerrado.\n");
    return 0;
}
