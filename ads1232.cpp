// Stuff relating to the ADS 1232 ADC
// 24 bit adc

#include "ads1232.h"

ADS1232::ADS1232(int pdwn_pin, int dout_pin, int sclk_pin) {
  _pdwn = pdwn_pin;
  _dout = dout_pin;
  _sclk = sclk_pin;
}

void ADS1232::init() {
  pinMode(_pdwn, OUTPUT);
  pinMode(_sclk, OUTPUT);
  pinMode(_dout, INPUT);
  digitalWrite(_sclk, LOW);
  reset();
  offset_calibration();
}

void ADS1232::reset() {
  disable();
  //delay(20);
  enable();
}
void ADS1232::enable() {
  digitalWrite(_pdwn, HIGH);
}
void ADS1232::disable() {
  digitalWrite(_pdwn, LOW);
}
void ADS1232::offset_calibration() {
  // perform offset calibration by toggling the clock an extra time after reading the adc
  read_blocking();
  digitalWrite(_sclk, HIGH); 
  digitalWrite(_sclk, LOW);
}
bool ADS1232::ready() {
  return (digitalRead(_dout) == LOW);
}
unsigned long ADS1232::read_blocking() { 
  while (!ready()) { 
    // wait for conversion to complete
  }
  unsigned long b1 = shiftIn(_dout, _sclk, MSBFIRST);
  unsigned long b2 = shiftIn(_dout, _sclk, MSBFIRST);
  unsigned long b3 = shiftIn(_dout, _sclk, MSBFIRST);

  // toggle clock once to reset the dout line to the idle state
  digitalWrite(_sclk, HIGH); 
  digitalWrite(_sclk, LOW);

  // format the data  
  return (b1 << 16) | (b2 << 8) | b3;
}

