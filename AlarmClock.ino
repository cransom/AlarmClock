/**The MIT License (MIT)

Copyright 2017, Stefan Ries

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Copyrights of the Libaries used -> see in the respective Tabs
*/

/* Functional description
 *  --- Setup
 *  An account for a free Developer account at https://www.wunderground.com/ must be registerd
 *  This gives acces to an API-Key which needs to be copied into the the code at WUNDERGRROUND_API_KEY
 *  Load the code to the Clock
 *  Power on
 *  The Clock will act as a WiFi Access Point and the WiFi Credentials can be programed by the mobile phone. See https://github.com/tzapu/WiFiManager
 *  THATS IT - The Clock recognize where you are, and adjust the time (incl. Day light saving) to you location
 *  (The first time you might finde some strange wake up times. This is due the fact that watchface and wake up time are store in the EEPROM in case of power fail)
 *  By turning the knob you can choose the watchface
 *  By pressing the knob it switches the alarm on an turning now will adjust the wake up time
 *  
 */



/*
Bill of material:
- Wemos Mini D1
- 0.96" I2C SSD1306 OLED Display
- Rotary encoder code switch EC11
- 5V Active Alarm Buzzer Beeper 9*5.5mm
All material can be sorced by Banggood. Please use this link:
https://www.banggood.com/?p=6R31122484684201508S as this supports the author of the libraries

Wiring:
//Oled - Display 
VCC - VCC
SDA - D2
SCL - D1
GND - GND
//Beeper
GND - GND
VCC - D7
//Rotary Encoder
Left - D5
Right - D6
Middle - D3
*/


#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <Wire.h> 
#include "SSD1306.h"
#include <Ticker.h>
#include "WifiLocator.h"
#include <EEPROM.h>
// Weather
#include <JsonListener.h>
#include "API_Wonder.h"
const boolean IS_METRIC = true;
const String WUNDERGRROUND_API_KEY = "Your Wunderground API";
const String WUNDERGRROUND_LANGUAGE = "EN";
String WUNDERGROUND_COUNTRY;
String WUNDERGROUND_CITY;
const int UPDATE_INTERVAL_SECS = 15 * 60; // Update every 15 minutes
WundergroundClient wunderground(IS_METRIC);
bool readyForWeatherUpdate = false;
Ticker ticker;

#include "Roboto_Condensed_Light_50.h"
#include "Roboto_Condensed_Light_58.h"
#include "Roboto_Condensed_Light_20.h"
#include "Roboto_Condensed_Light_16.h"
#include "Roboto_Condensed_20.h"
#include "WeatherStationFonts.h"
#define rightBound 127

// HOSTNAME for OTA update
#define HOSTNAME "Alarmclock-"

//EEPROM handling
#define ealarmFlag 0
#define ewatchFace 1
#define eWstd 2
#define eWmin 3
unsigned int eTimer;
boolean write2EEPROM = false;
#define eStoredelay 2000

#define encoderA 14
#define encoderB 12
#define beeper 13
#define encoderSwitch 0
#define alarmDuration 3 // Alarm is  munutes active
#define highContrast 255
#define lowContrast 1

int Wstd = 6;
int Wmin = 32;
int lastMin;
String Zeit, Wzeit;

#define numberofWatchfaces 3
#define wFDigital 0
#define wFAnalog 1
#define wFDigitalmitWetter 2
int watchFace = 0;
boolean beepEnable; // By timer
boolean alarmFlag = false; // Alarm active
boolean showDisplayFlag = true;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

SSD1306  display(0x3c, D2,D1);
Ticker alarmModulator;

WifiLocator locator;

void setup(){
  Serial.begin(115200);
  
  display.init();
  display.flipScreenVertically();
  display.setContrast(highContrast);
  display.setFont(Roboto_Condensed_20);
  display.clear();
  display.drawString(0, 20, "Conect WiFi");
  display.display();

  EEPROM.begin(20);
  Wstd = EEPROM.read(eWstd);
  Wmin = EEPROM.read(eWmin);
  watchFace = EEPROM.read(ewatchFace);
  alarmFlag = EEPROM.read(ealarmFlag);
  
  pinMode(encoderA,INPUT_PULLUP);
  pinMode(encoderB,INPUT_PULLUP);
  pinMode(encoderSwitch,INPUT_PULLUP);
  pinMode(beeper,OUTPUT);

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect();
 
  // OTA Setup
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  ArduinoOTA.begin();

  display.clear();
  display.drawString(0, 20, "Conect NTP");
  display.display();

  timeClient.begin();
  timeClient.update();

  locator.updateLocation();
  wunderground.updateLocation(WUNDERGRROUND_API_KEY, locator.getLat(), locator.getLon());
  WUNDERGROUND_COUNTRY = wunderground.getCountry();
  WUNDERGROUND_CITY = wunderground.getCity();
  display.clear();
  display.drawString(0, 20, wunderground.getCity());
  display.display();
  wunderground.updateConditions(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  timeClient.setTimeOffset(wunderground.getTzOffset());

  attachInterrupt(encoderA, setWeckZeit, FALLING);
  attachInterrupt(encoderSwitch, alarmSwap, RISING);

  alarmModulator.attach(1,beepEnabler);
  ticker.attach(UPDATE_INTERVAL_SECS, setReadyForWeatherUpdate);
}

void loop() { 
  ArduinoOTA.handle();
  showDisplayFlag = showDisplayFlag || (lastMin != timeClient.getMinutes());
  if (showDisplayFlag) {
    lastMin = timeClient.getMinutes();
    showDisplayFlag = false;
    displayScreen();
  }
  
  int aktHour = timeClient.getHours();
  int aktMinute = timeClient.getMinutes();
  int alarmHour = Wstd;
  int alarmMinute = Wmin;
  boolean nowAlarm = false;
  for (int i = 0; i < alarmDuration - 1; i++) {
    alarmMinute = alarmMinute + i;
    checkFormat(alarmHour, alarmMinute);
    if ((alarmHour == aktHour) && (alarmMinute == aktMinute)) nowAlarm = true;
  }
  digitalWrite(beeper,nowAlarm && alarmFlag && beepEnable);

  if (readyForWeatherUpdate) {
      wunderground.updateConditions(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
      timeClient.setTimeOffset(wunderground.getTzOffset());
      readyForWeatherUpdate = false;
  }

  if (write2EEPROM) {
    if (millis() >= (eTimer + eStoredelay)) {
      EEPROM.write(eWstd,Wstd);
      EEPROM.write(eWmin,Wmin);  
      EEPROM.write(ewatchFace,watchFace);
      EEPROM.write(ealarmFlag, alarmFlag);
      EEPROM.commit();
      write2EEPROM = false;
    }
  }

}




