// HPSystem bus (HP 1050 HPLC modules and possibly others)
// All lines are open collector, i.e. any module can pull the line low, and the line is only high when no module is pulling it low
// Some pins are active high, and some are active low, in order to create wired or / and conditions as appropriate.
// When the modules are set to GLOBAL mode, the startreq line is not used and start pulses are output.
// When the modules are set to HPSystem mode, pressing start on any module will output a startreq pulse.
//      This causes the autoinjector to begin the process of injecting a sequence of samples. The autoinjector will send start pulses
//      to tell the other modules to start the run for this chromatogram (e.g. will cause the pump to begin the programmed gradient timetable)

#include "hpsystem.h"

HPSystem::HPSystem(uint8_t poweron_pin, uint8_t preparerun_pin, uint8_t ready_pin, uint8_t start_pin, uint8_t stop_pin, uint8_t shutdown_pin, uint8_t startreq_pin) {
  _poweron = poweron_pin;
  _preparerun = preparerun_pin;
  _ready = ready_pin;
  _start = start_pin;
  _stop = stop_pin;
  _shutdown = shutdown_pin;
  _startreq = startreq_pin;
  pinMode(_poweron,    INPUT_PULLUP);
  pinMode(_preparerun, INPUT_PULLUP);
  pinMode(_ready,      INPUT_PULLUP);
  pinMode(_start,      INPUT_PULLUP);
  pinMode(_stop,       INPUT_PULLUP);
  pinMode(_shutdown,   INPUT_PULLUP);
  pinMode(_startreq,   INPUT_PULLUP);
}
uint8_t HPSystem::get_line_pin(enum Line line) {
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
void HPSystem::pulse_line(enum Line line, uint16_t duration) {
  assert_line(line);
  delay(duration);
  release_line(line);
}
bool HPSystem::read_line(enum Line line) {
  int pin = get_line_pin(line);
  return digitalRead(pin);
}

void HPSystem::startreq() {
  pulse_line(STARTREQ, 150);
}

void HPSystem::shutdown() {
  pulse_line(SHUTDOWN, 150);
}
bool HPSystem::isShutdown() {
  return (!read_line(SHUTDOWN));
}


char* HPSystem::getFlagString(char *buf) { // Buffer must be minimum 8 chars
  for (uint8_t i = 0; i < 8; i++) {
    buf[i] = 0;
  }

  uint8_t index = 0;
  if (!read_line(HPSystem::POWERON)) {
    buf[index++] = 'P';
  }
  if (!read_line(HPSystem::SHUTDOWN)) {
    buf[index++] = 'X';
  }
  if (!read_line(HPSystem::READY)) {
    buf[index++] = 'N';
  }

  if (!read_line(HPSystem::STARTREQ)) {
    buf[index++] = 'r';
  }
  if (!read_line(HPSystem::PREPARERUN)) {
    buf[index++] = 'p';
  }
  if (!read_line(HPSystem::START)) {
    buf[index++] = 'b';
  }
  if (!read_line(HPSystem::STOP)) {
    buf[index++] = 'e';
  }
  buf[index++] = 0;
  return buf;
}
