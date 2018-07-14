// ArDAQ: Arduino based data aquisition system for the HP 1050 HPLC
// David Robertson

// Pin Mapping
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
#define RUN_LED_PIN 13

// Includes
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include "hpsystem.h"
#include "ads1232.h"
#include "FiniteStateMachine.h"
RTC_DS1307 rtc;
HPSystem hp(HP_POWERON_PIN, HP_PREPARERUN_PIN, HP_READY_PIN, HP_START_PIN, HP_STOP_PIN, HP_SHUTDOWN_PIN, HP_STARTREQ_PIN);
ADS1232 adc(ADC_PDWN_PIN, ADC_DOUT_PIN, ADC_SCLK_PIN);


int sampleNumber;
unsigned long startTime;

// State functions
State noopState = State();
FSM fsm(noopState);
void st_idle_update();
void st_streq_enter();
void st_streq_update();
void st_streq_exit();
void st_prep_enter();
void st_prep_update();
void st_prepim_update();
void st_run_enter();
void st_run_update();
void st_run_exit();
void st_post_enter();
State st_idle   = State(NULL, st_idle_update, NULL);
State st_streq  = State(st_streq_enter, st_streq_update, st_streq_exit);
State st_prep   = State(st_prep_enter, st_prep_update,   NULL);
State st_prepim = State(st_prep_enter, st_prepim_update, NULL);
State st_run    = State(st_run_enter, st_run_update, st_run_exit);
State st_post   = State(st_post_enter, NULL, NULL);
bool manualOverride = false;

// IDLE STATE
void st_idle_update() {
  manualOverride=false;
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();
    switch (tolower(incomingByte)) {
      case 's': manualOverride=true; fsm.transitionTo(st_prepim); break;
      case 'r': fsm.transitionTo(st_streq); break;
      case 'x': hp.shutdown(); break;
    }
  }
  if (hp.read_line(HPSystem::POWERON)) {
    if (!hp.read_line(HPSystem::START)) {
      fsm.transitionTo(st_prepim);
    } else if (!hp.read_line(HPSystem::STARTREQ)) {
      fsm.transitionTo(st_prep);
    }
  }
}

// STREQ STATE
void st_streq_enter() {
  startTime = millis();
  hp.assert_line(HPSystem::STARTREQ);
}
void st_streq_update() {
  int delay = 150;
  if ( (millis()-startTime) >= delay) {
    fsm.transitionTo(st_prep);
  }
}
void st_streq_exit() {
  hp.release_line(HPSystem::STARTREQ);
}

// PREP/PREPIM STATE
void st_prep_enter() { // used for prepim as well
  adc.enable();
  adc.offset_calibration();
  sampleNumber = 0;
}
void st_prep_update() {
  if (!hp.read_line(HPSystem::START)) {
    fsm.transitionTo(st_run);
  }
}
void st_prepim_update() {
  fsm.transitionTo(st_run);
}

// RUN STATE
void st_run_enter() {
  startTime = millis();
  digitalWrite(RUN_LED_PIN, HIGH);
}
void st_run_exit() {
  adc.disable();
  digitalWrite(RUN_LED_PIN, LOW);
}
void st_run_update() {
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();
    switch (tolower(incomingByte)) {
      case 's': fsm.transitionTo(st_post); break;
      case 'x': hp.shutdown();   break;
    }
  }
  if (!manualOverride && !hp.read_line(HPSystem::STOP)) {
    fsm.transitionTo(st_post);
  }
  if (adc.ready()) {
    sampleNumber++;
    unsigned long iterationStartTime = millis() - startTime;
    unsigned long adcval = adc.read_blocking();

    Serial.print(sampleNumber);
    Serial.print("\t");

    Serial.print(iterationStartTime/(1000.0*60), 4);
    Serial.print("\t");

    // Output the raw ADC value
    Serial.print(adcval);
    Serial.print("\t");

    // System Status Flags column
    Serial.print(hp.getFlagString());
    Serial.print("\t");

    // Send the newline
    Serial.println();
  }
}

// POST RUN STATE
void st_post_enter() {
  fsm.transitionTo(st_idle);
}

//State st_shutdown = State();

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

  // FSM
  fsm.transitionTo(st_idle);
}

void loop() {
  fsm.update();
}

void sd_log() {
  Serial.print("# Initializing SD card...");
  if (SD.begin(SD_CS_PIN)) {
    Serial.println("OK.");
  } else {
    Serial.println("Card failed, or not present");
  }

  String dataString = "";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }

  // open the file. note that only one file can be open at a time
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  } else {
    Serial.println("## SD LOGGING ERROR");
  }

}
