#ifndef ADS1232_H
#define ADS1232_H

#include <Arduino.h>

class ADS1232 {
  public:
    ADS1232(uint8_t pdwn_pin, uint8_t dout_pin, uint8_t sclk_pin);
    void init();
    void reset();
    void enable();
    void disable();
    void offset_calibration();
    uint32_t read_blocking();
    bool ready();
    
  private:
    uint8_t _dout;
    uint8_t _sclk;
    uint8_t _pdwn;
};

#endif
