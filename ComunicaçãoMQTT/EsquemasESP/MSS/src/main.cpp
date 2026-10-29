#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

extern "C" {
#include "Arvore.h"
#include "sha256.h"
}

#define NUM_FOLHAS 16
#define MSS_WIRE_SIZE (4 * sizeof(uint32_t) + 32 + 32 * 32 + 32 + 64 + sizeof(PublicKeys) + sizeof(Assinatura))

const char *ssid = "Gabriel";
const char *password = "@tamara12";
const char *broker = "192.168.3.105";
const int porta = 1883;
const int BOTAO_PIN = 34;
const int LED_VERDE = 14;
const int LED_VERMELHO = 12;
const char *TOPICO_ENVIAR = "mss/esp_to_pc";
const char *MENSAGEM_PADRAO = "Mensagem assinada pelo ESP32 (MSS)";

WiFiClient espClient;
PubSubClient client(espClient);

static bool botaoPressionado() {
    return digitalRead(BOTAO_PIN) == HIGH;
}

static No *mssTree(Folha *folhas) {
    Serial.println("[MSS][ARVORE] Iniciando construcao da arvore.");
    No *nivel[NUM_FOLHAS];
    int quantidade = NUM_FOLHAS / 2;
    for (int i = 0; i < quantidade; i++) {
        nivel[i] = alocarNo();
        if (!nivel[i]) {
            Serial.printf("[MSS][ERRO] Falha ao alocar no da camada inicial %d.\n", i);
            return NULL;
        }
        conectarFolhasAoNo(nivel[i], &folhas[2 * i], &folhas[2 * i + 1]);
    }
    Serial.printf("[MSS][ARVORE] Camada inicial criada com %d nos.\n", quantidade);
    while (quantidade > 1) {
        int nova = quantidade / 2;
        No **proximo = (No **)malloc((size_t)nova * sizeof(No *));
        if (!proximo) {
            Serial.printf("[MSS][ERRO] Falha ao alocar camada com %d nos.\n", nova);
            return NULL;
        }
        for (int i = 0; i < nova; i++) {
            proximo[i] = alocarNo();
            if (!proximo[i]) {
                Serial.printf("[MSS][ERRO] Falha ao alocar no %d da camada.\n", i);
                free(proximo);
                return NULL;
            }
            proximo[i]->filho_esq = nivel[2 * i];
            proximo[i]->filho_dir = nivel[2 * i + 1];
            proximo[i]->tipo_filho_esq = TIPO_NO;
            proximo[i]->tipo_filho_dir = TIPO_NO;
            criarPai(proximo[i]);
        }
        memcpy(nivel, proximo, (size_t)nova * sizeof(No *));
        free(proximo);
        quantidade = nova;
        Serial.printf("[MSS][ARVORE] Nova camada criada com %d nos.\n", quantidade);
    }
    Serial.println("[MSS][ARVORE] Arvore concluida.");
    return nivel[0];
}

static void liberarContexto(No *raiz, Folha *folhas) {
    if (raiz) limparArvore(raiz);
    if (folhas) {
        for (int i = 0; i < NUM_FOLHAS; i++) {
            free(folhas[i].Skeys);
            free(folhas[i].Pkeys);
        }
        free(folhas);
    }
}

static bool serializar(const AssinaturaMSS *a, uint8_t *out) {
    if (!a || !a->wotsSignature || !a->folhaPkeys || a->tamanhoCaminho < 0 || a->tamanhoCaminho > 32) {
        Serial.println("[MSS][ERRO] Dados invalidos para serializacao.");
        return false;
    }
    uint8_t *p = out;
    uint32_t campos[4] = {(uint32_t)a->alturaArvore, (uint32_t)a->totalFolhas,
                          (uint32_t)a->indiceFolha, (uint32_t)a->tamanhoCaminho};
    memcpy(p, campos, sizeof(campos)); p += sizeof(campos);
    memcpy(p, a->hashFolha, 32); p += 32;
    memcpy(p, a->caminho, 32 * 32); p += 32 * 32;
    memcpy(p, a->caminhoDirecao, 32); p += 32;
    memcpy(p, a->leaf_PK_seed, 32); p += 32;
    memcpy(p, a->leaf_SK_seed, 32); p += 32;
    memcpy(p, a->folhaPkeys, sizeof(PublicKeys)); p += sizeof(PublicKeys);
    memcpy(p, a->wotsSignature, sizeof(Assinatura));
    return true;
}

