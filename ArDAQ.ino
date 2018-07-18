// ArDAQ: Arduino based data aquisition system for the HP 1050 HPLC
// David Robertson


// Pins
#define HP_POWERON_PIN    2
#define HP_PREPARERUN_PIN 3
#define HP_READY_PIN      4
#define HP_START_PIN      5
#define HP_STOP_PIN       6
#define HP_SHUTDOWN_PIN   7
#define HP_STARTREQ_PIN   8
#define SD_CS_PIN         10
#define ADC_PDWN_PIN      A0
#define ADC_DOUT_PIN      A1
#define ADC_SCLK_PIN      A2
#define RUN_LED_PIN       13

// Includes
#include "pinmap.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include "hpsystem.h"
#include "ads1232.h"
#include "hsm.h"
HPSystem hp(HP_POWERON_PIN, HP_PREPARERUN_PIN, HP_READY_PIN, HP_START_PIN, HP_STOP_PIN, HP_SHUTDOWN_PIN, HP_STARTREQ_PIN);
ADS1232 adc(ADC_PDWN_PIN, ADC_DOUT_PIN, ADC_SCLK_PIN);
RTC_DS1307 rtc;
HSM hsm(hp, adc, rtc, RUN_LED_PIN);

// State vars
int sampleNumber;
unsigned long startTime;
bool manualOverride = false;

// Initialization
void setup() {
  // Serial
  Serial.begin(115200);
  Serial.println();
  Serial.println("# ArDAQ starting");

  // LED
  pinMode(RUN_LED_PIN, OUTPUT);
  digitalWrite(RUN_LED_PIN, LOW);

  // RTC init
  rtc.begin();
  if (! rtc.isrunning()) {
    Serial.println("# RTC not already running, initializing to compile timestamp");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // ADC
  adc.init();
  adc.disable();

  // HSM
  hsm.onInitDone(); // go to the idle state
}

void loop() {
  static bool power = true;
  if (power != hp.read_line(HPSystem::POWERON)) {
    power = hp.read_line(HPSystem::POWERON);
    if (power) {
      hsm.onSignalPowerOn();
    } else {
      hsm.onSignalPowerOff();
    }
  }
  static bool ready = false;
  if (ready != hp.read_line(HPSystem::READY)) {
    ready = hp.read_line(HPSystem::READY);
    if (ready) {
      hsm.onSignalReady();
    } else {
      hsm.onSignalNotReady();
    }
  }
  static bool start = false;
  if (start != (!hp.read_line(HPSystem::START))) {
    start = (!hp.read_line(HPSystem::START));
    if (start) {
      hsm.onSignalStart();
    }
  }
  static bool stop  = false;
  if (stop != (!hp.read_line(HPSystem::STOP))) {
    stop = (!hp.read_line(HPSystem::STOP));
    if (stop) {
      hsm.onSignalStop();
    }
  }
  static bool startreq = false;
  if (startreq != (!hp.read_line(HPSystem::STARTREQ))) {
    startreq = (!hp.read_line(HPSystem::STARTREQ));
    if (startreq) {
      hsm.onSignalStartRequest();
    }
  }
  static bool prepare = false;
  if (prepare != (!hp.read_line(HPSystem::PREPARERUN))) {
    prepare = (!hp.read_line(HPSystem::PREPARERUN));
    if (prepare) {
      hsm.onSignalPrepare();
    }
  }
  static bool shutdown = false;
  if (shutdown != (!hp.read_line(HPSystem::SHUTDOWN))) {
    shutdown = (!hp.read_line(HPSystem::SHUTDOWN));
    if (shutdown) {
      hsm.onSignalShutdown();
    }
  }

  if (adc.ready()) {
    hsm.onAdcDataReady();
  }
  if (Serial.available() > 0) {
    hsm.onSerialAvailable();
  }

  hsm.onUpdate();
}
