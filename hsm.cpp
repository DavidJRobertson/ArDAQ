#include "hsm.h"
#include <SD.h>
#include <Arduino.h>

// State instances (needed to make the linker happy)
HSM::State              HSM::State::instance;
HSM::Init               HSM::Init::instance;
HSM::SendStartRequest   HSM::SendStartRequest::instance;
HSM::Idle               HSM::Idle::instance;
HSM::Run                HSM::Run::instance;
HSM::Sample             HSM::Sample::instance;
HSM::WaitForConversion  HSM::WaitForConversion::instance;

// HIERARCHICHAL STATE MACHINE METHODS
HSM::HSM(HPSystem &_hp, ADS1232 &_adc, RTC_DS1307 &_rtc, uint8_t _ledPin, uint8_t _sdCsPin) {
  hp = &_hp;
  adc = &_adc;
  rtc = &_rtc;
  ledPin = _ledPin;
  sdCsPin = _sdCsPin;
  currentState = &HSM::Init::instance;
  currentState->onEnter(*this, *currentState);
  currentState->onInit(*this, *currentState);
}
void HSM::transitionTo(HSM::State &newState) {
  for (State *cursor = currentState; cursor && !newState.isDescendantOf(cursor); cursor = cursor->getParentInstance()) {
    cursor->onExit(*this, newState);
  }

  State *oldState = currentState;
  currentState = &newState;


  int8_t todo = -1;
  for (State *cursor = (&newState)->getParentInstance(); cursor && !oldState->isDescendantOf(cursor); cursor = cursor->getParentInstance()) {
    todo++;
  }
  for (int8_t i = todo; i > 0; i--) {
    State *cursor = &newState;
    for (uint8_t j = 0; j < i; j++) {
      cursor = cursor->getParentInstance();
    }
    cursor->onEnter(*this, *oldState);
  }
  newState.onEnter(*this, *oldState);

  newState.onInit(*this, *oldState);
}
bool HSM::State::isDescendantOf(HSM::State *s) {
  State *cursor = this;
  while (cursor = cursor->getParentInstance()) {
    if (cursor == s) { return true; }
  }
  return false;
}
void HSM::debugPrintln(const char *str) {
  if (debug) {
    messagePrintln(str);
  }
}
void HSM::messagePrintln(const char *str) {
  DateTime now = rtc->now();
  printf("#\t%04d/%02d/%02d %02d:%02d:%02d\t%s\r\n", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second(), str);
}

// Init
void HSM::Init::onInitDone(HSM &hsm) {
  hsm.messagePrintln("ArDAQ Started");
  hsm.transitionTo(HSM::Idle::instance);
}

// Idle
void HSM::Idle::onEnter(HSM &hsm, HSM::State &fromState) {
  hsm.messagePrintln("Idle (commands: s=start acq., r=send start req., x=send shutdown)");
  hsm.debugPrintln("Entering Idle");
}
void HSM::Idle::onExit(HSM &hsm, HSM::State &toState) {
  hsm.debugPrintln("Exiting Idle");
}
void HSM::Idle::onSignalStart(HSM &hsm) {
  hsm.messagePrintln("HPSystem: start signal received");
  hsm.transitionTo(HSM::Run::instance);
}
void HSM::Idle::onSerialAvailable(HSM &hsm) {
  char incomingByte = Serial.read();
  switch (tolower(incomingByte)) {
    case 's':
      hsm.transitionTo(HSM::Run::instance);
      break;
    case 'r':
      hsm.transitionTo(HSM::SendStartRequest::instance);
      break;
    case 'x':
      hsm.hp->shutdown();
      break;
  }
}
void HSM::Idle::onSignalNotReady(HSM &hsm) {
  hsm.messagePrintln("HPSystem: Not Ready!");
}
void HSM::Idle::onSignalReady(HSM &hsm) {
  hsm.messagePrintln("HPSystem: Ready.");
}
void HSM::Idle::onSignalPowerOff(HSM &hsm) {
  hsm.messagePrintln("HPSystem: Check module power!");
}
void HSM::Idle::onSignalPowerOn(HSM &hsm) {
  hsm.messagePrintln("HPSystem: Module power OK.");
}

// SendStartRequest
void HSM::SendStartRequest::onInit(HSM &hsm, HSM::State &fromState) {
  hsm.messagePrintln("Sending start request...");
  hsm.hp->startreq();
  hsm.transitionTo(HSM::Idle::instance);
}