static uint8_t *empacotar(const char *msg, const uint8_t *pk, size_t pk_size,
                          const uint8_t *sig, size_t sig_size, size_t *out_size) {
    size_t msg_size = strlen(msg);
    *out_size = 12 + msg_size + pk_size + sig_size;
    uint8_t *out = (uint8_t *)malloc(*out_size);
    if (!out) return NULL;
    uint8_t *p = out;
    uint32_t size = (uint32_t)msg_size; memcpy(p, &size, 4); p += 4;
    memcpy(p, msg, msg_size); p += msg_size;
    size = (uint32_t)pk_size; memcpy(p, &size, 4); p += 4;
    memcpy(p, pk, pk_size); p += pk_size;
    size = (uint32_t)sig_size; memcpy(p, &size, 4); p += 4;
    memcpy(p, sig, sig_size);
    return out;
}

static bool gerarEEnviar() {
    Serial.println();
    Serial.println("========== MSS: INICIO ==========");
    Serial.printf("[MSS] Heap inicial: %u bytes.\n", ESP.getFreeHeap());
    Serial.printf("[MSS] Mensagem: \"%s\"\n", MENSAGEM_PADRAO);
    Serial.printf("[MSS] Folhas: %d | Tamanho assinatura serializada: %u bytes.\n",
                  NUM_FOLHAS, (unsigned int)MSS_WIRE_SIZE);

    uint32_t heap_antes = ESP.getFreeHeap();
    sha256_reset_counter();
    uint32_t inicio = micros();
    Folha *folhas = (Folha *)calloc(NUM_FOLHAS, sizeof(Folha));
    No *raiz = NULL;
    AssinaturaMSS *assinatura = folhas ? alocarAssinatura() : NULL;
    uint8_t *wire = NULL, *pacote = NULL;
    size_t pacote_size = 0;
    uint32_t tempo = 0;
    uint32_t ram = 0;
    char metricas[320];
    bool enviado = false;

    if (!folhas || !assinatura) {
        Serial.println("[MSS][ERRO] Falha ao alocar folhas ou assinatura.");
        goto cleanup;
    }
    Serial.println("[MSS] Memoria principal alocada.");
    for (int i = 0; i < NUM_FOLHAS; i++) {
        folhas[i].Skeys = mallocSkeys();
        folhas[i].Pkeys = mallocPkeys();
        if (!folhas[i].Skeys || !folhas[i].Pkeys) {
            Serial.printf("[MSS][ERRO] Falha ao alocar chaves da folha %d.\n", i);
            goto cleanup;
        }
    }
    Serial.println("[MSS] Chaves das folhas alocadas.");
    criarFolhas(folhas, NUM_FOLHAS);
    Serial.printf("[MSS] Folhas geradas. Heap atual: %u bytes.\n", ESP.getFreeHeap());
    raiz = mssTree(folhas);
    if (!raiz) {
        Serial.println("[MSS][ERRO] Falha ao construir a arvore.");
        goto cleanup;
    }
    Serial.print("[MSS] Raiz: ");
    for (int i = 0; i < 32; i++) {
        if (raiz->hash[i] < 16) Serial.print("0");
        Serial.print(raiz->hash[i], HEX);
    }
    Serial.println();
    criarAssinatura(assinatura, raiz, &folhas[0], 0, NUM_FOLHAS, (char *)MENSAGEM_PADRAO);
    Serial.printf("[MSS] Assinatura criada. Caminho de autenticacao: %d niveis.\n",
                  assinatura->tamanhoCaminho);
    wire = (uint8_t *)malloc(MSS_WIRE_SIZE);
    if (!wire) {
        Serial.println("[MSS][ERRO] Falha ao alocar buffer da assinatura.");
        goto cleanup;
    }
    if (!serializar(assinatura, wire)) goto cleanup;
    Serial.printf("[MSS] Assinatura serializada: %u bytes.\n", (unsigned int)MSS_WIRE_SIZE);
    pacote = empacotar(MENSAGEM_PADRAO, raiz->hash, 32, wire, MSS_WIRE_SIZE, &pacote_size);
    if (!pacote) {
        Serial.println("[MSS][ERRO] Falha ao empacotar pacote MQTT.");
        goto cleanup;
    }
    Serial.printf("[MSS] Pacote MQTT pronto: %u bytes.\n", (unsigned int)pacote_size);

    tempo = micros() - inicio;
    ram = heap_antes > ESP.getFreeHeap() ? heap_antes - ESP.getFreeHeap() : 0;
    snprintf(metricas, sizeof(metricas),
             "{\"dispositivo\":\"ESP32\",\"algoritmo\":\"MSS\",\"operacao\":\"assinatura\","
             "\"tempo_keygen_us\":%u,\"tempo_assinatura_us\":%u,\"numero_hashes\":%llu,"
             "\"tamanho_pkey_bytes\":32,\"tamanho_assinatura_bytes\":%u,\"uso_ram_bytes\":%u}",
             tempo, 0U, sha256_get_counter(), (unsigned int)MSS_WIRE_SIZE, ram);
    Serial.printf("[MSS] Metricas: %s\n", metricas);
    if (!client.publish("estat/esp", metricas)) {
        Serial.println("[MSS][ERRO] Falha ao publicar metricas em estat/esp.");
        goto cleanup;
    }
    Serial.println("[MSS] Metricas publicadas em estat/esp.");
    enviado = client.publish(TOPICO_ENVIAR, pacote, (unsigned int)pacote_size);
    if (enviado) {
        Serial.printf("[MSS] Pacote publicado em %s.\n", TOPICO_ENVIAR);
    } else {
        Serial.printf("[MSS][ERRO] Falha ao publicar pacote em %s.\n", TOPICO_ENVIAR);
    }

cleanup:
    if (enviado) {
        digitalWrite(LED_VERDE, HIGH); delay(500); digitalWrite(LED_VERDE, LOW);
    } else {
        digitalWrite(LED_VERMELHO, HIGH); delay(500); digitalWrite(LED_VERMELHO, LOW);
    }
    free(pacote); free(wire);
    if (assinatura) liberarAssinatura(assinatura);
    liberarContexto(raiz, folhas);
    Serial.printf("[MSS] Recursos liberados. Heap final: %u bytes.\n", ESP.getFreeHeap());
    Serial.printf("[MSS] Resultado: %s.\n", enviado ? "SUCESSO" : "FALHA");
    Serial.println("=========== MSS: FIM ===========");
    return enviado;
}

