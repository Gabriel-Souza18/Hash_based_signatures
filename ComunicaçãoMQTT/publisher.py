import paho.mqtt.client as mqtt
import time

broker = "localhost"  # ou IP do PC na rede local
topico = "teste/esp"

client = mqtt.Client()
client.connect(broker, 1883)

while True:
    client.publish(topico, "Olá ESP32!")
    print("Mensagem enviada")
    time.sleep(2)