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
#include "hsm.h"
HSM hsm;
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
  hsm.onUpdate();
  hsm.onUpdate();
  hsm.onUpdate();

  hsm.onSignalStart();
  hsm.onUpdate();
  hsm.onUpdate();
  hsm.onUpdate();
  hsm.onSignalStop();
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
