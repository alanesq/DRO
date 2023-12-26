/* -----------------------------------------------------------------------------------------


      Procedures triggered when a button is pressed on the screen - 25dec23


// -----------------------------------------------------------------------------------------
*/


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
  yAdj[currentCoord] = zAdj[currentCoord] + t;   
}


//  ------------- coordinates -----------------

void coord1pressed() { 
  log_system_message("button: C1");
  currentCoord = 0;
  displayRefreshX = 1;    // make sure the display updates
}

void coord2pressed() { 
  log_system_message("button: C2");
  currentCoord = 1;
  displayRefreshX = 1;    
}

void coord3pressed() { 
  log_system_message("button: C3");
  currentCoord = 2;
  displayRefreshX = 1;   
}


//  ------------- misc -----------------

void button4Pressed() { 
  log_system_message("button: screen 2");
  drawScreen(2);      // switch to page 2
}


// -----------------------------------------------------------------------------------------
// page 2

void button5Pressed() { 
  log_system_message("button: screen 1");
  drawScreen(1);
}

void button6Pressed() { 
  log_system_message("button: reboot");
  delay(500);          // give time to send the above html
  ESP.restart();
  delay(5000);         // restart fails without this delay  
}


// -----------------------------------------------------------------------------------------
// page 3

// keypad

// add digit to number and update display
void displayKeypadNo(int keyPressed) {
  if (displayingPage != 3) return;                              // keypad is only for screen 3
  keyEnteredNumber += keyPressed;
  // set up font
    tft.setFreeFont(&sevenSeg16pt7b);      
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(1);
  tft.setTextPadding( tft.textWidth("8") * DROnoOfDigits );    // width to delete previous number
  tft.drawString(keyEnteredNumber, keyX, keyY - 30 );  
}

void buttonKey1Pressed() {
  log_system_message("keypad: 1 pressed");
  displayKeypadNo(1);      // add digit to number and update display
}

void buttonKey2Pressed() {
  log_system_message("keypad: 2 pressed");
  displayKeypadNo(2);      // add digit to number and update display
}

void buttonKey3Pressed() {
  displayKeypadNo(3);      // add digit to number and update display
}

void buttonKey4Pressed() {
  log_system_message("keypad: 4 pressed");
  displayKeypadNo(4);      // add digit to number and update display
}

void buttonKey5Pressed() {
  log_system_message("keypad: 5 pressed");
  displayKeypadNo(5);      // add digit to number and update display
}

void buttonKey6Pressed() {
  log_system_message("keypad: 6 pressed");
  displayKeypadNo(6);      // add digit to number and update display
}

void buttonKey7Pressed() {
  log_system_message("keypad: 7 pressed");
  displayKeypadNo(7);      // add digit to number and update display
}

void buttonKey8Pressed() {
  log_system_message("keypad: 8 pressed");
  displayKeypadNo(8);      // add digit to number and update display
}

void buttonKey9Pressed() {
  log_system_message("keypad: 9 pressed");
  displayKeypadNo(9);      // add digit to number and update display
}

void buttonKey0Pressed() {
  log_system_message("keypad: 0 pressed");
  displayKeypadNo(0);      // add digit to number and update display
}

void buttonKeyEnterPressed() {
  log_system_message("keypad: ENTER pressed");
}


// -----------------------------------------------------------------------------------------
// end
