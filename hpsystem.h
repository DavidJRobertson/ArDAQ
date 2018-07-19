#ifndef HPSYSTEM_H
#define HPSYSTEM_H

#include <Arduino.h>

class HPSystem {
  public:
    enum Line{
      POWERON = 1,      // High when ALL connected modules are powered on. Low when ANY connected module is powered off.
      PREPARERUN = 2,   // Active low
      READY = 3,        // High when ALL powered modules are in the ready state. Low when ANY powered module is in the not ready state (e.g. pump pressure not stable).
      START = 4,        // Pulse sent by autosampler at start of run
      STOP = 5,         // Pulse sent by autosampler at end of run
      SHUTDOWN = 6,     // High during normal operation. Low when ANY connected module experiences a serious fault (e.g. if a leak is detected). Stops the pump!
      STARTREQ = 7,     // Send a pulse on this line to ask the autosampler to begin
    };

    HPSystem(uint8_t poweron_pin, uint8_t preparerun_pin, uint8_t ready_pin, uint8_t start_pin, uint8_t stop_pin, uint8_t shutdown_pin, uint8_t startreq_pin);

    void assert_line(enum Line line);
    void release_line(enum Line line);
    void set_line(enum Line line, bool value);
    void pulse_line(enum Line line, uint16_t delay);
    bool read_line(enum Line line);

    char* getFlagString(char* buf); // buffer must be min. 8 chars long

    void startreq();
    void shutdown();
    bool isShutdown();

  private:
    uint8_t get_line_pin(enum Line line);
    uint8_t _poweron;
    uint8_t _preparerun;
    uint8_t _ready;
    uint8_t _start;
    uint8_t _stop;
    uint8_t _shutdown;
    uint8_t _startreq;
};

#endif
