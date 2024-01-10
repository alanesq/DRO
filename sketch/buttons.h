/* -----------------------------------------------------------------------------------------


      Procedures triggered when a button is pressed on the screen - 10Jan24


// -----------------------------------------------------------------------------------------
*/


// -----------------------------------------------------------------------------------------
// on all pages

//  ------------- pages -----------------    #noPages

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


// -----------------------------------------------------------------------------------------
// page 2

void twoRebootPressed() { 
  log_system_message("button: reboot");
  delay(500);          // give time to send the above html
  ESP.restart();
  delay(5000);         // restart fails without this delay  
}

void twoWifiPressed() { 
  log_system_message("button: wifi enable");
  startTheWifi();
}



// -----------------------------------------------------------------------------------------
// page 3

void threePage1Pressed() { 
  log_system_message("button: screen 1");
  drawScreen(1);
}

// keypad
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


// page 4 - step through gcode coordinates

  void buttonKeyStepNextPressed() {
    gcodeStepPosition++;
    if (gcodeStepPosition > gcodeLineCount) gcodeStepPosition = 1;
  }

  void buttonKeyStepPrevPressed() {
    gcodeStepPosition--;
    if (gcodeStepPosition < 1) gcodeStepPosition = gcodeLineCount;       
  }  

  void buttonKeyStepResetPressed() {
    gcodeStepPosition = 1;
  }  
  
  void buttonKeyStepKBDPressed() {           // use number enbtered on keypad (page 3)
    int numFromKBD = keyEnteredNumber.toInt();
    if (numFromKBD > 0 && numFromKBD <= gcodeLineCount) gcodeStepPosition = numFromKBD;
  }  

// -----------------------------------------------------------------------------------------
// end
