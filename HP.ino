void checkFormat(int &s, int &m) {
  if (m > 59){
    m = m - 60;
    s++;  
  }
  if (m < 0) {
    m = m + 60;
    s--;
  }
  if (s > 23) s = s - 24;
  if (s < 0) s = s +24;
}

void displayScreen() {
  if (watchFace > (numberofWatchfaces - 1)) watchFace = 0;
  if (watchFace < 0) watchFace = numberofWatchfaces - 1;
  display.clear();
  Zeit = timeClient.getFormattedTime();  
  switch (watchFace) {
    case wFDigital:
      display.setContrast(lowContrast);
      if (alarmFlag) {
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(Roboto_Condensed_Light_50);
        display.drawString(0, -5, Zeit.substring(0,5));
        display.setFont(Roboto_Condensed_20);
        if (Wstd < 10) Wzeit = "0" + String(Wstd);
          else Wzeit = String(Wstd);
        if (Wmin < 10) Wzeit = Wzeit + ":" + "0" + String(Wmin);
          else Wzeit = Wzeit + ":" + String(Wmin);
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        display.drawString(rightBound, 45, Wzeit);
      } //alarmFlag
      else {
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(Roboto_Condensed_Light_58);
        display.drawString(0, 0, Zeit.substring(0,5));
      }
      break; //wFDigital
    case wFAnalog:
      display.setContrast(highContrast);
      displayAnalogClock();
      if (alarmFlag) {
        display.setFont(Roboto_Condensed_Light_20);
        if (Wstd < 10) Wzeit = "0" + String(Wstd);
          else Wzeit = String(Wstd);
        if (Wmin < 10) Wzeit = Wzeit + ":" + "0" + String(Wmin);
          else Wzeit = Wzeit + ":" + String(Wmin);
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        display.drawString(rightBound, 45, Wzeit);
      }
      break; //wFAnalog
    case wFDigitalmitWetter:
      display.setContrast(lowContrast);
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.setFont(Roboto_Condensed_Light_50);
      display.drawString(64, -5, Zeit.substring(0,5));
      if (alarmFlag) {  
        display.setFont(Roboto_Condensed_20);
        if (Wstd < 10) Wzeit = "0" + String(Wstd);
          else Wzeit = String(Wstd);
        if (Wmin < 10) Wzeit = Wzeit + ":" + "0" + String(Wmin);
          else Wzeit = Wzeit + ":" + String(Wmin);
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        display.drawString(rightBound, 45, Wzeit);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        String temp = wunderground.getCurrentTemp() + "°C";
        int ICP = display.getStringWidth(temp);
        display.drawString(0,45,temp);
        display.setFont(Meteocons_Plain_21);
        String weatherIcon = wunderground.getTodayIcon();
        display.drawString(ICP +4,44,weatherIcon); 
       } //alarmFlag
      else {
        display.setFont(Roboto_Condensed_20);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        String temp = wunderground.getCurrentTemp() + "°C";
        int ICP = display.getStringWidth(temp);
        ICP = (ICP +25)/2;
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.drawString(64-ICP,45,temp);
        display.setFont(Meteocons_Plain_21);
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        String weatherIcon = wunderground.getTodayIcon();
        display.drawString(64 + ICP,44,weatherIcon); 
      }
      break; //wFDigitalmitWetter

  
  } // switchcase
  display.display();
}


void displayAnalogClock() {
  int screenW = 128;
  int screenH = 64;
  int clockCenterX = 32; //screenW/2;
  int clockCenterY = (screenH/2);   
  int clockRadius = 32;
  int x =0;
  int y =0;
  int x2,x3,y2,y3;
  float angle;
  // display.drawRect(0,0,128,64);
 
  // Weather
  display.setFont(Meteocons_Plain_21);
  String weatherIcon = wunderground.getTodayIcon();
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(rightBound,2, weatherIcon); 
  display.setFont(Roboto_Condensed_Light_16);
  String temp = wunderground.getCurrentTemp() + "°C";
  display.drawString(rightBound,24, temp);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  // Alalog watch
  display.fillCircle(clockCenterX, clockCenterY, 2);
    //hour ticks
  for( int z=0; z < 360;z= z + 30 ){
  //Begin at 0° and stop at 360°
    angle = z ;
    angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
    x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    display.drawLine( x2 + x , y2 + y , x3 + x , y3 + y);
  }
      // display minute hand
  angle = timeClient.getMinutes() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  display.drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display hour hand
  angle = timeClient.getHours() * 30 + int( ( timeClient.getMinutes() / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  display.drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
}

