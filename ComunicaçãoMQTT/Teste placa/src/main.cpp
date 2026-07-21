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
  Serial.print("Recebido [");
  Serial.print(topic);
  Serial.print("]: ");
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.println("Teste Serial OK");
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
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
    if (client.connect("ESP32_Cliente")) {
      client.subscribe("teste/esp");
      Serial.println("Conectado ao broker");
    } else {
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