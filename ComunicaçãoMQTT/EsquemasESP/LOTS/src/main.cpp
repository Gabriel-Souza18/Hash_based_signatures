#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <sodium.h>

// Configuração da Rede e Broker
const char* ssid = "Gabriel";
const char* password = "@tamara12";
const char* broker = "192.168.3.105"; // IP do PC rodando o broker Mosquitto
const int porta = 1883;

const int LED_PIN = 2;
const char* TOPICO_ENVIAR = "lots/esp_to_pc";
const char* MENSAGEM_PADRAO = "Mensagem pré-pronta assinada pelo ESP32 (LOTS)";

WiFiClient espClient;
PubSubClient client(espClient);

extern "C" {
#include "keys.h"
#include "sha256.h"
}

// Empacota mensagem, chave pública e assinatura no formato padrão:
// [4B tam_msg][msg][4B tam_pk][pk][4B tam_sig][sig]
uint8_t* empacotarDadosESP(const char *mensagem, const uint8_t *pkey, size_t tamanho_pkey, const uint8_t *assinatura, size_t tamanho_assinatura, size_t *tamanho_pacote) {
    if (tamanho_pacote == NULL) return NULL;

    size_t tamanho_msg = mensagem ? strlen(mensagem) : 0;
    size_t t_pk = (pkey && tamanho_pkey > 0) ? tamanho_pkey : 0;
    size_t t_as = (assinatura && tamanho_assinatura > 0) ? tamanho_assinatura : 0;

    *tamanho_pacote = sizeof(uint32_t) + tamanho_msg +
                      sizeof(uint32_t) + t_pk +
                      sizeof(uint32_t) + t_as;

    uint8_t *pacote = (uint8_t*)malloc(*tamanho_pacote);
    if (pacote == NULL) return NULL;

    uint8_t *ptr = pacote;

    // Grava Mensagem
    uint32_t t_msg_32 = (uint32_t)tamanho_msg;
    memcpy(ptr, &t_msg_32, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    if (tamanho_msg > 0) {
        memcpy(ptr, mensagem, tamanho_msg);
        ptr += tamanho_msg;
    }

    // Grava Chave Pública
    uint32_t t_pk_32 = (uint32_t)t_pk;
    memcpy(ptr, &t_pk_32, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    if (t_pk > 0) {
        memcpy(ptr, pkey, t_pk);
        ptr += t_pk;
    }

    // Grava Assinatura
    uint32_t t_as_32 = (uint32_t)t_as;
    memcpy(ptr, &t_as_32, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    if (t_as > 0) {
        memcpy(ptr, assinatura, t_as);
    }

    return pacote;
}

// Gera chaves LOTS, assina a mensagem pré-pronta e publica via MQTT
bool gerarEEnviarAssinaturaLOTS() {
    Serial.println("\n[LOTS] Iniciando geracao de chaves, assinatura e envio pelo ESP32...");

    uint32_t heap_antes = ESP.getFreeHeap();
    Serial.print("[LOTS] Free Heap antes: ");
    Serial.print(heap_antes);
    Serial.println(" bytes.");

    // 1. Aloca e gera par de chaves
    SecretKeys *sKeys = malloc_Skeys();
    PublicKeys *pKeys = malloc_Pkeys();
    if (!sKeys || !pKeys) {
        Serial.println("[LOTS] Erro de memoria ao alocar chaves.");
        if (sKeys) free(sKeys);
        if (pKeys) free(pKeys);
        return false;
    }

    Serial.println("[LOTS] Gerando chaves secretas e publicas no ESP32...");
    uint32_t t0 = micros();
    generateSecretKeys(sKeys);
    generatePublicKeys(pKeys, sKeys);
    uint32_t t_keygen = micros() - t0;
    Serial.print("[LOTS] Chaves geradas em ");
    Serial.print(t_keygen / 1000.0);
    Serial.println(" ms.");

    // 2. Calcula hash SHA-256 da mensagem pré-pronta
    const char *msg = MENSAGEM_PADRAO;
    Serial.print("[LOTS] Mensagem a assinar: \"");
    Serial.print(msg);
    Serial.println("\"");

    uint8_t msgHash[32];
    sha256_bytes((const uint8_t*)msg, strlen(msg), msgHash);

    // 3. Assina a mensagem
    uint8_t (*assinatura)[KEY_SIZE] = (uint8_t (*)[KEY_SIZE])malloc(256 * KEY_SIZE);
    if (!assinatura) {
        Serial.println("[LOTS] Erro ao alocar memoria para assinatura.");
        freeKeys(pKeys, sKeys);
        return false;
    }

    t0 = micros();
    assinarMSG(msgHash, sKeys, assinatura);
    uint32_t t_sign = micros() - t0;
    Serial.print("[LOTS] Mensagem assinada em ");
    Serial.print(t_sign / 1000.0);
    Serial.println(" ms.");

    // Calcula uso de RAM no Heap alocado para a operação
    uint32_t heap_depois = ESP.getFreeHeap();
    uint32_t ram_alocada = (heap_antes > heap_depois) ? (heap_antes - heap_depois) : 0;

    // 4. Publica as métricas do transmissor ESP32 no tópico "estat/esp"
    char metricas_json[256];
    snprintf(metricas_json, sizeof(metricas_json),
             "{\"dispositivo\":\"ESP32\",\"algoritmo\":\"LOTS\",\"operacao\":\"assinatura\","
             "\"tempo_keygen_us\":%u,\"tempo_assinatura_us\":%u,"
             "\"tamanho_pkey_bytes\":%u,\"tamanho_assinatura_bytes\":%u,"
             "\"uso_ram_bytes\":%u}",
             t_keygen, t_sign, (unsigned int)sizeof(PublicKeys), (unsigned int)(256 * KEY_SIZE), ram_alocada);

    if (client.publish("estat/esp", metricas_json)) {
        Serial.print("[LOTS] Metricas enviadas para 'estat/esp': ");
        Serial.println(metricas_json);
    } else {
        Serial.println("[LOTS] Falha ao enviar metricas para 'estat/esp'.");
    }

    // 5. Empacota
    size_t tamanho_pacote = 0;
    uint8_t *pacote = empacotarDadosESP(
        msg,
        (const uint8_t*)pKeys, sizeof(PublicKeys),
        (const uint8_t*)assinatura, 256 * KEY_SIZE,
        &tamanho_pacote
    );

    bool enviado = false;
    if (pacote) {
        Serial.print("[LOTS] Publicando pacote (");
        Serial.print(tamanho_pacote);
        Serial.print(" bytes) no topico [");
        Serial.print(TOPICO_ENVIAR);
        Serial.println("]...");

        enviado = client.publish(TOPICO_ENVIAR, pacote, tamanho_pacote);
        if (enviado) {
            Serial.println("[LOTS] Pacote publicado com sucesso pelo ESP32!");
            // Pisca o LED 2 vezes para indicar sucesso no envio
            for (int i = 0; i < 2; i++) {
                digitalWrite(LED_PIN, HIGH);
                delay(150);
                digitalWrite(LED_PIN, LOW);
                delay(150);
            }
        } else {
            Serial.println("[LOTS] Falha ao publicar pacote MQTT.");
        }
        free(pacote);
    } else {
        Serial.println("[LOTS] Erro ao empacotar pacote.");
    }

    // Limpeza de recursos
    free(assinatura);
    freeKeys(pKeys, sKeys);

    return enviado;
}

void reconectar() {
    while (!client.connected()) {
        Serial.print("Tentando conectar ao broker MQTT...");
        if (client.connect("ESP32_LOTS_Transmitter")) {
            Serial.println(" conectado!");
        } else {
            Serial.print(" falhou, rc=");
            Serial.print(client.state());
            Serial.println(". Tentando novamente em 5 segundos...");
            delay(5000);
        }
    }
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Desabilita o detector de Brownout
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n========================================");
    Serial.println("Inicializando transmissor LOTS (ESP32)...");
    Serial.println("========================================");
    pinMode(LED_PIN, OUTPUT);

    // Inicializa libsodium na ESP32 (usado por generateSecretKeys)
    if (sodium_init() < 0) {
        Serial.println("Erro critico: falha ao inicializar libsodium na ESP32.");
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi conectado!");
    Serial.print("IP da ESP32: ");
    Serial.println(WiFi.localIP());

    client.setServer(broker, porta);
    client.setBufferSize(30000); // Garante buffer suficiente para o pacote LOTS (~24KB)
}

void loop() {
    if (!client.connected()) {
        reconectar();
    }
    client.loop();

    // Gera e envia a cada 10 segundos
    static unsigned long ultimo_envio = 0;
    if (millis() - ultimo_envio > 10000 || ultimo_envio == 0) {
        ultimo_envio = millis();
        gerarEEnviarAssinaturaLOTS();
    }
}