#ifndef ADS1232_H
#define ADS1232_H

#include <Arduino.h>

class ADS1232 {
  public:
    ADS1232(int pdwn_pin, int dout_pin, int sclk_pin);
    void init();
    void reset();
    void enable();
    void disable();
    void offset_calibration();
    unsigned long read_blocking();
    bool ready();
    
  private:
    int _dout;
    int _sclk;
    int _pdwn;
};

#endif
