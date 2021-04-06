//----------------------------------------//
//  Payload.ino
//
//  created 17 09 2020
//  by Luiz Henrique Cassettari
//----------------------------------------//

#include <CayenneLPP.h>

void PayloadNow(Stream &out)
{
  CayenneLPP lpp(64);
  lpp.addVoltage(1, getVcc());

  uint32_t id = medidor_id();
  if (id)
  {
    lpp.addGenericSensor(1, id);
    float energy = medidor_energy();
    float inductive = medidor_inductive();
    float capacitive = medidor_capacitive();
    float reverse = medidor_reverse();
    
    lpp.addEnergy(1, energy);
    if (inductive)  lpp.addEnergy(2, inductive);
    if (capacitive)  lpp.addEnergy(3, capacitive);
    if (reverse)  lpp.addEnergy(4, reverse);
  }
  out.write(lpp.getBuffer(), lpp.getSize());
}


//----------------------------------------//
// getVcc
//----------------------------------------//

double getVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high << 8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  double t = result / 1000.0;
  return t;
}
