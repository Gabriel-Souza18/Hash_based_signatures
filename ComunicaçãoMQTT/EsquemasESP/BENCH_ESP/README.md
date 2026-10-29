# BENCH_ESP

Benchmark dos algoritmos de assinaturas baseadas em hash executado na ESP32.
O projeto não utiliza MQTT: os resultados são exibidos diretamente no monitor
Serial.

## Requisitos

- ESP32 Dev Module (`esp32dev`);
- PlatformIO instalado;
- Cabo USB para upload e monitoramento;
- Botão ligado ao GPIO34;
- Resistor externo de aproximadamente 10 kΩ entre GPIO34 e GND.

O GPIO34 é somente entrada e não possui pull-up/pull-down interno. Use um
resistor externo de 10 kΩ entre GPIO34 e GND, e faça o botão aplicar 3,3 V ao
GPIO34 quando pressionado. Sem esse resistor, o pino fica flutuante e pode ser
interpretado como permanentemente pressionado.

## Fluxo pelo botão

O botão continua ativo em todos os ambientes:

1. Ao ligar a placa, o LED vermelho indica que o benchmark está aguardando;
2. Ao pressionar o botão, o LED vermelho apaga e o LED verde acende;
4. O benchmark selecionado é executado `BENCH_RUNS` vezes (atualmente 100);
5. Os resultados de keygen, assinatura e verificação são enviados ao Serial;
6. Cada repetição conclui o ciclo de geração, assinatura e verificação antes
   de liberar as estruturas temporárias;
7. Ao terminar, o LED verde apaga, o vermelho acende novamente e a placa
   aguarda o botão ser liberado;
8. Um novo benchmark pode ser iniciado pressionando o botão novamente.

Pinos utilizados:

| Função | GPIO |
|---|---:|
| Botão | 34 |
| LED vermelho | 12 |
| LED verde | 14 |

## Compilar e gravar

Execute os comandos a partir desta pasta:

```bash
cd ComunicaçãoMQTT/EsquemasESP/BENCH_ESP
```

Compile o algoritmo desejado:

```bash
pio run -e <ambiente>
```

Grave a firmware na ESP32:

```bash
pio run -e <ambiente> -t upload
```

Abra o monitor Serial:

```bash
pio device monitor -b 115200
```

Também é possível gravar e abrir o monitor em sequência:

```bash
pio run -e <ambiente> -t upload && pio device monitor -b 115200
```

Substitua `<ambiente>` por um dos ambientes abaixo.

## Ambientes disponíveis

### LOTS

```bash
pio run -e esp32dev_bench_lots -t upload
pio device monitor -b 115200
```

Executa:

- geração de chaves secretas;
- geração de chaves públicas;
- assinatura;
- verificação.

### WOTS+

```bash
pio run -e esp32dev_bench_wots -t upload
pio device monitor -b 115200
```

Executa as operações do WOTS+ usando as funções existentes em `WOTS/keys.c`
e `WOTS/prf.c`.

### HORS

```bash
pio run -e esp32dev_bench_hors -t upload
pio device monitor -b 115200
```

Executa geração de chaves, assinatura e verificação HORS.

### HORST

```bash
pio run -e esp32dev_bench_horst -t upload
pio device monitor -b 115200
```

Executa geração de chaves e árvore HORST, assinatura e verificação.

### MSS

```bash
pio run -e esp32dev_bench_mss -t upload
pio device monitor -b 115200
```

Executa geração das folhas e da árvore Merkle, assinatura MSS e verificação.
O MSS utiliza o WOTS+ original como componente interno.

### SPHINCS

```bash
pio run -e esp32dev_bench_sphincs -t upload
pio device monitor -b 115200
```

Executa geração de chaves, assinatura SPHINCS e verificação. O ambiente inclui
os módulos originais XMSS, HORST, WOTS+ e SHA-256.

## Saída no Serial

Configure o monitor em **115200 baud**. Cada execução informa somente:

- algoritmo;
- operação;
- número da execução;
- tempo em microssegundos;
- número de hashes contados;
- RAM consumida;
- resultado da verificação (`VALIDA` ou `INVALIDA`).

Exemplo de início de execução:

```text
ALGORITMO=SPHINCS
OPERACAO=KEYGEN EXECUCAO=1 TEMPO_US=... HASHES=... RAM_BYTES=...
```

No LOTS, as quatro operações de cada repetição usam o mesmo
`EXECUCAO=<n>`. Para repetir o teste do mesmo algoritmo, não é necessário fazer
novo upload:
libere o botão e pressione-o novamente.
