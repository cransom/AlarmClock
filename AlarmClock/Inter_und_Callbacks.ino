void beepEnabler() {
  beepEnable = !beepEnable;
  showDisplayFlag = true;
}

void setWeckZeit() {
  if (!digitalRead(encoderA)) {
    if (alarmFlag) {
      if (digitalRead(encoderB)) Wmin++;
        else Wmin--;
      checkFormat(Wstd,Wmin);
    } //alarmFlag
    else {
      watchFace++;       
    } //!alarmFlag
    showDisplayFlag = true; 
    eTimer = millis();
    write2EEPROM = true;
  } //!encoderA
} //setWeckZeit

void alarmSwap() {
  alarmFlag = !alarmFlag;
  showDisplayFlag = true;
  eTimer = millis();
  write2EEPROM = true;
}

void configModeCallback (WiFiManager *myWiFiManager) {
  display.clear();
  display.drawString(0, 20, "Enter WiFi Data");
  display.display();
}

void setReadyForWeatherUpdate() {
  readyForWeatherUpdate = true;
}