static void reconectar() {
    while (!client.connected()) {
        Serial.printf("[MQTT] Conectando ao broker %s:%d...\n", broker, porta);
        if (client.connect("ESP32_MSS_Transmitter")) {
            Serial.println("[MQTT] Conectado ao broker.");
            return;
        }
        Serial.printf("[MQTT][ERRO] Falha na conexao, estado=%d. Nova tentativa em 5 s.\n",
                      client.state());
        delay(5000);
    }
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    pinMode(BOTAO_PIN, INPUT);
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_VERMELHO, OUTPUT);
    Serial.println();
    Serial.println("===== Inicializando publisher MSS =====");
    Serial.printf("[MSS] MQTT_MAX_PACKET_SIZE esperado: %u bytes.\n",
                  (unsigned int)MSS_WIRE_SIZE + 128U);
    Serial.printf("[WIFI] Conectando a rede \"%s\"...\n", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.printf("[WIFI] Conectado. IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[WIFI] RSSI: %d dBm.\n", WiFi.RSSI());
    client.setServer(broker, porta);
    client.setBufferSize(10000);
    Serial.println("[MSS] Inicializacao concluida. Pressione o botao para assinar.");
    Serial.println("[MSS] GPIO34 requer resistor externo de 10k para GND.");

    uint32_t inicio_baixo = millis();
    while (botaoPressionado() && millis() - inicio_baixo < 3000) {
        Serial.println("[MSS][AVISO] GPIO34 esta HIGH; aguardando estado solto (LOW).");
        delay(500);
    }
    if (botaoPressionado()) {
        Serial.println("[MSS][ERRO] GPIO34 permaneceu HIGH. Verifique o resistor para GND.");
    } else {
        Serial.println("[MSS] Botao inicialmente solto.");
    }
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
        Serial.println("[MSS] Botao pressionado; iniciando debounce.");
    }
    if (atual && anterior && !aguardando_liberacao && millis() - borda >= 50) {
        Serial.println("[MSS] Debounce confirmado; iniciando operacao.");
        gerarEEnviar();
        aguardando_liberacao = true;
    }
    if (aguardando_liberacao && !atual) {
        Serial.println("[MSS] Botao liberado.");
        aguardando_liberacao = false;
    }
    anterior = atual;
}
