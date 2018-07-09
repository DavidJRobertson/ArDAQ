// HPSystem bus (HP 1050 HPLC modules and possibly others)
// All lines are open collector, i.e. any module can pull the line low, and the line is only high when no module is pulling it low
// Some pins are active high, and some are active low, in order to create wired or / and conditions as appropriate.
// When the modules are set to GLOBAL mode, the startreq line is not used and start pulses are output. 
// When the modules are set to HPSystem mode, pressing start on any module will output a startreq pulse. 
//      This causes the autoinjector to begin the process of injecting a sequence of samples. The autoinjector will send start pulses
//      to tell the other modules to start the run for this chromatogram (e.g. will cause the pump to begin the programmed gradient timetable)

#include "hpsystem.h"

HPSystem::HPSystem(int poweron_pin, int preparerun_pin, int ready_pin, int start_pin, int stop_pin, int shutdown_pin, int startreq_pin) {
  _poweron = poweron_pin;
  _preparerun = preparerun_pin;
  _ready = ready_pin;
  _start = start_pin;
  _stop = stop_pin;
  _shutdown = shutdown_pin;
  _startreq = startreq_pin;
  pinMode(_poweron,    INPUT);
  pinMode(_preparerun, INPUT);
  pinMode(_ready,      INPUT);
  pinMode(_start,      INPUT);
  pinMode(_stop,       INPUT);
  pinMode(_shutdown,   INPUT);
  pinMode(_startreq,   INPUT);
}

int HPSystem::get_line_pin(enum Line line) {
  switch (line) {
    case POWERON: return _poweron;
    case PREPARERUN: return _preparerun;
    case READY: return _ready;
    case START: return _start;
    case STOP: return _stop;
    case SHUTDOWN: return _shutdown;
    case STARTREQ: return _startreq;
  }
}

void HPSystem::assert_line(enum Line line) {
  int pin = get_line_pin(line);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}
void HPSystem::release_line(enum Line line) {
  int pin = get_line_pin(line);
  pinMode(pin, INPUT);
}
void HPSystem::set_line(enum Line line, bool value) {
  if (value) {
    assert_line(line);
  } else {
    release_line(line);
  }
}
void HPSystem::pulse_line(enum Line line, int duration) {
  assert_line(line);
  delay(duration);
  release_line(line);
}

bool HPSystem::read_line(enum Line line) {
  int pin = get_line_pin(line);
  return digitalRead(pin);
}
