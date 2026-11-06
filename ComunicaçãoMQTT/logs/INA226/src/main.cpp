#include <Arduino.h>
#include <LittleFS.h>
#include <Wire.h>

#define INA_ADDR 0x40
#define SHUNT_OHMS 0.1f
#define CURRENT_LSB 0.0001f  // 100 µA por bit
#define BOTAO_PIN 34

static File arquivoLog;
static bool gravando = false;
static bool botaoAnterior = false;
static unsigned long ultimoDebounce = 0;

void writeReg(uint8_t reg, uint16_t val) {
  Wire.beginTransmission(INA_ADDR);
  Wire.write(reg);
  Wire.write(val >> 8);
  Wire.write(val & 0xFF);
  Wire.endTransmission();
}

int16_t readReg(uint8_t reg) {
  Wire.beginTransmission(INA_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)INA_ADDR, (uint8_t)2);
  return (Wire.read() << 8) | Wire.read();
}

void nomeArquivo(char *nome, size_t tamanho, unsigned long inicio) {
  unsigned long segundos = inicio / 1000;
  unsigned int horas = segundos / 3600;
  unsigned int minutos = (segundos % 3600) / 60;
  unsigned int segundosRestantes = segundos % 60;
  unsigned int milissegundos = inicio % 1000;

  snprintf(nome, tamanho, "/INA226_%04u%02u%02u_%03u.txt",
           horas, minutos, segundosRestantes, milissegundos);
}

void iniciarGravacao() {
  char nome[40];
  nomeArquivo(nome, sizeof(nome), millis());
  arquivoLog = LittleFS.open(nome, FILE_WRITE);

  if (!arquivoLog) {
    Serial.println("Botao aberto - erro ao criar arquivo");
    return;
  }

  gravando = true;
  Serial.println("Botao aberto - gravacao iniciada");
}

void finalizarGravacao() {
  if (arquivoLog) {
    arquivoLog.flush();
    arquivoLog.close();
  }

  gravando = false;
  Serial.println("Botao fechado - gravacao encerrada");
}

void setup() {
  Serial.begin(115200);
  pinMode(BOTAO_PIN, INPUT);
  Wire.begin(21, 22);

  if (!LittleFS.begin(true)) {
    Serial.println("Erro ao montar LittleFS");
  }

  writeReg(0x00, 0x4527);  // média de 16 amostras, 1.1 ms, modo contínuo
  uint16_t cal = (uint16_t)(0.00512f / (CURRENT_LSB * SHUNT_OHMS));  // 512
  writeReg(0x05, cal);
}

void loop() {
  bool botaoAtual = digitalRead(BOTAO_PIN) == HIGH;
  if (botaoAtual != botaoAnterior && millis() - ultimoDebounce >= 50) {
    ultimoDebounce = millis();
    botaoAnterior = botaoAtual;

    if (botaoAtual) {
      if (gravando) {
        finalizarGravacao();
      } else {
        iniciarGravacao();
      }
    }
  }

  if (!gravando) {
    delay(20);
    return;
  }

  float busV   = (uint16_t)readReg(0x02) * 0.00125f;   // V
  float shuntMv = readReg(0x01) * 0.0025f;             // mV
  float current = readReg(0x04) * CURRENT_LSB * 1000;  // mA
  float power   = readReg(0x03) * CURRENT_LSB * 25 * 1000; // mW

  if (arquivoLog) {
    arquivoLog.printf("%lu;%.3f;%.3f;%.1f;%.1f\n",
                      millis(), busV, shuntMv, current, power);
  }

  delay(500);
}