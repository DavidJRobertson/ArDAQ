// ArDAQ: Arduino based data aquisition system for the HP 1050 HPLC
// David Robertson

// HPSystem remote control interface
#define HP_POWERON_PIN    2 
#define HP_PREPARERUN_PIN 3 
#define HP_READY_PIN      4 
#define HP_START_PIN      5 
#define HP_STOP_PIN       6
#define HP_SHUTDOWN_PIN   7 
#define HP_STARTREQ_PIN   8 
#include "hpsystem.h"
HPSystem hp(HP_POWERON_PIN, HP_PREPARERUN_PIN, HP_READY_PIN, HP_START_PIN, HP_STOP_PIN, HP_SHUTDOWN_PIN, HP_STARTREQ_PIN);

// SD Card
#define SD_CS_PIN 10
#include <SPI.h>
#include <SD.h>

// DS1307 Real Time Clock
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;

// ADC: ADS1232
// other digital inputs were hardwired to appropriate values (10sps), see the datasheet 
#define ADC_PDWN_PIN A0
#define ADC_DOUT_PIN A1
#define ADC_SCLK_PIN A2
#include "ads1232.h"
ADS1232 adc(ADC_PDWN_PIN, ADC_DOUT_PIN, ADC_SCLK_PIN);

// Running LED
#define RUN_LED_PIN 13


#include <Fsm.h>


// Global variables
bool running = false;
unsigned long startTime = 0;
unsigned long sampleNumber = 0;

enum state {
  ST_IDLE=0,
  ST_RUN=1
};
enum state currentState = ST_IDLE;

// Initialization
void setup() {  
  pinMode(RUN_LED_PIN, OUTPUT);
  digitalWrite(RUN_LED_PIN, LOW);
  
  Serial.begin(115200);
  Serial.println();
  Serial.println("# ArDAQ starting");

  rtc_init();
  sd_init();
  adc.init();
  adc.disable(); 
  
  Serial.println("#    Commands: s=stop, z=zero/start, r=hpstartreq, x=hpshutdown");
}

// Main loop


void loop() {
  enum state nextState = currentState;
  switch (currentState) {
    case ST_IDLE:
      handleSerialCommands();
      if (hp.read_line(HPSystem::POWERON) && !hp.read_line(HPSystem::START)) {
        nextState = ST_IDLE;
      }
      break;

    case ST_RUN:
      handleSerialCommands();
      if (!hp.read_line(HPSystem::STOP)) {
        nextState = ST_IDLE;
      }
      takesample();
      break;
  }
  currentState = nextState;
}



void takesample() {
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

  // Output HPSystem line status
  Serial.print(hp.read_line(HPSystem::POWERON));
  Serial.print(hp.read_line(HPSystem::PREPARERUN));
  Serial.print(hp.read_line(HPSystem::READY));
  Serial.print(hp.read_line(HPSystem::START));
  Serial.print(hp.read_line(HPSystem::STOP));
  Serial.print(hp.read_line(HPSystem::SHUTDOWN));
  Serial.print(hp.read_line(HPSystem::STARTREQ));
  Serial.print("\t");

  // System Status Flags column
  if (!hp.read_line(HPSystem::POWERON)) {
    Serial.print('P');
  }
  if (!hp.read_line(HPSystem::SHUTDOWN)) {
    Serial.print('X');
  }
  if (!hp.read_line(HPSystem::READY)) {
    Serial.print('N');
  }
  Serial.print("\t");

  // Event Flags column
  if (!hp.read_line(HPSystem::STARTREQ)) {
    Serial.print('r'); 
  }
  if (!hp.read_line(HPSystem::PREPARERUN)) {
    Serial.print('p');
  }
  if (!hp.read_line(HPSystem::STOP)) {
    Serial.print('b'); //begin
  }
  if (!hp.read_line(HPSystem::START)) {
    Serial.print('e'); //end
  }
  Serial.print("\t");

  // Send the newline
  Serial.println();
}


// SERIAL COMMANDS
void handleSerialCommands() {
  // Check for serial commands
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();
    switch (tolower(incomingByte)) {
      case 's': commandStop();                            break;
      case 'z': commandStartZero();                       break;
      case 'r': hp.pulse_line(HPSystem::STARTREQ, 150);   break;
      case 'x': hp.pulse_line(HPSystem::SHUTDOWN, 500);   break;
    }
    Serial.flush();
  }
}

void commandStop() {
  running = false;
  adc.disable();
  digitalWrite(RUN_LED_PIN, LOW);
}

void commandStartZero() { // zero timestamp column or begin if stopped
  running = true;
  adc.enable();
  adc.offset_calibration();
  sampleNumber = 0;
  startTime = millis(); 
  digitalWrite(RUN_LED_PIN, HIGH);
}



void rtc_init() {
  rtc.begin();
  if (! rtc.isrunning()) {
    Serial.println("# RTC not already running, initializing to compile timestamp");    
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  // rtc.now();
}



void sd_init() {
  Serial.print("# Initializing SD card...");
  if (SD.begin(SD_CS_PIN)) {
    Serial.println("OK.");
  } else {
    Serial.println("Card failed, or not present");
  }

  
}



void sd_log() {
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
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("## SD LOGGING ERROR");
  }
}

