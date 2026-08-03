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

---

## Comunicação LOTS com Coleta de Métricas (`lotsPlaca/`)

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
cd ComunicaçãoMQTT/lotsPlaca/PC/
make
./publisherLots
```

**3. Compilar e enviar firmware para a ESP32:**
```bash
cd ComunicaçãoMQTT/lotsPlaca/LotsPlaca/
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