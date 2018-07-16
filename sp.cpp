/*
// STREQ STATE
void st_streq_enter() {
  Serial.println("Entering st_streq");
  Serial.println("# Sending start request...");
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
  Serial.println("Entering st_prep or st_prepim");
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
*/
