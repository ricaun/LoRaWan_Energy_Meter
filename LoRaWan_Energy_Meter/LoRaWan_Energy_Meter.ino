//----------------------------------------//
//  LoRaWan_Energy_Meter.ino
//
//  created 23 10 2020
//  by Luiz Henrique Cassettari
//----------------------------------------//

void setup()
{
  Serial.begin(115200);
  LoRa_setup();
  LoRa_sendMessage();
}

void loop() {
  LoRa_loop();
  if (!LoRa_busy())
  {
    Serial.println("Sleep");
    delay(10);
    LoRa_endSleep();
    pima_read();
    LoRa_sendMessage();
    Serial.println("Send Message!");
  }
}
