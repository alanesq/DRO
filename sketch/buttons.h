/* -----------------------------------------------------------------------------------------


          Procedures triggered when a button is pressed on the screen - 18Jan24

          Part of the  "SuperLowBudget-DRO" sketch - https://github.com/alanesq/DRO

          
// -----------------------------------------------------------------------------------------
*/

// on all pages

  // switch page
  void OnePage1Pressed() { 
    log_system_message("button: page 1 selected"); 
    drawScreen(1);      // switch to page 1
  }

  void OnePage2Pressed() { 
    log_system_message("button: page 2 selected"); 
    drawScreen(2);      // switch to page 2
  }

  void OnePage3Pressed() { 
    log_system_message("button: page 3 selected"); 
    drawScreen(3);      // switch to page 3
  }

  void OnePage4Pressed() { 
    log_system_message("button: page 4 selected"); 
    drawScreen(4);      // switch to page 4
  }


// -----------------------------------------------------------------------------------------
// page 1

  //  ------------- zero -----------------
  // zero the current position

  void zeroXpressed() { 
    log_system_message("button: zeroX");
    xAdj[currentCoord] = xReading;    // zero X
  }

  void zeroYpressed() { 
    log_system_message("button: zeroY");
    yAdj[currentCoord] = yReading;    // zero Y
  }

  void zeroZpressed() { 
    log_system_message("button: zeroZ");
    zAdj[currentCoord] = zReading;    // zero Z
  }

  void zAllpressed() { 
    log_system_message("button: zAll");
    xAdj[currentCoord] = xReading;    // zero X, Y and Z
    yAdj[currentCoord] = yReading;     
    zAdj[currentCoord] = zReading;     
  }


  //  ------------- half -----------------
  // half the current displayed value

  void halfXpressed() { 
    log_system_message("button: halfX");
    float t = (xReading - xAdj[currentCoord]) / 2.0;  // half of current displayed reading
    xAdj[currentCoord] = xAdj[currentCoord] + t;      
  }

  void halfYpressed() { 
    log_system_message("button: halfY");
    float t = (yReading - yAdj[currentCoord]) / 2.0;   
    yAdj[currentCoord] = yAdj[currentCoord] + t;   
  }

  void halfZpressed() { 
    log_system_message("button: halfZ");
    float t = (zReading - zAdj[currentCoord]) / 2.0;   
    zAdj[currentCoord] = zAdj[currentCoord] + t;   
  }


  //  ------------- coordinates -----------------
  // switch between the 3 coordinates

  void coord1pressed() { 
    log_system_message("button: C1");
    currentCoord = 0;
    displayReadings();      // make sure the display updates
  }

  void coord2pressed() { 
    log_system_message("button: C2");
    currentCoord = 1;
    displayReadings();     
  }

  void coord3pressed() { 
    log_system_message("button: C3");
    currentCoord = 2;
    displayReadings();     
  }


  // hold current reading
  void oneHoldPressed() {
    log_system_message("button: hold current reading");
    
    // refresh readings from calipers 
      if (!refreshCalipers()) {
        delay(20);
        refreshCalipers();
      }
    
    // store current readings
      float tXreading = xReading;
      float tYreading = yReading;
      float tZreading = zReading;
   
    // display 'HOLD' and wait
      tft.fillScreen(TFT_BLACK);                  // Clear the screen
      tft.setFreeFont(FM9);                       // standard Free Mono font - available sizes: 9, 12, 18 or 24
      tft.setTextSize(2);                         // 1 or 2
      const int sSpacing = 22;                    // line spacing
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("H O L D !", 60, 50);       
      delay(p1HoldButtonDelay);                   // wait
      drawScreen(displayingPage);                 // re-draw the screen

    // refresh readings from calipers 
      if (!refreshCalipers()) {
        delay(20);
        refreshCalipers();
      }    

    // adjust coordinates to new reading
      for (int i = 0; i < 3; i++) {
        xAdj[i] = xAdj[i] - tXreading + xReading;
        yAdj[i] = yAdj[i] - tYreading + yReading;
        zAdj[i] = zAdj[i] - tZreading + zReading;
      }
  }


