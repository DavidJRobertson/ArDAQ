#include "hsm.h"
#include <Arduino.h>

// State instances (needed to make the linker happy)
HSM::State HSM::State::instance;
HSM::Init  HSM::Init::instance;
HSM::Idle  HSM::Idle::instance;
HSM::Run   HSM::Run::instance;

// HIERARCHICHAL STATE MACHINE METHODS
HSM::HSM() {
  currentState = &HSM::Init::instance;
  currentState->onEnter(*this, *currentState);
  currentState->onInit(*this, *currentState);
}
void HSM::transitionTo(HSM::State &newState) {
  currentState->onExit(*this, newState);
  for (State *cursor = currentState->getParentInstance(); cursor && !newState.isDescendantOf(cursor); cursor = cursor->getParentInstance()) {
    cursor->onExit(*this, newState);
  }

  State *oldState = currentState;
  currentState = &newState;


  int8_t todo = -1;
  for (State *cursor = &newState; cursor && !oldState->isDescendantOf(cursor); cursor = cursor->getParentInstance()) {
    todo++;
  }
  for (uint8_t i = todo; i > 0; i--) {
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

// INIT
void HSM::Init::onInitDone(HSM &hsm) {
  hsm.transitionTo(HSM::Idle::instance);
}

// IDLE
void HSM::Idle::onEnter(HSM &hsm, HSM::State &fromState) {
  Serial.println("Entering Idle");
}
void HSM::Idle::onExit(HSM &hsm, HSM::State &toState) {
  Serial.println("Exiting Idle");
}
void HSM::Idle::onUpdate(HSM &hsm) {
  Serial.println(".");
  delay(1000);
}
void HSM::Idle::onSignalStart(HSM &hsm) {
  hsm.transitionTo(HSM::Run::instance);
}
void HSM::Idle::onSignalStartRequest(HSM &hsm) {
  hsm.transitionTo(HSM::Run::instance);
}


#define RUN_LED_PIN 13
// RUN
void HSM::Run::onEnter(HSM &hsm, HSM::State &fromState) {
  Serial.println("Entering Run");
  //startTime = millis();
  digitalWrite(RUN_LED_PIN, HIGH);
}
void HSM::Run::onExit(HSM &hsm, HSM::State &toState) {
  //adc.disable();
  digitalWrite(RUN_LED_PIN, LOW);
  Serial.println("Exiting Run");
}
void HSM::Run::onSignalStop(HSM &hsm) {
  hsm.transitionTo(HSM::Idle::instance);
}
void HSM::Run::onUpdate(HSM &hsm) {
  Serial.println(":");
  delay(1000);
}

void HSM::Sample::onEnter(HSM &hsm, HSM::State &fromState) {
  Serial.println("Entering Run>Sample");
}
void HSM::Sample::onExit(HSM &hsm, HSM::State &toState) {
  Serial.println("Exiting Run>Sample");
}
