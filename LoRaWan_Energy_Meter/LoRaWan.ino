//----------------------------------------//
//  LoRaWan.ino
//
//  created 17 09 2020
//  by Luiz Henrique Cassettari
//----------------------------------------//

#include <SPI.h>
#include <LoRa.h>

#include <ArduinoUniqueID.h>
#include <LoRaWanPacket.h>

const int csPin = 10;
const int resetPin = -1;
const int irqPin = 2;
const int powerPin = 8;

const char *appEui = "XXXXXXXXXXXXXXXX";
const char *appKey = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

#define VCC powerPin
#define VCC_ON pinMode(VCC, OUTPUT); digitalWrite(VCC, LOW); delay(10);
#define VCC_OFF pinMode(VCC, OUTPUT); digitalWrite(VCC, HIGH);

struct LoRa_config
{
  long Frequency;
  int SpreadingFactor;
  long SignalBandwidth;
  int CodingRate4;
  bool enableCrc;
  bool invertIQ;
  int SyncWord;
  int PreambleLength;
};

// ttn US915
static LoRa_config txLoRa = {903900000, 7, 125000, 5, true, false, 0x34, 8};
static LoRa_config rxLoRa = {923300000, 7, 500000, 5, false, true, 0x34, 8};
static LoRa_config rxLoRa2 = {923300000, 12, 500000, 5, false, true, 0x34, 8};

static int busy = 0;
static int sleep = 60;

const static int delay_join = 5000;
const static int delay_uplink = 1000;

void onTxDone();
void onReceive(int packetSize);

void LoRa_begin()
{
  VCC_ON;
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(txLoRa.Frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }
}

void LoRa_sleep()
{
  LoRa.sleep();
}

void LoRa_end()
{
  busy = 0;
  LoRa.sleep();
  LoRa.end();
  SPI.end();
  VCC_OFF;
}

void LoRa_endSleep()
{
  LoRa_end();
  system_sleep_x(sleep);
  LoRa_begin();
}

void LoRa_loop()
{
  if (busy == 2)
  {
    long time = LoRaWanPacket.isJoin() ? delay_join : delay_uplink;
    time -= 100;

    if (LoRa_runEvery(time))
    {
      busy = 3;
      LoRa_RxMode();
      Serial.println("LoRa_RxMode!");
    }
  }
  else if (busy == 3)
  {
    long time = 1000;
    if (LoRa_runEvery(time))
    {
      busy = 4;
      LoRa_RxMode2();
      Serial.println("LoRa_RxMode2!");
    }
  }
  else if (busy == 4)
  {
    long time = 1000;

    if (LoRa_runEvery(time))
    {
      LoRa_sleep();
      busy = 0;
      Serial.println("LoRa_busy_off!");
    }
  }
}

bool LoRa_busy()
{
  return busy > 0;
}

void LoRa_setup()
{
  UniqueID8dump(Serial);
  LoRaWanPacket.join(appEui, appKey);
  LoRaWanPacket.setDevEui(UniqueID8);

  LoRa_begin();

  LoRa.receive();
  uint32_t seed = (uint32_t)LoRa_random() << 24 | (uint32_t)LoRa_random() << 16 | (uint32_t)LoRa_random() << 8 | (uint32_t)getVcc();
  randomSeed(seed);

  Serial.println("LoRa init succeeded.");
  Serial.println();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
}

void onReceive(int packetSize) {
  busy = 0;
  Serial.println("Receive!");
  LoRaWanPacket.clear();
  while (LoRa.available()) {
    LoRaWanPacket.write(LoRa.read());
  }
  int port = LoRaWanPacket.decode();
  int length = LoRaWanPacket.length();
  switch (port) {
    case 0:
      break;
    case 1:
      if (length == 4)
        sleep = LoRaWanPacket.readLong();
      else if (length == 2)
        sleep = LoRaWanPacket.readInt();
      else if (length == 1)
        sleep = LoRaWanPacket.read();
      break;
  }
}

void LoRa_sendMessage()
{
  busy = 1;
  LoRa_TxMode();
  LoRaWanPacket.clear();
  PayloadNow(LoRaWanPacket);
  if (LoRaWanPacket.encode())
  {
    LoRa.beginPacket();
    LoRa.write(LoRaWanPacket.buffer(), LoRaWanPacket.length());
    LoRa.endPacket(true);
  }
}

void onTxDone() {
  busy = 2;
  LoRa_runEvery(0);
  LoRa_sleep();
}

boolean LoRa_runEvery(unsigned long interval)
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


byte LoRa_random()
{
  static byte result = 0;
  for (int i = 0; i < 8; i++) {
    result = (result << 1) | (result >> 7); // Spread randomness around / rotate left
    result ^= LoRa.random(); // XOR preserves randomness
  }
  return result;
}

void LoRa_setConfig(struct LoRa_config config)
{
  LoRa.setFrequency(config.Frequency);
  LoRa.setSpreadingFactor(config.SpreadingFactor);
  LoRa.setSignalBandwidth(config.SignalBandwidth);
  LoRa.setCodingRate4(config.CodingRate4);
  if (config.enableCrc)
    LoRa.enableCrc();
  else
    LoRa.disableCrc();
  if (config.invertIQ)
    LoRa.enableInvertIQ();
  else
    LoRa.disableInvertIQ();
  LoRa.setSyncWord(config.SyncWord);
  LoRa.setPreambleLength(config.PreambleLength);
}

void LoRa_TxMode()
{
  LoRa_setConfig(txLoRa);
  LoRa.idle();
}

void LoRa_RxMode()
{
  LoRa_setConfig(rxLoRa);
  LoRa.receive();
}

void LoRa_RxMode2()
{
  LoRa_setConfig(rxLoRa2);
  LoRa.receive();
}
