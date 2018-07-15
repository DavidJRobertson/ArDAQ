#include "hsm.h"
#include <Arduino.h>

void HSM::Idle::onEnter(HSM &hsm) {
  Serial.println("Entering Idle");
}
