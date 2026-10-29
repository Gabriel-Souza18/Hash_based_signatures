# Comunicação MQTT

A comunicação MQTT é realizada utilizando o broker [Mosquitto](https://mosquitto.org/).

---

## Scripts de Teste Básico

- `publisher.py`: Publica mensagens do computador para a ESP32 (`teste/esp`).
- `subscriber.py`: Fica no aguardo de mensagens enviadas pela ESP32 para o computador (`teste/pc`).

## Código da Placa de Teste (`Teste placa/`)

O código presente no diretório `Teste placa/`:
- Recebe a mensagem enviada pelo computador e pisca o LED ao recebê-la corretamente.
- Envia uma mensagem de retorno para o computador.
- **Serve como base para a criação de novos projetos em `Esquemas/`.**

---

## Esquemas (`Esquemas/`)

Contém as implementações da comunicação MQTT integradas com cada algoritmo de assinatura. Cada subpasta é um projeto PlatformIO independente para a ESP32.

```
Esquemas/
├── LOTS/       ← Lamport OTS (implementado)
├── WOTS/       ← Winternitz OTS (base)
├── HORS/       ← Hash to Obtain Random Subset (publisher PC + publisher ESP32)
├── HORST/      ← HORS with Trees (base)
├── MSS/        ← Merkle Signature Scheme (publisher PC + publisher ESP32)
└── SPHINCS/    ← SPHINCS (base)
```

> Pastas marcadas como **(base)** contêm apenas o projeto mínimo copiado de `Teste placa/`, sem implementação do algoritmo ainda.

### MSS com métricas

O MSS usa o mesmo formato binário de transporte de LOTS/WOTS:
`[tamanho da mensagem][mensagem][tamanho da chave pública][raiz MSS][tamanho da assinatura][assinatura MSS serializada]`.
O publisher da ESP32 assina uma mensagem ao pressionar o botão e publica em `mss/esp_to_pc`.
O programa do PC funciona como receptor/verificador por padrão e como publisher para a ESP32 com `--send`.

```bash
# PC: compilar e executar o receptor/verificador
cd ComunicaçãoMQTT/EsquemasPC/MSS
make
./publisherMss

# PC: publicar uma assinatura MSS para a ESP32
./publisherMss --send

# ESP32: compilar/uploadar o publisher MSS
cd ComunicaçãoMQTT/EsquemasESP/MSS
pio run --target upload
pio device monitor
```

As métricas MSS são publicadas em `estat/pc` e `estat/esp` e consolidadas pelo
[`logger.py`](logs/logger.py), como nos esquemas LOTS e WOTS.

### HORS com métricas

O HORS utiliza `HORS_T=1024` e `HORS_K=26`. A chave pública tem 32.768 bytes,
portanto o projeto usa `MQTT_MAX_PACKET_SIZE=40000`.

O formato do pacote é:
`[tamanho da mensagem][mensagem][tamanho da chave pública][PKeys HORS][tamanho da assinatura][assinatura HORS]`.

```bash
# PC: compilar e executar o receptor/verificador
cd ComunicaçãoMQTT/EsquemasPC/HORS
make
./publisherHors

# PC: publicar uma assinatura HORS para a ESP32
./publisherHors --send

# ESP32: compilar/uploadar o publisher HORS
cd ComunicaçãoMQTT/EsquemasESP/HORS
pio run --target upload
pio device monitor
```

O publisher da ESP32 assina a mensagem ao pressionar o botão e publica em
`hors/esp_to_pc`. As métricas são enviadas para `estat/esp`; o receptor PC
publica os dados de verificação em `estat/pc`.

---

## Comunicação LOTS com Coleta de Métricas (`Esquemas/LOTS/`)

Este diretório contém a implementação da comunicação MQTT integrada com a assinatura **LOTS (Lamport OTS)** e um sistema automático de coleta de métricas de desempenho.

### Arquitetura

```
PC (publisherLots.c)                    ESP32 (main.cpp)
  |                                        |
  |-- Gera chaves (SK, PK)                |
  |-- Assina mensagem                     |
  |-- Empacota [Msg + PKey + Assinatura]  |
  |-- Publica no tópico "lots" ---------> |-- Recebe e desempacota
  |-- Publica métricas em "estat/pc" ---> |-- Verifica assinatura
  |                                        |-- Publica métricas em "estat/esp"
  |                                        |
  
  logger.py (escuta "estat/#")
  |-- Recebe métricas do PC e da ESP32
  |-- Consolida e salva em algoritmo_timestamp.json
```

### Métricas Coletadas

| Métrica | PC | ESP32 |
| :--- | :---: | :---: |
| Tempo de geração de chaves (SK e PK) | ✅ | — |
| Tempo de assinatura | ✅ | — |
| Tempo de verificação | — | ✅ |
| Número de hashes SHA-256 | ✅ | ✅ |
| Tamanho das chaves e assinatura (bytes) | ✅ | ✅ |
| Uso de memória RAM (Heap dinâmico) | — | ✅ |

> **Nota:** O uso de RAM no PC é medido separadamente com Valgrind (ver `TestesGerais/`), pois não é possível obter essa métrica em tempo real de forma precisa no Linux.

### Tópicos MQTT Utilizados

| Tópico | Direção | Conteúdo |
| :--- | :--- | :--- |
| `lots` | PC → ESP32 | Pacote binário: `[Msg + PKey + Assinatura]` |
| `estat/pc` | PC → Broker | JSON com métricas de geração de chaves e assinatura |
| `estat/esp` | ESP32 → Broker | JSON com métricas de verificação e uso de RAM |

### Como Executar

**1. Iniciar o Logger de Métricas (terminal 1):**
```bash
cd ComunicaçãoMQTT/
python3 logger.py
```

**2. Iniciar o Publisher no PC (terminal 2):**
```bash
cd ComunicaçãoMQTT/Esquemas/LOTS/PC/
make
./publisherLots
```

**3. Compilar e enviar firmware para a ESP32:**
```bash
cd ComunicaçãoMQTT/Esquemas/LOTS/
pio run --target upload
pio device monitor
```

> Lembre-se de configurar o `ssid`, `password` e o IP do `broker` no arquivo `src/main.cpp`.

### Saída

As métricas são salvas automaticamente na pasta `ComunicaçãoMQTT/` como arquivos JSON nomeados no formato:
```
{algoritmo}_{YYYYMMDD_HHMMSS}.json
```
Exemplo: `lots_20260803_201500.json`





## BENCH 
pio run -e esp32dev_bench_lots -t upload
pio device monitor -b 115200

pio run -e esp32dev_bench_wots -t upload
pio device monitor -b 115200

