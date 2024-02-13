/* -----------------------------------------------------------------------------------------


          Procedures triggered when a button is pressed on the screen - 13Feb24

          Part of the  "SuperLowBudget-DRO" sketch - https://github.com/alanesq/DRO

          
// -----------------------------------------------------------------------------------------
*/

// on all pages

  // page buttons
    void OnePage1Pressed() { 
      log_system_message("button: page 1 selected"); 
      drawScreen(1);      // switch to page 1
    }

    void OnePage2Pressed() { 
      log_system_message("button: page 2 selected"); 
      drawScreen(2);  
    }

    void OnePage3Pressed() { 
      log_system_message("button: page 3 selected"); 
      drawScreen(3);   
    }

    void OnePage4Pressed() { 
      log_system_message("button: page 4 selected"); 
      drawScreen(4);   
    }

    void OnePage5Pressed() {           // note: page5 is not enabled but here as an option (change 'numberOfPages' in DRO.ino)
      log_system_message("button: page 5 selected"); 
      drawScreen(5);    
    }  


// -----------------------------------------------------------------------------------------
// page 1

  //  ------------- zero -----------------
  // zero the current position

    void zeroXpressed() { 
      if (caliperCount < 1) return;
      refreshCalipers(3);    // make sure we have a valid current reading
      log_system_message("button: zero " + calipers[0].title);
      calipers[0].adj[currentCoord] = calipers[0].reading;    // zero X
    }

    void zeroYpressed() { 
      if (caliperCount < 2) return;
      refreshCalipers(3);    
      log_system_message("button: zero " + calipers[1].title);
      calipers[1].adj[currentCoord] = calipers[1].reading;    // zero Y
    }

    void zeroZpressed() { 
      if (caliperCount < 3) return;
      refreshCalipers(3);   
      log_system_message("button: zero " + calipers[2].title);
      calipers[2].adj[currentCoord] = calipers[2].reading;    // zero Z
    }

    void zAllpressed() { 
      log_system_message("button: zAll");
      refreshCalipers(3);   
      for (int c=0; c < caliperCount; c++) {
        calipers[c].adj[currentCoord] = calipers[c].reading;    // zero all
      }
    }


  //  ------------- half -----------------
  // half the current displayed value

    void halfXpressed() { 
      if (caliperCount < 1) return;
      log_system_message("button: half "  + calipers[0].title);
      float t = (calipers[0].reading - calipers[0].adj[currentCoord]) / 2.0;        // half of current displayed reading
      calipers[0].adj[currentCoord] = calipers[0].adj[currentCoord] + t;      
    }

    void halfYpressed() { 
      if (caliperCount < 1) return;
      log_system_message("button: half " + calipers[1].title);
      float t = (calipers[1].reading - calipers[1].adj[currentCoord]) / 2.0;   
      calipers[1].adj[currentCoord] = calipers[1].adj[currentCoord] + t;    
    }

    void halfZpressed() { 
      if (caliperCount < 2) return;
      log_system_message("button: half " + calipers[2].title);
      float t = (calipers[2].reading - calipers[2].adj[currentCoord]) / 2.0;   
      calipers[2].adj[currentCoord] = calipers[2].adj[currentCoord] + t;   
    }


  //  ------------- coordinates -----------------
  // switch between the 3 coordinates

    void coord1pressed() { 
      if (noOfCoordinates < 1) return;
      log_system_message("button: C1");
      currentCoord = 0;
      displayReadings();      // make sure the display updates
    }

    void coord2pressed() { 
      if (noOfCoordinates < 2) return;
      log_system_message("button: C2");
      currentCoord = 1;
      displayReadings();     
    }

    void coord3pressed() { 
      if (noOfCoordinates < 3) return;
      log_system_message("button: C3");
      currentCoord = 2;
      displayReadings();     
    }

    void coord4pressed() { 
      if (noOfCoordinates < 4) return;
      log_system_message("button: C4");
      currentCoord = 3;
      displayReadings();     
    }  


  // hold current reading
    void oneHoldPressed() {
      log_system_message("button: hold current reading");
      
      refreshCalipers(2);                          // refresh readings from calipers  
      
      // store current readings
        float tReading[caliperCount];
        for (int c=0; c < caliperCount; c++) {
          tReading[c] = calipers[c].reading;
        }
    
      // display 'HOLD' and wait
        tft.fillScreen(TFT_BLACK);                  // Clear the screen
        tft.setFreeFont(FM9);                       // standard Free Mono font - available sizes: 9, 12, 18 or 24
        tft.setTextSize(2);                         // 1 or 2
        const int sSpacing = 22;                    // line spacing
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("H O L D !", 60, 50);       
        delay(p1HoldButtonDelay);                   // wait
        drawScreen(displayingPage);                 // re-draw the screen

      refreshCalipers(2);                           // refresh readings from calipers  

      // adjust coordinates to new reading
        for (int i = 0; i < noOfCoordinates; i++) {
          for (int c=0; c < caliperCount; c++) {
            calipers[c].adj[i] = calipers[c].adj[i] - tReading[c] + calipers[c].reading;
          }
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
    refreshCalipers(2);  // refresh readings from calipers  
    // log time of last reading to prevent first reading zeroing it
      for (int c=0; c < caliperCount; c++) {
        calipers[c].lastReadTime = millis();            
      } 
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

  void buttonKeyMinusPressed() {
    if (keyEnteredNumber.indexOf('-') != -1) {   // number is already negative
      keyEnteredNumber = keyEnteredNumber.substring(1, keyEnteredNumber.length());
    } else {
      keyEnteredNumber = "-" + keyEnteredNumber;
    }
    verifyNumber();
  }  

  void buttonKeyCLRPressed() {
    keyEnteredNumber = "";
    verifyNumber();
  }

  void buttonKeyBackPressed() {
    if (keyEnteredNumber.length() < 1) return; // String is empty
    keyEnteredNumber = keyEnteredNumber.substring(0, keyEnteredNumber.length() - 1);
    verifyNumber();
  }  

  void buttonKeyEnterPressed() {
    log_system_message("keypad: ENTER pressed, number entered is " + keyEnteredNumber);
    keyEnteredNumber = "";
    verifyNumber();
  }

  // setx
  void buttonp3setxPressed() {
    if (noOfCoordinates < 1) return;
    if (keyEnteredNumberVal >= DROerrorCodeReduced || keyEnteredNumberVal <= -(DROerrorCodeReduced / 10) ) return;     // number beyond limit
    log_system_message("button: set " + calipers[0].title + " pressed: " + String(keyEnteredNumberVal));
    calipers[0].adj[currentCoord] = calipers[0].reading - keyEnteredNumberVal;
    calipers[0].lastReadTime = millis();    
  }

  // sety
  void buttonp3setyPressed() {
    if (noOfCoordinates < 2) return;
    if (keyEnteredNumberVal >= DROerrorCodeReduced || keyEnteredNumberVal <= -(DROerrorCodeReduced / 10) ) return;     // number beyond limit
    log_system_message("button: set " + calipers[1].title + " pressed: " + String(keyEnteredNumberVal));
    calipers[1].adj[currentCoord] = calipers[1].reading - keyEnteredNumberVal;
    calipers[1].lastReadTime = millis();    
  }

  // setz
  void buttonp3setzPressed() {
    if (noOfCoordinates < 2) return;
    if (keyEnteredNumberVal >= DROerrorCodeReduced || keyEnteredNumberVal <= -(DROerrorCodeReduced / 10) ) return;     // number beyond limit
    log_system_message("button: set " + calipers[2].title + " pressed: " + String(keyEnteredNumberVal));
    calipers[2].adj[currentCoord] = calipers[2].reading - keyEnteredNumberVal;
    calipers[2].lastReadTime = millis();    
  }


// -----------------------------------------------------------------------------------------
// page 4 - step through the gcode coordinates

  // next position
  void buttonKeyStepNextPressed() {
    if (gcodeLineCount == 0) return;    // nothing in the store
    gcodeStepPosition++;
    if (gcodeStepPosition > gcodeLineCount) gcodeStepPosition = 1;
  }

  // previous position
  void buttonKeyStepPrevPressed() {
    if (gcodeLineCount == 0) return;    // nothing in the store
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

  // store current position
  void buttonStorePosPressed() {           
     log_system_message("button: store current position pressed");
     if (gcodeLineCount > maxGcodeStringLines - 1) return;      // no more space
     if (gcodeLineCount < 0) gcodeLineCount = 0;                // just in case something odd has happened
     gcodeLineCount++;
     gcodeStepPosition = gcodeLineCount;                        // move to this new position in gcode steps 
      for (int c=0; c < caliperCount; c++) {
        inc[c] = 1;                                           // enable axis in stored data list
        gcode[c][gcodeStepPosition - 1] = calipers[c].reading - calipers[c].adj[currentCoord];       // store current position
      }
     drawScreen(4);      // redraw screen to clear previous text
  } 

  // clear store
  void buttonClearStorePressed() {           
    clearGcode();
    drawScreen(4);      // redraw screen to clear previous text
  }    


// -----------------------------------------------------------------------------------------
// end
