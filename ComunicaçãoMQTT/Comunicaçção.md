# Comunicação MQTT

A comunicação MQTT é realizada utilizando o broker [Mosquitto](https://mosquitto.org/).

## Scripts de Teste

- `publisher.py`: Publica mensagens do computador para a ESP32 (`teste/esp`).
- `subscriber.py`: Fica no aguardo de mensagens enviadas pela ESP32 para o computador (`teste/pc`).

## Código da Placa (`Teste placa/`)

O código presente no diretório `Teste placa/`:
- Recebe a mensagem enviada pelo computador e pisca o LED ao recebê-la corretamente.
- Envia uma mensagem de retorno para o computador.