#ifndef HPSYSTEM_H
#define HPSYSTEM_H

#include <Arduino.h>

class HPSystem {
  public:
    enum Line{
      POWERON = 1,      // High when ALL connected modules are powered on. Low when ANY connected module is powered off.
      PREPARERUN = 2,   // Active low
      READY = 3,        // High when ALL powered modules are in the ready state. Low when ANY powered module is in the not ready state (e.g. pump pressure not stable).
      START = 4,
      STOP = 5,
      SHUTDOWN = 6,     // High during normal operation. Low when ANY connected module experiences a serious fault (e.g. if a leak is detected). Stops the pump!
      STARTREQ = 7,
    };
  
    HPSystem(int poweron_pin, int preparerun_pin, int ready_pin, int start_pin, int stop_pin, int shutdown_pin, int startreq_pin);

    
    
    void assert_line(enum Line line);
    void release_line(enum Line line);
    void set_line(enum Line line, bool value);
    void pulse_line(enum Line line, int value);
    bool read_line(enum Line line);
    
  private:
    int get_line_pin(enum Line line);
    int _poweron;
    int _preparerun;
    int _ready;
    int _start;
    int _stop;
    int _shutdown;
    int _startreq;
};

#endif
