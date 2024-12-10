#include <Arduino.h>

// Representa o quecada campo do placar gasta de byte em um array
typedef struct
{
  uint8_t inicio[2];
  uint8_t equipeA[3];
  uint8_t periodo[1];
  uint8_t equipeB[3];
  uint8_t setfaltas[2];
  uint8_t cronometro[4];
  uint8_t setfaltasb[2];
  uint8_t tempoaA[1];
  uint8_t tempoaB[1];
  uint8_t prog[2];
  uint8_t alarme[1];
  uint8_t servico[1];
  uint8_t reservado[1];
  uint8_t crc[1];
  uint8_t fim[3];
} placar_info_t;

// Array de números de acordo com o placar
const uint8_t cnum[] = {
    0xb0,
    0x31,
    0x32,
    0xb3,
    0x34,
    0xb5,
    0xb6,
    0x37,
    0x38,
    0xb9};

int gols = 0; // Contador de gols da equipe A

// Inicializa com um pacote aleatorio
placar_info_t pack = {0x02, 0x92, 0xbf, 0xb0, 0xb3, 0x45, 0x31, 0xb9, 0xb9, 0xbf, 0x32, 0xb0, 0xb0, 0xb0, 0xb0, 0xbf, 0x34, 0xb0, 0xb0, 0x32, 0x34, 0x32, 0xb0, 0x00, 0xd9, 0x02, 0x21, 0x23};

// Função que calcula o CRC8
uint8_t calcularCRC8(uint8_t *data, size_t length, uint8_t poly = 0x01, uint8_t init = 0x80)
{
  uint8_t crc = init;

  for (size_t i = 0; i < length; i++)
  {
    crc ^= data[i];

    for (int j = 0; j < 8; j++)
    {
      if (crc & 0x80)
        crc = (crc << 1) ^ poly;
      else
        crc <<= 1;
    }
  }

  return crc & 0xFF; // Garante que o CRC permaneça dentro de 8 bits
}

// Variável para controle de tempo
unsigned long lastUpdate = 0;
const unsigned long interval = 3000; // 3 segundos em milissegundos

void setup()
{
  // Configura a comunicação serial
  Serial.begin(9600);
}

void atualizarGols()
{
  // Atualiza os gols no array do placar
  uint8_t c = gols / 100; // Centenas
  if (c == 0)
  {
    pack.equipeA[0] = 0xbf;
  }
  else
  {
    pack.equipeA[0] = cnum[c];
  }

  pack.equipeA[1] = cnum[(gols % 100) / 10]; // Dezenas
  pack.equipeA[2] = cnum[gols % 10];         // Unidades

  gols++;
  if (gols > 199) // Reseta os gols ao atingir 200
    gols = 0;
}

void loop()
{
  // Calcula o CRC para os 24 primeiros bytes do pacote
  pack.crc[0] = calcularCRC8((uint8_t *)&pack, sizeof(placar_info_t) - 4);

  // // Verifica se o CRC calculado é igual ao CRC medido
  // Serial.printf("CRC calculado: %02X, CRC medido: %02X\n", pack.crc[0], pack.crc[0]);

  // Envia os dados do placar via serial
  Serial.write((uint8_t *)&pack, sizeof(placar_info_t));

  // Incrementa os gols a cada 3 segundos
  if (millis() - lastUpdate >= interval)
  {
    lastUpdate = millis();
    atualizarGols();
  }

  delay(100); // Pequeno delay
}
