# Comunicação MQTT com Assinatura LOTS (Lamport OTS)

Este diretório contém a implementação do teste de comunicação MQTT integrado com a assinatura baseada em hash **LOTS (Lamport One-Time Signature)**. 

O sistema é dividido em duas partes:
1. **PC (Publicador em C)**: Gera as chaves, assina a mensagem, empacota tudo e publica via MQTT.
2. **Placa (Receptor ESP32)**: Se inscreve no tópico, recebe o pacote, desempacota e verifica a assinatura localmente.

---

##  Formato do Pacote Unificado
Para garantir que a mensagem, a chave pública e a assinatura cheguem de forma atômica (sem perdas ou fora de ordem), os dados são serializados em um único buffer binário enviado via MQTT no seguinte formato:

```
[Tamanho Msg (4 Bytes)] + [Texto Msg (N Bytes)] + [Tamanho PKey (4 Bytes)] + [Chave Pública (16 KB)] + [Tamanho Assinatura (4 Bytes)] + [Assinatura (8 KB)]
```

---

##  1. Executando o Publicador no PC (`PC/`)

O publicador lê as mensagens digitadas por você no terminal de forma dinâmica, assina e envia o pacote.

### Pré-requisitos
Instale as bibliotecas necessárias:
```bash
sudo apt install libmosquitto-dev libsodium-dev
```

### Como Compilar e Rodar:
```bash
cd PC/
make
./publisherLots
```

---

## 2. Executando o Receptor na ESP32 (`LotsPlaca/`)

O firmware do microcontrolador extrai os dados do pacote MQTT e faz a verificação matemática usando a biblioteca criptográfica nativa da placa.

### Como Compilar e Enviar para a Placa:
```bash
cd LotsPlaca/
pio run --target upload
pio device monitor
```
*Observação: Lembre-se de configurar o seu Wi-Fi (`ssid` e `password`) e o IP do seu computador (`broker`) no arquivo `src/main.cpp`.*
