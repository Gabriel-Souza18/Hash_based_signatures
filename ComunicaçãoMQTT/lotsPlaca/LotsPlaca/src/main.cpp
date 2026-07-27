#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "mbedtls/sha256.h"

// Configuração da Rede e Broker
const char* ssid = "Gabriel";
const char* password = "@tamara12";
const char* broker = "192.168.3.105"; // IP do PC rodando o broker Mosquitto
const int porta = 1883;

const int LED_PIN = 2;
const int KEY_SIZE = 32;

WiFiClient espClient;
PubSubClient client(espClient);

extern "C" {
#include "keys.h"
#include "sha256.h"
}

// Callback do MQTT para receber e processar o pacote
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("\n[MQTT] Pacote recebido no topico [");
    Serial.print(topic);
    Serial.print("] com ");
    Serial.print(length);
    Serial.println(" bytes.");

    // O pacote mínimo precisa conter pelo menos os 3 inteiros de tamanho (3 * 4 = 12 bytes)
    if (length < 12) {
        Serial.println("Erro: Pacote muito curto.");
        return;
    }

    const byte *ptr = payload;

    // 1. Extrai o tamanho da mensagem (4 bytes)
    uint32_t tamanho_msg;
    memcpy(&tamanho_msg, ptr, 4);
    ptr += 4;

    if (4 + tamanho_msg + 4 > length) {
        Serial.println("Erro: Pacote malformado (tamanho da mensagem excede limite).");
        return;
    }

    // Aloca e extrai a mensagem
    char *mensagem = (char*)malloc(tamanho_msg + 1);
    if (mensagem == NULL) {
        Serial.println("Erro de memoria ao alocar mensagem.");
        return;
    }
    memcpy(mensagem, ptr, tamanho_msg);
    mensagem[tamanho_msg] = '\0';
    ptr += tamanho_msg;

    // 2. Extrai o tamanho da chave pública (4 bytes)
    uint32_t tamanho_pkey;
    memcpy(&tamanho_pkey, ptr, 4);
    ptr += 4;

    if (4 + tamanho_msg + 4 + tamanho_pkey + 4 > length) {
        Serial.println("Erro: Pacote malformado (tamanho da pkey excede limite).");
        free(mensagem);
        return;
    }

    // Aloca e extrai a chave pública
    PublicKeys *pKeys = (PublicKeys*)malloc(tamanho_pkey);
    if (pKeys == NULL) {
        Serial.println("Erro de memoria ao alocar chave publica.");
        free(mensagem);
        return;
    }
    memcpy(pKeys, ptr, tamanho_pkey);
    ptr += tamanho_pkey;

    // 3. Extrai o tamanho da assinatura (4 bytes)
    uint32_t tamanho_assinatura;
    memcpy(&tamanho_assinatura, ptr, 4);
    ptr += 4;

    if (4 + tamanho_msg + 4 + tamanho_pkey + 4 + tamanho_assinatura != length) {
        Serial.println("Erro: Tamanho total do pacote nao condiz com a assinatura.");
        free(mensagem);
        free(pKeys);
        return;
    }

    // Aloca e extrai a assinatura
    uint8_t (*assinatura)[KEY_SIZE] = (uint8_t (*)[KEY_SIZE])malloc(tamanho_assinatura);
    if (assinatura == NULL) {
        Serial.println("Erro de memoria ao alocar assinatura.");
        free(mensagem);
        free(pKeys);
        return;
    }
    memcpy(assinatura, ptr, tamanho_assinatura);

    // Exibe dados básicos
    Serial.print("Mensagem: \"");
    Serial.print(mensagem);
    Serial.println("\"");
    Serial.print("Tamanho Chave Publica: ");
    Serial.print(tamanho_pkey);
    Serial.println(" bytes.");
    Serial.print("Tamanho Assinatura: ");
    Serial.print(tamanho_assinatura);
    Serial.println(" bytes.");

    // 4. Gera hash SHA-256 local da mensagem recebida
    uint8_t msgHash[32];
    sha256_bytes((const uint8_t*)mensagem, strlen(mensagem), msgHash);

    // 5. Verifica a assinatura
    Serial.println("[LOTS] Iniciando verificacao da assinatura...");
    bool resultado = verificarMSG(msgHash, pKeys, assinatura);

    if (resultado) {
        Serial.println("[LOTS] ASSINATURA VALIDA!");
        // Pisca o LED 3 vezes rapidamente
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(100);
            digitalWrite(LED_PIN, LOW);
            delay(100);
        }
    } else {
        Serial.println("[LOTS] ASSINATURA INVALIDA!");
        // Deixa o LED aceso por 2 segundos para indicar erro
        digitalWrite(LED_PIN, HIGH);
        delay(2000);
        digitalWrite(LED_PIN, LOW);
    }

    // Libera buffers
    free(mensagem);
    free(pKeys);
    free(assinatura);
}

void reconectar() {
    while (!client.connected()) {
        Serial.print("Tentando conectar ao broker MQTT...");
        if (client.connect("ESP32_LOTS_Receiver")) {
            client.subscribe("lots");
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
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nInicializando receptor LOTS...");
    pinMode(LED_PIN, OUTPUT);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi conectado!");

    client.setServer(broker, porta);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconectar();
    }
    client.loop();
}