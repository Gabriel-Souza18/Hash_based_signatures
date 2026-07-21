import paho.mqtt.client as mqtt

def on_message(client, userdata, msg):
    print(f"Recebido: {msg.payload.decode()}")

broker = "localhost"
topico = "teste/pc"

client = mqtt.Client()
client.on_message = on_message
client.connect(broker, 1883)
client.subscribe(topico)
client.loop_forever()