// Run
void HSM::Run::onEnter(HSM &hsm, HSM::State &fromState) {
  hsm.debugPrintln("Entering Run");
  hsm.adc->enable();
  hsm.adc->offset_calibration();
  hsm.sampleNumber = 0;
  hsm.messagePrintln("Run started (commands: s=stop acq., x=send shutdown)");
  hsm.startTime = millis();
  digitalWrite(hsm.ledPin, HIGH);
}
void HSM::Run::onExit(HSM &hsm, HSM::State &toState) {
  hsm.debugPrintln("Exiting Run");
  hsm.messagePrintln("Run ended");
  hsm.adc->disable();
  digitalWrite(hsm.ledPin, LOW);
}
void HSM::Run::onInit(HSM &hsm, HSM::State &fromState) {
  hsm.transitionTo(HSM::WaitForConversion::instance);
}
void HSM::Run::onSignalStop(HSM &hsm) {
  hsm.messagePrintln("HPSystem: stop signal received");
  hsm.transitionTo(HSM::Idle::instance); //// TODO postrun
}
void HSM::Run::onSerialAvailable(HSM &hsm) {
  char incomingByte = Serial.read();
  switch (tolower(incomingByte)) {
    case 's':
      hsm.transitionTo(HSM::Idle::instance);
      break;
    case 'x':
      hsm.hp->shutdown();
      hsm.transitionTo(HSM::Idle::instance);
      break;
  }
}
void HSM::Run::onSignalNotReady(HSM &hsm) {
  hsm.messagePrintln("HPSystem: Not Ready!");
}
void HSM::Run::onSignalReady(HSM &hsm) {
  hsm.messagePrintln("HPSystem: Ready. ");
}
void HSM::Run::onSignalPowerOff(HSM &hsm) {
  hsm.messagePrintln("HPSystem: Check module power!");
}
void HSM::Run::onSignalPowerOn(HSM &hsm) {
  hsm.messagePrintln("HPSystem: Module power OK. ");
}

// Run > WaitForConversion
void HSM::WaitForConversion::onEnter(HSM &hsm, HSM::State &fromState) {
  hsm.debugPrintln("Entering Run > WaitForConversion");
}
void HSM::WaitForConversion::onExit(HSM &hsm, HSM::State &toState) {
  hsm.debugPrintln("Exiting Run > WaitForConversion");
}
void HSM::WaitForConversion::onAdcDataReady(HSM &hsm) {
  hsm.transitionTo(HSM::Sample::instance);
}

// Run > Sample
void HSM::Sample::onEnter(HSM &hsm, HSM::State &fromState) {
  hsm.debugPrintln("Entering Run > Sample");
}
void HSM::Sample::onExit(HSM &hsm, HSM::State &toState) {
  hsm.debugPrintln("Exiting Run > Sample");
}
void HSM::Sample::onInit(HSM &hsm, HSM::State &fromState) {
  // Take the sample
  int32_t adcval = hsm.adc->read_blocking();
  uint32_t sampleTime = millis() - hsm.startTime;
  hsm.sampleNumber++;

  // Print it out
  float timeMins = sampleTime/(1000.0*60);
  char timeBuf[24];
  dtostrf(timeMins, 0, 5, timeBuf);

  char flagBuf[8];
  hsm.hp->getFlagString(flagBuf);

  // ref = 2.470v = +-1.235v
  float adcMilliVolts = adcval * 1235.0/8388608;
  char milliVoltsBuf[16];
  dtostrf(adcMilliVolts, 10, 4, milliVoltsBuf);

  printf("%" PRId32 "\t%s\t%s\t%" PRId32 "\t%s\r\n", hsm.sampleNumber,  &timeBuf, &milliVoltsBuf, adcval, &flagBuf);

  // Then wait for the next conversion to complete
  hsm.transitionTo(HSM::WaitForConversion::instance);
}


bool HSM::sd_log(const char* logEntry) {
  debugPrintln("# in sd_log()");
  if (SD.begin(sdCsPin)) {
    messagePrintln("SD card initialized");
  } else {
    return false; // card not present/usable
  }

  // open the file. note that only one file can be open at a time
  File dataFile = SD.open("ardaq.log", FILE_WRITE);
  if (! dataFile) {
    messagePrintln("SD LOGGING ERROR");
    return false;
  }

  dataFile.println(logEntry);
  dataFile.close();
  Serial.println(logEntry);

  return true;
}
