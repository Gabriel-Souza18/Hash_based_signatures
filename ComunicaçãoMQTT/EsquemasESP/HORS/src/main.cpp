#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

extern "C" {
#include "keys.h"
#include "sha256.h"
}

#define HORS_PKEY_SIZE ((size_t)HORS_T * HORS_N)
#define HORS_SIGNATURE_SIZE ((size_t)HORS_K * HORS_N)
#define HORS_PACKET_SIZE (12U + 64U + HORS_PKEY_SIZE + HORS_SIGNATURE_SIZE)

const char *ssid = "Gabriel";
const char *password = "@tamara12";
const char *broker = "192.168.3.105";
const int porta = 1883;
const int BOTAO_PIN = 34;
const int LED_VERDE = 14;
const int LED_VERMELHO = 12;
const char *TOPICO_ENVIAR = "hors/esp_to_pc";
const char *MENSAGEM_PADRAO = "Mensagem assinada pelo ESP32 (HORS)";

WiFiClient espClient;
PubSubClient client(espClient);

static bool botaoPressionado() {
    return digitalRead(BOTAO_PIN) == HIGH;
}

static uint8_t *empacotar(const char *mensagem, const uint8_t *pkey,
                          size_t tamanho_pkey, const uint8_t *assinatura,
                          size_t tamanho_assinatura, size_t *tamanho_pacote) {
    size_t tamanho_msg = strlen(mensagem);
    *tamanho_pacote = 12 + tamanho_msg + tamanho_pkey + tamanho_assinatura;
    uint8_t *pacote = (uint8_t *)malloc(*tamanho_pacote);
    if (!pacote) return NULL;

    uint8_t *p = pacote;
    uint32_t tamanho = (uint32_t)tamanho_msg;
    memcpy(p, &tamanho, 4); p += 4;
    memcpy(p, mensagem, tamanho_msg); p += tamanho_msg;
    tamanho = (uint32_t)tamanho_pkey;
    memcpy(p, &tamanho, 4); p += 4;
    memcpy(p, pkey, tamanho_pkey); p += tamanho_pkey;
    tamanho = (uint32_t)tamanho_assinatura;
    memcpy(p, &tamanho, 4); p += 4;
    memcpy(p, assinatura, tamanho_assinatura);
    return pacote;
}

