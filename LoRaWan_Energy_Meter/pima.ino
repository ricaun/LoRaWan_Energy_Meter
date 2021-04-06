//----------------------------------------//
//  pima.ino
//
//  created 17 09 2020
//  by Luiz Henrique Cassettari
//----------------------------------------//

#include <PIMA.h>
#include <SoftwareSerial.h>

struct sMedidorEnergia
{
  uint32_t id;
  uint32_t active;
  uint32_t inductive;
  uint32_t capacitive;
  uint32_t reverse;
};

sMedidorEnergia MedidorEnergia;

#define TX 4
#define RX 3
#define POWER 5

SoftwareSerial mySerial(RX, TX); // RX, TX

void pimaMessage (PIMALayer pima);

void pima_init()
{
  serial_init();
  PIMA.begin(&mySerial);
  PIMA.onMessage(pimaMessage);
}

void pima_loop()
{
  PIMA.loop();
}

void pima_end()
{
  serial_end();
}

void pima_read()
{
  pima_runEvery(0);
  pima_clear();
  pima_init();
  while(medidor_id() == 0)
  {
    pima_loop();
    if (pima_runEvery(5000)) break;
  }
  pima_end();
}

boolean pima_runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

void pimaMessage(PIMALayer pima)
{
  Serial.print("id : ");
  Serial.println(pima.id);
  Serial.print("index : ");
  Serial.println(pima.index , HEX);
  Serial.print("value : ");
  Serial.println(pima.value);
  switch(pima.index)
  {
    case PIMA_INDEX_ACTIVE:
      MedidorEnergia.active = pima.value;
      break;
    case PIMA_INDEX_INDUCTIVE:
      MedidorEnergia.inductive = pima.value;
      break;
    case PIMA_INDEX_CAPACITIVE:
      MedidorEnergia.capacitive = pima.value;
      break;
    case PIMA_INDEX_REVERSE:
      MedidorEnergia.reverse = pima.value;
      break;
  }
  if (MedidorEnergia.id == 0)
  {
    MedidorEnergia.id = pima.id;
  }
}

void serial_init()
{
  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, HIGH);
  mySerial.begin(2400);
}

void serial_end()
{
  mySerial.end();
  pinMode(POWER, INPUT);
}

float medidor_energy()
{
  return MedidorEnergia.active;
}

float medidor_inductive()
{
  return MedidorEnergia.inductive;
}

float medidor_capacitive()
{
  return MedidorEnergia.capacitive;
}

float medidor_reverse()
{
  return MedidorEnergia.reverse;
}

uint32_t medidor_id()
{
  return MedidorEnergia.id;
}

void pima_clear()
{
  MedidorEnergia.id = 0;
  MedidorEnergia.active = 0;
  MedidorEnergia.inductive = 0;
  MedidorEnergia.capacitive = 0;
  MedidorEnergia.reverse = 0;
}
