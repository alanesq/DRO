/* -----------------------------------------------------------------------------------------


      Procedures triggered when a button is pressed on the screen - 29Dec23


// -----------------------------------------------------------------------------------------
*/


// check if too soon since last button was pressed
bool checkButtonTiming() {
  if (millis() - lastTouch < minTimeBetweenPress) {
    lastTouch = millis();
    return 1;
  } else {
    return 0;
  }
}


// -----------------------------------------------------------------------------------------
// page 1

//  ------------- zero -----------------

void zeroXpressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: zeroX");
  xAdj[currentCoord] = xReading;    // zero X
}

void zeroYpressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: zeroY");
  yAdj[currentCoord] = yReading;    // zero Y
}

void zeroZpressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: zeroZ");
  zAdj[currentCoord] = zReading;    // zero Z
}

void zAllpressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: zAll");
  xAdj[currentCoord] = xReading;    // zero X, Y and Z
  yAdj[currentCoord] = yReading;     
  zAdj[currentCoord] = zReading;     
}


//  ------------- half -----------------

void halfXpressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: halfX");
  float t = (xReading - xAdj[currentCoord]) / 2.0;  // half of current displayed reading
  xAdj[currentCoord] = xAdj[currentCoord] + t;      
}

void halfYpressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: halfY");
  float t = (yReading - yAdj[currentCoord]) / 2.0;   
  yAdj[currentCoord] = yAdj[currentCoord] + t;   
}

void halfZpressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: halfZ");
  float t = (zReading - zAdj[currentCoord]) / 2.0;   
  yAdj[currentCoord] = zAdj[currentCoord] + t;   
}


//  ------------- coordinates -----------------

void coord1pressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: C1");
  currentCoord = 0;
  displayRefreshX = 1;    // make sure the display updates
}

void coord2pressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: C2");
  currentCoord = 1;
  displayRefreshX = 1;    
}

void coord3pressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: C3");
  currentCoord = 2;
  displayRefreshX = 1;   
}


//  ------------- misc -----------------

void OnePage2Pressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: screen 2"); 
  drawScreen(2);      // switch to page 2
}

void OnePage3Pressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: screen 3"); 
  drawScreen(3);      // switch to page 3
}

void OnePage4Pressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: screen 4"); 
  //drawScreen(4);      // switch to page 3
}

void OnePage5Pressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: screen 5"); 
  //drawScreen(5);      // switch to page 3
}


// -----------------------------------------------------------------------------------------
// page 2

void twoPage1Pressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: screen 1");
  drawScreen(1);
}

void twoRebootPressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: reboot");
  delay(500);          // give time to send the above html
  ESP.restart();
  delay(5000);         // restart fails without this delay  
}


// -----------------------------------------------------------------------------------------
// page 3

void threePage1Pressed() { 
  if (checkButtonTiming()) return;           // if too soon 
  log_system_message("button: screen 1");
  drawScreen(1);
}

// keypad
  // check entered number is valid and convert to a float
  void verifyNumber() {          
    if (checkButtonTiming()) return;                                 // if too soon 
    if (keyEnteredNumber.length() > 11) keyEnteredNumber = "";       // too long
    keyEnteredNumberVal = keyEnteredNumber.toFloat();                // convert to a float
    if (serialDebug) Serial.println("Number entered: " + String(keyEnteredNumberVal, DROnoOfDigits));
  }

  void buttonKey1Pressed() {
    if (checkButtonTiming()) return;           // if too soon 
    keyEnteredNumber += "1";
    verifyNumber();
  }

  void buttonKey2Pressed() {
    if (checkButtonTiming()) return;           // if too soon 
    keyEnteredNumber += "2";
    verifyNumber();
  }

  void buttonKey3Pressed() {
    if (checkButtonTiming()) return;           // if too soon 
    keyEnteredNumber += "3";
    verifyNumber();
  }

  void buttonKey4Pressed() {
    if (checkButtonTiming()) return;           // if too soon 
    keyEnteredNumber += "4";
    verifyNumber();
  }

  void buttonKey5Pressed() {
    if (checkButtonTiming()) return;           // if too soon 
    keyEnteredNumber += "5";
    verifyNumber();
  }

  void buttonKey6Pressed() {
    if (checkButtonTiming()) return;           // if too soon 
    keyEnteredNumber += "6";
    verifyNumber();
  }

  void buttonKey7Pressed() {
    if (checkButtonTiming()) return;           // if too soon 
    keyEnteredNumber += "7";
    verifyNumber();
  }

  void buttonKey8Pressed() {
    if (checkButtonTiming()) return;           // if too soon 
    keyEnteredNumber += "8";
    verifyNumber();
  }

  void buttonKey9Pressed() {
    if (checkButtonTiming()) return;           // if too soon 
    keyEnteredNumber += "9";
    verifyNumber();
  }

  void buttonKey0Pressed() {
    if (checkButtonTiming()) return;           // if too soon 
    keyEnteredNumber += "0";
    verifyNumber();
  }

  void buttonKeyPointPressed() {
    if (checkButtonTiming()) return;           // if too soon 
    if (keyEnteredNumber.indexOf('.') != -1) return;   // already a decimal point in the number
    keyEnteredNumber += ".";
    verifyNumber();
  }  

  void buttonKeyCLRPressed() {
    if (checkButtonTiming()) return;           // if too soon 
    keyEnteredNumber = "";
  }

  void buttonKeyBackPressed() {
    if (checkButtonTiming()) return;           // if too soon 
    if (keyEnteredNumber.length() < 1) return; // String is empty
    keyEnteredNumber = keyEnteredNumber.substring(0, keyEnteredNumber.length() - 1);
  }  

  void buttonKeyEnterPressed() {
    if (checkButtonTiming()) return;           // if too soon 
    log_system_message("keypad: ENTER pressed, number entered is " + keyEnteredNumber);
    keyEnteredNumber = "";
  }


// -----------------------------------------------------------------------------------------
// end