static bool gerarEEnviar() {
    Serial.println();
    Serial.println("========== HORS: INICIO ==========");
    Serial.printf("[HORS] T=%d, K=%d, chave publica=%u bytes, assinatura=%u bytes.\n",
                  HORS_T, HORS_K, (unsigned int)HORS_PKEY_SIZE,
                  (unsigned int)HORS_SIGNATURE_SIZE);
    Serial.printf("[HORS] Pacote estimado: %u bytes.\n", (unsigned int)HORS_PACKET_SIZE);
    uint32_t heap_antes = ESP.getFreeHeap();
    Serial.printf("[HORS] Heap inicial: %u bytes.\n", heap_antes);

    Keys *keys = (Keys *)malloc(sizeof(Keys));
    Assinatura *assinatura = (Assinatura *)malloc(sizeof(Assinatura));
    uint8_t *pacote = NULL;
    size_t pacote_size = 0;
    uint32_t inicio_keygen = 0;
    uint32_t tempo_keygen = 0;
    uint32_t inicio_sign = 0;
    uint32_t tempo_sign = 0;
    unsigned long long hashes_keygen = 0;
    unsigned long long hashes_sign = 0;
    char metricas[360];
    uint32_t uso_ram = 0;
    bool enviado = false;
    if (!keys || !assinatura) {
        Serial.println("[HORS][ERRO] Falha ao alocar chaves ou assinatura.");
        goto cleanup;
    }

    sha256_reset_counter();
    inicio_keygen = micros();
    gerarKeys(keys);
    tempo_keygen = micros() - inicio_keygen;
    hashes_keygen = sha256_get_counter();
    Serial.printf("[HORS] Chaves geradas em %u us; hashes=%llu.\n",
                  tempo_keygen, hashes_keygen);

    sha256_reset_counter();
    inicio_sign = micros();
    assinarMensagem(MENSAGEM_PADRAO, strlen(MENSAGEM_PADRAO),
                    assinatura, keys->SKeys);
    tempo_sign = micros() - inicio_sign;
    hashes_sign = sha256_get_counter();
    Serial.printf("[HORS] Assinatura gerada em %u us; hashes=%llu.\n",
                  tempo_sign, hashes_sign);

    pacote = empacotar(MENSAGEM_PADRAO, (const uint8_t *)keys->PKeys,
                       HORS_PKEY_SIZE, (const uint8_t *)assinatura,
                       HORS_SIGNATURE_SIZE, &pacote_size);
    if (!pacote) {
        Serial.println("[HORS][ERRO] Falha ao alocar pacote MQTT.");
        goto cleanup;
    }
    Serial.printf("[HORS] Pacote pronto: %u bytes.\n", (unsigned int)pacote_size);
    uso_ram = heap_antes > ESP.getFreeHeap() ? heap_antes - ESP.getFreeHeap() : 0;

    snprintf(metricas, sizeof(metricas),
             "{\"dispositivo\":\"ESP32\",\"algoritmo\":\"HORS\",\"operacao\":\"assinatura\","
             "\"tempo_keygen_us\":%u,\"tempo_assinatura_us\":%u,"
             "\"numero_hashes\":%llu,\"tamanho_pkey_bytes\":%u,"
             "\"tamanho_assinatura_bytes\":%u,\"uso_ram_bytes\":%u}",
             tempo_keygen, tempo_sign, hashes_keygen + hashes_sign,
             (unsigned int)HORS_PKEY_SIZE, (unsigned int)HORS_SIGNATURE_SIZE,
             (unsigned int)uso_ram);
    if (!client.publish("estat/esp", metricas)) {
        Serial.println("[HORS][ERRO] Falha ao publicar metricas.");
        goto cleanup;
    }
    Serial.println("[HORS] Metricas publicadas em estat/esp.");
    enviado = client.publish(TOPICO_ENVIAR, pacote, (unsigned int)pacote_size);
    Serial.printf("[HORS] Publicacao do pacote: %s.\n", enviado ? "SUCESSO" : "FALHA");

cleanup:
    if (enviado) {
        digitalWrite(LED_VERDE, HIGH); delay(500); digitalWrite(LED_VERDE, LOW);
    } else {
        digitalWrite(LED_VERMELHO, HIGH); delay(500); digitalWrite(LED_VERMELHO, LOW);
    }
    free(pacote);
    free(assinatura);
    free(keys);
    Serial.printf("[HORS] Heap final: %u bytes.\n", ESP.getFreeHeap());
    Serial.println("=========== HORS: FIM ===========");
    return enviado;
}

static void reconectar() {
    while (!client.connected()) {
        Serial.printf("[MQTT] Conectando ao broker %s:%d...\n", broker, porta);
        if (client.connect("ESP32_HORS_Transmitter")) {
            Serial.println("[MQTT] Conectado ao broker.");
            return;
        }
        Serial.printf("[MQTT][ERRO] Estado=%d; nova tentativa em 5 s.\n", client.state());
        delay(5000);
    }
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    pinMode(BOTAO_PIN, INPUT);
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_VERMELHO, OUTPUT);
    Serial.println("\n===== Inicializando publisher HORS =====");
    Serial.println("[HORS] GPIO34 requer resistor externo de 10k para GND.");
    WiFi.begin(ssid, password);
    Serial.printf("[WIFI] Conectando a rede \"%s\"...\n", ssid);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.printf("\n[WIFI] Conectado. IP: %s, RSSI: %d dBm.\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
    client.setServer(broker, porta);
    client.setBufferSize(40000);
    Serial.println("[HORS] Inicializacao concluida. Pressione o botao para assinar.");
}

void loop() {
    if (!client.connected()) reconectar();
    client.loop();

    static bool anterior = false;
    static bool aguardando_liberacao = false;
    static unsigned long borda = 0;
    bool atual = botaoPressionado();
    if (atual && !anterior) {
        borda = millis();
        Serial.println("[HORS] Botao pressionado; iniciando debounce.");
    }
    if (atual && anterior && !aguardando_liberacao && millis() - borda >= 50) {
        Serial.println("[HORS] Debounce confirmado; iniciando assinatura.");
        gerarEEnviar();
        aguardando_liberacao = true;
    }
    if (aguardando_liberacao && !atual) {
        Serial.println("[HORS] Botao liberado.");
        aguardando_liberacao = false;
    }
    anterior = atual;
}
