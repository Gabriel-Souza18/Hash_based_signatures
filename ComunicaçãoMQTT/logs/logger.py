import paho.mqtt.client as mqtt
import json
import os
from datetime import datetime

# Configurações do Broker MQTT
BROKER = "localhost"
PORT = 1883
TOPICS = [("estat/#", 0)]

pc_metrics = None  # Armazenamento temporário para as métricas do PC

def save_json(esp_data):
    global pc_metrics
    
    # Extrai o nome do algoritmo e gera o timestamp
    algo = esp_data.get("algoritmo", "algoritmo").lower()
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"{algo}_{timestamp}.json"
    
    # Consolida os dados do PC e da ESP32 em um único JSON
    consolidated = {
        "pc": pc_metrics if pc_metrics else {},
        "esp": esp_data
    }
    
    # Salva o arquivo na mesma pasta do script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    file_path = os.path.join(script_dir, filename)
    
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(consolidated, f, indent=4)
        
    print(f"\n[LOGGER] JSON salvo com sucesso em: {file_path}")
    print(json.dumps(consolidated, indent=2))
    
    # Reseta o buffer temporário
    pc_metrics = None

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"[LOGGER] Conectado ao broker MQTT em '{BROKER}'!")
        client.subscribe(TOPICS)
        print(f"[LOGGER] Escutando tópicos de métricas: {TOPICS}")
    else:
        print(f"[LOGGER] Erro ao conectar ao broker. Código: {rc}")

def on_message(client, userdata, msg):
    global pc_metrics
    
    try:
        data = json.loads(msg.payload.decode())
    except json.JSONDecodeError:
        print(f"[LOGGER] Erro ao decodificar JSON no tópico {msg.topic}")
        return
        
    if msg.topic == "estat/pc":
        pc_metrics = data
        print("[LOGGER] Recebidas métricas do PC.")
        
    elif msg.topic == "estat/esp":
        print("[LOGGER] Recebidas métricas da ESP32. Gravando JSON...")
        save_json(data)

def main():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    
    # Permite passar o broker como argumento de linha de comando
    import sys
    broker_address = BROKER
    if len(sys.argv) > 1:
        broker_address = sys.argv[1]
        
    client.connect(broker_address, PORT, 60)
    
    try:
        client.loop_forever()
    except KeyboardInterrupt:
        print("\n[LOGGER] Encerrando...")

if __name__ == "__main__":
    main()
