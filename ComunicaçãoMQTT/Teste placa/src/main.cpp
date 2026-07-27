#include <Arduino.h>
/*
const int LED_PIN = 2;

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);

  digitalWrite(LED_PIN, LOW);
  delay(1000);
}*/

#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Gabriel";
const char* password = "@tamara12";
const char* broker = "192.168.3.105";   // IP do PC que executa o Mosquitto
const int porta = 1883;

const int LED_PIN = 2;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("\nRecebido pacote no topico [");
  Serial.print(topic);
  Serial.print("] com ");
  Serial.print(length);
  Serial.println(" bytes.");

  if (length < 8) {
    Serial.println("Erro: Pacote curto demais.");
    return;
  }

  // Pisca o LED indicando recepcao
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);

  // 1. Extrai o tamanho da mensagem (primeiros 4 bytes)
  uint32_t tamanho_msg;
  memcpy(&tamanho_msg, payload, 4);

  if (4 + tamanho_msg + 4 > length) {
    Serial.println("Erro: Tamanho da mensagem invalido ou pacote truncado.");
    return;
  }

  // 2. Extrai a mensagem
  char* mensagem = (char*)malloc(tamanho_msg + 1);
  if (mensagem == NULL) {
    Serial.println("Erro de memoria ao alocar string da mensagem.");
    return;
  }
  memcpy(mensagem, payload + 4, tamanho_msg);
  mensagem[tamanho_msg] = '\0';

  // 3. Extrai o tamanho da assinatura (4 bytes após a mensagem)
  uint32_t tamanho_assinatura;
  memcpy(&tamanho_assinatura, payload + 4 + tamanho_msg, 4);

  if (4 + tamanho_msg + 4 + tamanho_assinatura != length) {
    Serial.println("Erro: Integridade do pacote corrompida (tamanho assinatura incorreto).");
    free(mensagem);
    return;
  }

  // 4. Extrai a assinatura binaria
  byte* assinatura = (byte*)malloc(tamanho_assinatura);
  if (assinatura == NULL) {
    Serial.println("Erro de memoria ao alocar buffer da assinatura.");
    free(mensagem);
    return;
  }
  memcpy(assinatura, payload + 4 + tamanho_msg + 4, tamanho_assinatura);

  // Exibe informações desempacotadas
  Serial.println("====== DADOS DESEMPACOTADOS ======");
  Serial.print("Mensagem: \"");
  Serial.print(mensagem);
  Serial.println("\"");
  
  Serial.print("Tamanho Assinatura: ");
  Serial.print(tamanho_assinatura);
  Serial.println(" bytes.");
  
  Serial.print("Assinatura (primeiros 16 bytes em HEX): ");
  for (unsigned int i = 0; i < 16 && i < tamanho_assinatura; i++) {
    char hexBuf[3];
    sprintf(hexBuf, "%02x", assinatura[i]);
    Serial.print(hexBuf);
  }
  Serial.println("...");
  Serial.println("==================================");

  // Libera a memória alocada
  free(mensagem);
  free(assinatura);
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Aguarda a serial se estabilizar
  Serial.println("Teste Serial OK");
  pinMode(LED_PIN, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
//    digitalWrite(LED_PIN, LOW);
    delay(500);
    Serial.print(".");
  }
//  digitalWrite(LED_PIN, HIGH);

  Serial.println("\nWiFi conectado");

  client.setServer(broker, porta);
  client.setCallback(callback);
}

void reconectar() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao broker MQTT...");
    if (client.connect("ESP32_Cliente")) {
      client.subscribe("teste/esp");
      Serial.println(" conectado!");
    } else {
      Serial.print(" falhou, rc=");
      Serial.print(client.state());
      Serial.println(". Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) reconectar();
  client.loop();

  // Publica uma mensagem a cada 3 segundos
  static unsigned long ultimo = 0;
  if (millis() - ultimo > 3000) {
    ultimo = millis();
    client.publish("teste/pc", "Olá PC!");
    Serial.println("Mensagem enviada");
  }
}