// -----------------------------------------------------------------------------------------
// page 2

  // reboot the device
  void twoRebootPressed() { 
    log_system_message("button: reboot");
    delay(500);          // give time to send the above html
    ESP.restart();
    delay(5000);         // restart fails without this delay  
  }

  // eanble wifi
  void twoWifiPressed() { 
    log_system_message("button: wifi enable");
    startTheWifi();
  }

  // store current position in to eeprom
  void twoStorePressed() {
    log_system_message("button: store settings");
    settingsEeprom(1);   // store to eeprom 
  }

  // recall position from eeprom
  void twoRecallPressed() {
    log_system_message("button: recall settings");
    settingsEeprom(0);   // read from eeprom 
    refreshCalipers();   // update display
    // log time of last reading to prevent first reading zeroing it
      lastReadingTimeX = millis();            
      lastReadingTimeY = millis(); 
      lastReadingTimeZ = millis(); 
  }


// -----------------------------------------------------------------------------------------
// page 3

// switch to page 1
void threePage1Pressed() { 
  log_system_message("button: screen 1");
  drawScreen(1);
}

// neumeric keypad 
// curently only used by page 4 to jump to position

  // check entered number is valid and convert to a float
  void verifyNumber() {          
    if (keyEnteredNumber.length() > noDigitsOnNumEntry) keyEnteredNumber = "";       // too long
    keyEnteredNumberVal = keyEnteredNumber.toFloat();                // convert to a float
    if (serialDebug) Serial.println("Number entered: " + String(keyEnteredNumberVal, DROnoOfDigits1 + DROnoOfDigits2));
  }

  void buttonKey1Pressed() {
    keyEnteredNumber += "1";
    verifyNumber();
  }

  void buttonKey2Pressed() {
    keyEnteredNumber += "2";
    verifyNumber();
  }

  void buttonKey3Pressed() {
    keyEnteredNumber += "3";
    verifyNumber();
  }

  void buttonKey4Pressed() {
    keyEnteredNumber += "4";
    verifyNumber();
  }

  void buttonKey5Pressed() {
    keyEnteredNumber += "5";
    verifyNumber();
  }

  void buttonKey6Pressed() {
    keyEnteredNumber += "6";
    verifyNumber();
  }

  void buttonKey7Pressed() {
    keyEnteredNumber += "7";
    verifyNumber();
  }

  void buttonKey8Pressed() {
    keyEnteredNumber += "8";
    verifyNumber();
  }

  void buttonKey9Pressed() {
    keyEnteredNumber += "9";
    verifyNumber();
  }

  void buttonKey0Pressed() {
    keyEnteredNumber += "0";
    verifyNumber();
  }

  void buttonKeyPointPressed() {
    if (keyEnteredNumber.indexOf('.') != -1) return;   // already a decimal point in the number
    keyEnteredNumber += ".";
    verifyNumber();
  }  

  void buttonKeyCLRPressed() {
    keyEnteredNumber = "";
  }

  void buttonKeyBackPressed() {
    if (keyEnteredNumber.length() < 1) return; // String is empty
    keyEnteredNumber = keyEnteredNumber.substring(0, keyEnteredNumber.length() - 1);
  }  

  void buttonKeyEnterPressed() {
    log_system_message("keypad: ENTER pressed, number entered is " + keyEnteredNumber);
    keyEnteredNumber = "";
  }


// -----------------------------------------------------------------------------------------
// page 4 - step through the gcode coordinates

  // next position
  void buttonKeyStepNextPressed() {
    gcodeStepPosition++;
    if (gcodeStepPosition > gcodeLineCount) gcodeStepPosition = 1;
  }

  // previous position
  void buttonKeyStepPrevPressed() {
    gcodeStepPosition--;
    if (gcodeStepPosition < 1) gcodeStepPosition = gcodeLineCount;       
  }  

  // back to first position
  void buttonKeyStepResetPressed() {
    gcodeStepPosition = 1;
  }  
  
  // jump to number enbtered on keypad (page 3)
  void buttonKeyStepKBDPressed() {           
    int numFromKBD = keyEnteredNumber.toInt();
    if (numFromKBD > 0 && numFromKBD <= gcodeLineCount) gcodeStepPosition = numFromKBD;
  }  


// -----------------------------------------------------------------------------------------
// end
