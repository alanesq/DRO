/*******************************************************************************************************************
 *
 *                                              SuperLowBudget-DRO               11feb24
 *                                              ------------------
 *
 *          3 Channel DRO using cheap digital calipers and a ESP32-2432S028R (aka Cheap Yellow Display)
 *                     see: https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display
 *
 *                                      https://github.com/alanesq/DRO
 *
 *
 *                         Tested in Arduino IDE 2.3.0 with Espressif board manager ESP32 2.0.14
 *
 *                 Included files: settings.h, standard.h, ota.h, sevenSeg.h, Free_Fonts.h, buttons.h & wifi.h
 *
 *   If using WifiManager the first time the ESP starts it will create an access point "ESPDRO" which you need to connect to 
 *    in order to enter your wifi details.  Default password = "password"   (change this in wifi.h)
 *    for more info. see: https://www.youtube.com/watch?v=Errh7LEEug0
 *
 *
 *             Distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
 *                    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 ********************************************************************************************************************

  Libraries used:     https://github.com/PaulStoffregen/XPT2046_Touchscreen - 1.4
                      https://github.com/Bodmer/TFT_eSPI - 2.5.34
                      https://github.com/Bodmer/TFT_eWidget - 0.0.5
                      https://github.com/tzapu/WiFiManager - 2.0.16         


  Ribbon cable soldered to one side of the esp32 module: https://github.com/alanesq/DRO/blob/main/PCB/ribbonCablePins.jpeg
  The 3 colour LED should be removed.
    3.3v      (from nearby connector)
    gnd       (from nearby connector)
    gpio 0    Caliper X clock
    gpio 4    Caliper X data
    gpio 16   Caliper Z clock
    gpio 17   Caliper Y clock
    gpio 5    Caliper Y data
    gpio 3    Serial Rx    
    gpio 1    Serial Tx 
    gpio 22   Caliper Z data

  Pins that should be ok to use: 22, 32, 33, 26 (also 35, 18 which are input only)
  other possible use pins: 4, 16, 17 (onboard LED) : 5, 23, 18, 19 (sd card) : 1, 3 (Serial) 0 (onboard button) : 34 (LDR)


  Test Points:
      S1	GND	              near USB-SERIAL
      S2	3.3v	              for ESP32
      S3	5v	              near USB-SERIAL
      S4	GND	              for ESP32
      S5	3.3v	              for TFT
      JP0 (pad nearest USB socket)    5v TFT LDO
      JP0	3.3v	              TFT LDO
      JP3 (pad nearest USB socket)    5v ESP32 LDO
      JP3	3.3v	              ESP32 LDO    


    Digital Caliper pinout (VCC = edge of circuit board): 
          VCC (1.5V) - I just use a simple voltage divider of 100ohm resistors to supply 1.65v as they use very little power
          CLOCK
          DATA
          GND


    Colours available to use on display: 
          TFT_BLACK, TFT_NAVY, TFT_DARKGREEN, TFT_DARKCYAN, TFT_MAROON, TFT_PURPLE, TFT_OLIVE,
          TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLACK, TFT_GREEN, TFT_CYAN, TFT_RED, TFT_MAGENTATFT_YELLOW,
          TFT_WHITE, TFT_ORANGE, TFT_GREENYELLOW, TFT_PINK, TFT_BROWN, TFT_GOLD, TFT_SILVER, TFT_SKYBLUE, TFT_VIOLET


    Wifi features:
      Simulate pressing the screen:       http://x.x.x.x/touch?x=100&y=50
      Restart the esp32:                  http://x.x.x.x/reboot
      Check if it is live:                http://x.x.x.x/ping
      Log of activity:                    http://x.x.x.x/log
      Testing code:                       http://x.x.x.x/test               (see 'handleTest()' at bottom of page)
      Update via OTA:                     http://x.x.x.x/ota                (default password is 'password')
      List stored positions               http://x.x.x.x/stored

*/

#include <Arduino.h>                      // required to use Strings 
String enteredGcode;                      // store for the entered gcode on web page
#include "settings.h"                     // load in settings for the DRO from settings.h file

#if (!defined ESP32)
  #error This code is for the ESP32 based 'Cheap Yellow Display' only
#endif

#include <esp_task_wdt.h>                 // watchdog timer   - see: https://iotassistant.io/esp32/enable-hardware-watchdog-timer-esp32-arduino-ide/     
#include <WiFi.h>

// forward declarations
  void log_system_message(String smes);   // in standard.h
  void handleRoot();
  void handleData();
  void handlePing();
  void settingsEeprom(bool eDirection);
  void handleTest();
  void drawScreen(int);      
  void displayReadings();
  void startTheWifi();
  bool refreshCalipers(int);
  void clearGcode();


  // -------------------------------------------- CYD DISPLAY --------------------------------------------

    #include <SPI.h>
    #include <FS.h>
    #include <XPT2046_Touchscreen.h>

    SPIClass mySpi = SPIClass(HSPI);            // note: I had to change VSPI to HSPI
    XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);  

    #include <TFT_eSPI.h>                       // display driver
    #include <TFT_eWidget.h>                    // Widget library
    #include "Free_Fonts.h"                     // Standard fonts info
    #include "sevenSeg.h"                       // seven segment style font
    TFT_eSPI tft = TFT_eSPI();
      
    MeterWidget dro  = MeterWidget(&tft);       // Meter used at startup

    unsigned long lastTouch = 0;                // last time touchscreen button was pressed
    
    // entered gcode
      float gcode[caliperCount][maxGcodeStringLines];    // store for positions extracted from the gcode
      int gcodeLineCount = 0;                   // number of positions that have been extracted from the gcode
      bool inc[caliperCount] = {0};             // which coordinates to process
      int gcodeStepPosition = 1;                // current position in the extracted positions
      float gcodeDROadj[caliperCount] = {0};    // temp adjustments to DRO readings for locating position when stepping through gcode positions


// --------------------------------------- Menu / buttons ---------------------------------------

  // font to use for all buttons  
    #define MENU_FONT FM9                                // standard Free Mono font - available sizes: 9, 12, 18 or 24
    #define MENU_SIZE 2                                  // font size (1 or 2)

    // Page selection buttons
    // Note: to add more pages, in 'DRO.ino' increase 'numberOfPages', modify the two sections marked with '#noPages' and add procedures to 'buttons.h'
      const int numberOfPages = 4;                       // the number of pages available (note - it is already configured for 5)
      const int pageButtonWidth = 34;                    // width of the buttons
      const int pageButtonSpacing = 5;                   // space between buttons
      const int pageButtonHeight = (SCREEN_HEIGHT / numberOfPages) - pageButtonSpacing;  // height of each button
  
    // DRO readings size/position settings 
        const int DROdisplaySpacing = 10;                // space between the DRO readings
        const int DROnoOfDigits1 = 4;                    // number of digits to display in caliper readings (before decimal)
        const int DROnoOfDigits2 = 2;                    // number of digits to display in caliper readings (after decimal)
        const int DROwidth = 200;                        // width of DRO reading display   i.e. x position of zero buttons  
        const int DROheight = 200;                       // Height of the DRO reading display for placement of lower buttons  
        const int DRObuttonheight = 30;                  // buttons around DRO readings              
        const int DRObuttonWidth = 35;                     
        const int DROdbuttonPlacement = 70;              // vertical spacing of the DRO buttons  
        
    // page 2 - misc 
      const int p2secondColumn = 150;                    // y position of second column of buttons 
      const int p2buttonSpacing = 8;                     // space between rows of buttons

    // page 3 - numeric keypad 
      const int noDigitsOnNumEntry = 10;                 // maximum length of entered number
      const int keyWidth = 45;
      const int keyHeight = 32;
      const int keySpacing = 10;
      const int keyX = 110;                              // position on screen (top left)
      const int keyY = 30;
      const int setKeyWidth = 65;                        // width os set buttons
      
    // page 4 - gcode
      const int p4ButtonSpacing = 70;                    // button spacing
      const int p4ButtonGap = 4;                         // gap between buttons

    
    int displayingPage = 1;        // which screen page is live

    typedef void (*buttonActionCallback)(void);   
    
    // struct to create a button
    struct buttonStruct {
      int page;                    // which screen page it belongs to (0 = show on all pages)
      bool enabled;                // if this button is enabled
      String label;                // buttons title
      int16_t x, y;                // location on screen (top left)
      uint16_t width, height;
      uint16_t outlineColour;
      uint16_t outlineThickness;
      uint16_t fillColour;
      uint16_t textColour;
      ButtonWidget* btn;            // button object (widget)
      buttonActionCallback action;  // procedure called when button pressed
    };


  // create the button widgets - there has to be a better way? - ensure there are enough for the number of buttons required (check log to verify)
    ButtonWidget butnW[] = { &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, 
                             &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, 
                             &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft, &tft };   
    int widgetCount = (sizeof(butnW) / sizeof(*butnW)) - 1;     // counter used for assigning widgets to buttons


  // keypad (Page 3)
    String keyEnteredNumber = "";      // the number entered on the keypad (String)
    float keyEnteredNumberVal = 0;     // the above string as an float


  // ---------------------------------- -define the button widgets --------------------------------------

    #include "buttons.h"             // the procedures which are triggered when a button is pressed

    const int buttonSpacing = 6;     // spacing used when placing the buttons on display
  
    buttonStruct screenButtons[] {               

    // all pages
      // page selection buttons - #noPages
      //    page, enabled, title, x, y, width, height, border colour, burder thickness, fill colour, pressed colour, button, procedure to run when button is pressed
      {0, 1, "P1", SCREEN_WIDTH - pageButtonWidth, (pageButtonHeight + pageButtonSpacing) * 0, pageButtonWidth, pageButtonHeight, TFT_WHITE, 1, TFT_SILVER, TFT_BLACK, &butnW[widgetCount--], &OnePage1Pressed},
      {0, 1, "P2", SCREEN_WIDTH - pageButtonWidth, (pageButtonHeight + pageButtonSpacing) * 1, pageButtonWidth, pageButtonHeight, TFT_WHITE, 1, TFT_SILVER, TFT_BLACK, &butnW[widgetCount--], &OnePage2Pressed},
      {0, 1, "P3", SCREEN_WIDTH - pageButtonWidth, (pageButtonHeight + pageButtonSpacing) * 2, pageButtonWidth, pageButtonHeight, TFT_WHITE, 1, TFT_SILVER, TFT_BLACK, &butnW[widgetCount--], &OnePage3Pressed},
      {0, 1, "P4", SCREEN_WIDTH - pageButtonWidth, (pageButtonHeight + pageButtonSpacing) * 3, pageButtonWidth, pageButtonHeight, TFT_WHITE, 1, TFT_SILVER, TFT_BLACK, &butnW[widgetCount--], &OnePage4Pressed},
      {0, 1, "P5", SCREEN_WIDTH - pageButtonWidth, (pageButtonHeight + pageButtonSpacing) * 4, pageButtonWidth, pageButtonHeight, TFT_WHITE, 1, TFT_SILVER, TFT_BLACK, &butnW[widgetCount--], &OnePage5Pressed},

    // Page 1

      // Zero buttons
      {1, calipers[0].enabled, "Z", DROwidth, 0, DRObuttonWidth, DRObuttonheight * 2 - buttonSpacing, TFT_WHITE, 1, TFT_DARKGREEN, TFT_BLACK, &butnW[widgetCount--], &zeroXpressed},
      {1, calipers[1].enabled, "Z", DROwidth, DROdbuttonPlacement * 1, DRObuttonWidth,  DRObuttonheight * 2 - buttonSpacing, TFT_WHITE, 1, TFT_DARKGREEN, TFT_BLACK, &butnW[widgetCount--], &zeroYpressed},
      {1, calipers[2].enabled, "Z", DROwidth, DROdbuttonPlacement * 2, DRObuttonWidth,  DRObuttonheight * 2 - buttonSpacing, TFT_WHITE, 1, TFT_DARKGREEN, TFT_BLACK, &butnW[widgetCount--], &zeroZpressed},
      {1, 1, "Z", DROwidth, DROheight, DRObuttonWidth * 2 + buttonSpacing,  DRObuttonheight, TFT_WHITE, 1, TFT_DARKGREEN, TFT_BLACK, &butnW[widgetCount--], &zAllpressed},

      // half buttons
      {1, calipers[0].enabled, "1/2", DROwidth + DRObuttonWidth + buttonSpacing, 0, DRObuttonWidth,  DRObuttonheight * 2 - buttonSpacing, TFT_WHITE, 1, TFT_DARKCYAN, TFT_BLACK, &butnW[widgetCount--], &halfXpressed},
      {1, calipers[1].enabled, "1/2", DROwidth + DRObuttonWidth + buttonSpacing, DROdbuttonPlacement * 1, DRObuttonWidth,  DRObuttonheight * 2 - buttonSpacing, TFT_WHITE, 1, TFT_DARKCYAN, TFT_BLACK, &butnW[widgetCount--], &halfYpressed},
      {1, calipers[2].enabled, "1/2", DROwidth + DRObuttonWidth + buttonSpacing, DROdbuttonPlacement * 2, DRObuttonWidth,  DRObuttonheight * 2 - buttonSpacing, TFT_WHITE, 1, TFT_DARKCYAN, TFT_BLACK, &butnW[widgetCount--], &halfZpressed},
      
      // Coordinate select buttons
      {1, 1, "C1", (DRObuttonWidth + buttonSpacing) * 0, DROheight, DRObuttonWidth,  DRObuttonheight, TFT_WHITE, 1, TFT_MAROON, TFT_BLACK, &butnW[widgetCount--], &coord1pressed},
      {1, 1, "C2", (DRObuttonWidth + buttonSpacing) * 1, DROheight, DRObuttonWidth,  DRObuttonheight, TFT_WHITE, 1, TFT_MAROON, TFT_BLACK, &butnW[widgetCount--], &coord2pressed},
      {1, 1, "C3", (DRObuttonWidth + buttonSpacing) * 2, DROheight, DRObuttonWidth,  DRObuttonheight, TFT_WHITE, 1, TFT_MAROON, TFT_BLACK, &butnW[widgetCount--], &coord3pressed},
      {1, 1, "C4", (DRObuttonWidth + buttonSpacing) * 3, DROheight, DRObuttonWidth,  DRObuttonheight, TFT_WHITE, 1, TFT_MAROON, TFT_BLACK, &butnW[widgetCount--], &coord4pressed},

      // hold button
      {1, 1, "H", (DRObuttonWidth + buttonSpacing) * 4, DROheight, DRObuttonWidth - buttonSpacing,  DRObuttonheight, TFT_WHITE, 1, TFT_RED, TFT_BLACK, &butnW[widgetCount--], &oneHoldPressed},

    // page 2
    
      {2, 1, "En. Wifi", 0, SCREEN_HEIGHT - (DRObuttonheight + p2buttonSpacing) * 1, 140, DRObuttonheight, TFT_WHITE, 1, TFT_GREEN, TFT_BLACK, &butnW[widgetCount--], &twoWifiPressed},
      {2, 1, "Reboot", p2secondColumn, SCREEN_HEIGHT - (DRObuttonheight + p2buttonSpacing) * 1, 120, DRObuttonheight, TFT_WHITE, 1, TFT_RED, TFT_BLACK, &butnW[widgetCount--], &twoRebootPressed},
      {2, 1, "Store", p2secondColumn, (DRObuttonheight + p2buttonSpacing) * 0, 120, DRObuttonheight, TFT_WHITE, 1, TFT_YELLOW, TFT_BLACK, &butnW[widgetCount--], &twoStorePressed},
      {2, 1, "Recall", p2secondColumn, (DRObuttonheight + p2buttonSpacing) * 1, 120, DRObuttonheight, TFT_WHITE, 1, TFT_YELLOW, TFT_BLACK, &butnW[widgetCount--], &twoRecallPressed},

    // page 3 
      
      // keypad
      {3, 1, "1", keyX + 0 * (keyWidth + keySpacing), keyY + 2 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKey1Pressed},
      {3, 1, "2", keyX + 1 * (keyWidth + keySpacing), keyY + 2 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKey2Pressed},
      {3, 1, "3", keyX + 2 * (keyWidth + keySpacing), keyY + 2 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKey3Pressed},
      {3, 1, "4", keyX + 0 * (keyWidth + keySpacing), keyY + 1 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKey4Pressed},
      {3, 1, "5", keyX + 1 * (keyWidth + keySpacing), keyY + 1 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKey5Pressed},
      {3, 1, "6", keyX + 2 * (keyWidth + keySpacing), keyY + 1 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKey6Pressed},
      {3, 1, "7", keyX + 0 * (keyWidth + keySpacing), keyY + 0 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKey7Pressed},
      {3, 1, "8", keyX + 1 * (keyWidth + keySpacing), keyY + 0 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKey8Pressed},
      {3, 1, "9", keyX + 2 * (keyWidth + keySpacing), keyY + 0 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKey9Pressed},
      {3, 1, "0", keyX + 0 * (keyWidth + keySpacing), keyY + 3 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKey0Pressed},
      //{3, 1, "ENTER", keyX + 1 * (keyWidth + keySpacing), keyY + 3 * (keyHeight + keySpacing), keyWidth * 2 + keySpacing, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKeyEnterPressed},
      {3, 1, "CLR", keyX + 0 * (keyWidth + keySpacing), keyY + 4 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKeyCLRPressed},
      {3, 1, "BACK", keyX + 1 * (keyWidth + keySpacing), keyY + 4 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKeyBackPressed},
      {3, 1, ".", keyX + 1 * (keyWidth + keySpacing), keyY + 3 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKeyPointPressed},
      {3, 1, "-", keyX + 2 * (keyWidth + keySpacing), keyY + 3 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &butnW[widgetCount--], &buttonKeyMinusPressed},
      
      // set DRO reading to entered number
      {3, calipers[0].enabled, "set" + calipers[0].title, 0, SCREEN_HEIGHT - 3 * (keyHeight + keySpacing), setKeyWidth, DRObuttonheight, TFT_WHITE, 1, TFT_GREEN, TFT_BLACK, &butnW[widgetCount--], &buttonp3setxPressed},
      {3, calipers[1].enabled, "set" + calipers[1].title, 0, SCREEN_HEIGHT - 2 * (keyHeight + keySpacing), setKeyWidth, DRObuttonheight, TFT_WHITE, 1, TFT_GREEN, TFT_BLACK, &butnW[widgetCount--], &buttonp3setyPressed},
      {3, calipers[2].enabled, "set" + calipers[2].title, 0, SCREEN_HEIGHT - 1 * (keyHeight + keySpacing), setKeyWidth, DRObuttonheight, TFT_WHITE, 1, TFT_GREEN, TFT_BLACK, &butnW[widgetCount--], &buttonp3setzPressed},
 
    // page 4
      {4, 1, "Prev", p4ButtonSpacing * 0, SCREEN_HEIGHT - DRObuttonheight, p4ButtonSpacing - p4ButtonGap, DRObuttonheight, TFT_WHITE, 1, TFT_GREEN, TFT_BLACK, &butnW[widgetCount--], &buttonKeyStepPrevPressed},
      {4, 1, "Next", p4ButtonSpacing * 1, SCREEN_HEIGHT - DRObuttonheight, p4ButtonSpacing - p4ButtonGap, DRObuttonheight, TFT_WHITE, 1, TFT_GREEN, TFT_BLACK, &butnW[widgetCount--], &buttonKeyStepNextPressed},
      {4, 1, "Reset", p4ButtonSpacing * 2, SCREEN_HEIGHT - DRObuttonheight, p4ButtonSpacing - p4ButtonGap, DRObuttonheight, TFT_WHITE, 1, TFT_YELLOW, TFT_BLACK, &butnW[widgetCount--], &buttonKeyStepResetPressed},
      {4, 1, "KBD", p4ButtonSpacing * 3, SCREEN_HEIGHT - DRObuttonheight, p4ButtonSpacing - p4ButtonGap, DRObuttonheight, TFT_WHITE, 1, TFT_YELLOW, TFT_BLACK, &butnW[widgetCount--], &buttonKeyStepKBDPressed},
      {4, 1, "Store position", p4ButtonSpacing * 0, SCREEN_HEIGHT - DRObuttonheight * 2 - p4ButtonGap, p4ButtonSpacing * 2 - p4ButtonGap, DRObuttonheight, TFT_WHITE, 1, TFT_YELLOW, TFT_BLACK, &butnW[widgetCount--], &buttonStorePosPressed},
      {4, 1, "Clear all", p4ButtonSpacing * 2, SCREEN_HEIGHT - DRObuttonheight * 2 - p4ButtonGap, p4ButtonSpacing * 2 - p4ButtonGap, DRObuttonheight, TFT_WHITE, 1, TFT_GREEN, TFT_BLACK, &butnW[widgetCount--], &buttonClearStorePressed},

    };
    uint8_t buttonCount = sizeof(screenButtons) / sizeof(*screenButtons);     // find the number of buttons created
    

  // ----------------------------------------------------------------------------------------------------
    

unsigned long wifiBackupRetryTime = 300;  // not used but here to keep compatibility with my standard wifi.h file
bool sTouched = 0;                        // flag if a button is curently pressed on the screen

bool OTAEnabled = 0;                      // flag to show if OTA has been enabled (via supply of password on http://x.x.x.x/ota)
int wifiok = 0;                           // flag if wifi connection is ok 
unsigned long wifiDownTime = 0;           // if wifi connection is lost this records at what time it was lost
float DROerrorCode = 8888.0;              // error code used for DRO readings
bool showPress = 0;                       // show touch data on screen (for calibration / testing)
bool showDROerrors = 0;                   // show errors on DRO readings display 

#include "wifi.h"                         // Load the Wifi / NTP stuff
#include "standard.h"                     // Some standard procedures

#include <EEPROM.h>                       // for storing settings in eeprom (not used at present)

#if ENABLE_OTA
  #include "ota.h"                        // Over The Air updates (OTA)
#endif


// ---------------------------------------------------------------
//                           start wifi
// ---------------------------------------------------------------
// called from setup or when wifi enabled from screen menu

void startTheWifi() {

      // There are three options for type of wifi
        if (wifiType == 1) startWifiManager();                                    // Connect to wifi using wifi manager (see wifi.h)
        if (wifiType == 2) startWifi();                                           // Connect to wifi using fixed details (see wifi.h)
        if (wifiType == 3) startAccessPoint();                                    // Serve own access point - note: commant out line below

      //WiFi.mode(WIFI_STA);     // turn off access point - options are WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF

      // set up web page request handling
        server.on(HomeLink, handleRoot);         // root page
        server.on("/data", handleData);          // supplies information to update root page via Ajax)
        server.on("/ping", handlePing);          // ping requested
        server.on("/log", handleLogpage);        // system log (in standard.h)
        server.on("/test", handleTest);          // testing page
        server.on("/reboot", handleReboot);      // reboot the esp
        server.on("/touch", handleTouch);        // for simulating a press on the touchscreen
        server.on("/stored", handelStored);      // display stored coordinates
        //server.onNotFound(handleNotFound);       // invalid page requested
        #if ENABLE_OTA
          server.on("/ota", handleOTA);          // ota updates web page
        #endif

      // start web server
        if (serialDebug) Serial.println("Starting web server");  

      server.begin();

      // Stop wifi going to sleep (if enabled it can cause wifi to drop out randomly especially on esp8266 boards)
          WiFi.setSleep(false);

      // Finished connecting to network
        if (wifiType == 1 || wifiType == 2) {
          if (WiFi.status() == WL_CONNECTED) {
            log_system_message("Wifi started, ip=" + WiFi.localIP().toString());
            wifiEnabled = 1;
          } else {
            log_system_message("Wifi failed to start");
            wifiEnabled = 0;
          }
        } else {
          log_system_message("Access point, ip=" + WiFi.softAPIP().toString());
          wifiEnabled = 1;
        }
    }


// ---------------------------------------------------------------
//    -SETUP     SETUP     SETUP     SETUP     SETUP     SETUP
// ---------------------------------------------------------------

void setup() {

  const int MeterSpeed = 20;   // spped of meter movements

  if (serialDebug) {           // if serial debug information is enabled
    Serial.begin(serialSpeed); // start serial comms
    //delay(2000);             // gives platformio chance to start serial monitor

    Serial.println("\n\n\n");  // some line feeds
    Serial.println("-----------------------------------");
    Serial.printf("Starting - %s - %s \n", stitle, sversion);
    Serial.println("-----------------------------------");
    Serial.println( "Device type: " + String(ARDUINO_BOARD) + ", chip id: " + String(ESP_getChipId(), HEX));
    // Serial.println(ESP.getFreeSketchSpace());
  }

  // Cheap Yellow Display
    pinMode(SCREEN_BACKLIGHT, OUTPUT);
    digitalWrite(SCREEN_BACKLIGHT, HIGH);       // turn backlight on
    //tft.init();
    tft.begin();
    tft.setRotation(SCREEN_ROTATION);           // set display to landscape
    tft.fillScreen(TFT_BLACK);                  // Clear the screen
    tft.setFreeFont(FM9);                       // standard Free Mono font - available sizes: 9, 12, 18 or 24
    tft.setTextSize(1);                         // 1 or 2
    const int sSpacing = 22;                    // line spacing
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString(String(stitle), 5, 0);   
    tft.drawString(String(sversion), SCREEN_WIDTH - 90, sSpacing * 1);  
    tft.setTextColor(TFT_WHITE, TFT_BLACK); 
    tft.drawString("See: github.com/alanesq/DRO", 0, sSpacing * 2); 

  // display a demo meter    see: https://github.com/Bodmer/TFT_eWidget/blob/main/examples/Meters/Analogue_meters/Analogue_meters.ino
    dro.setZones(75, 100, 50, 75, 25, 50, 0, 25);   
    dro.analogMeter(40, 95, 100.0, "STARTUP", "0", "25", "50", "75", "100"); 

  // reserve memory for the log to reduce stack fragmentation (yes, I use Strings - deal with it ;-)
    for (int i=0; i < LogNumber; i++) { 
      system_message[i].reserve(maxLogEntryLength);
    }

  // reserve memory for stored gcode
    enteredGcode.reserve(maxGcodeStringLines * estimateAverageLineLength);        

  // if (serialDebug) Serial.setDebugOutput(true);             // to enable extra diagnostic info

  //   settingsEeprom(0);                                      // read stored settings from eeprom

  if (wifiEnabled) {
    // update screen
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("Starting wifi ...", 5, sSpacing * 3);   
      dro.updateNeedle(25, MeterSpeed);                        // move dial to 25
    startTheWifi();                                            // start wifi
      // update screen
        tft.setTextColor(TFT_WHITE, TFT_BLACK);     
        if (wifiType != 3) {                                   // if connecting to an existing wifi  
          if (WiFi.status() == WL_CONNECTED) { 
            tft.drawString("IP = " + WiFi.localIP().toString() + "      ", 5, sSpacing * 3);  
          } else {
            tft.drawString("Wifi failed to start", 5, sSpacing * 3);  
            wifiEnabled = 0;
          }
        } else {                                               // if using access point
            tft.drawString("AP:" + String(AP_SSID) + ", ip:" + WiFi.softAPIP().toString(), 5, sSpacing * 3);  
            wifiEnabled = 1;
        }
        dro.updateNeedle(50, MeterSpeed);                      // move dial to 50
  }

  // watchdog timer (esp32)
    if (serialDebug) Serial.println("Configuring watchdog timer");
    esp_task_wdt_init(WDT_TIMEOUT, true);                      //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL);                                    //add current thread to WDT watch        

  // Start the SPI for the touch screen and init the TS library
    mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(mySpi);
    ts.setRotation(SCREEN_ROTATION);  

  // initialise buttons
    char tLabel[21]; 
    for (uint8_t b = 0; b < buttonCount; b++) {
      screenButtons[b].label.toCharArray(tLabel, 20);                   // convert label from String to char array for the next command
      screenButtons[b].btn->initButtonUL(screenButtons[b].x, screenButtons[b].y, screenButtons[b].width, screenButtons[b].height, screenButtons[b].outlineColour, screenButtons[b].fillColour, screenButtons[b].textColour, tLabel, 1);
      screenButtons[b].btn->setPressAction(screenButtons[b].action);    // procedure called when the button is pressed
    }
 
  dro.updateNeedle(100, MeterSpeed);                 // move dial to 100 

  // caliper gpio pins
    for (int x=0; x < caliperCount; x++) {
      pinMode(calipers[x].clockPIN, INPUT);
      pinMode(calipers[x].dataPIN, INPUT);
    }
  
  // get readings from calipers
    if (!refreshCalipers(3)) {    
      delay(2000);                // give calipers a chance to start-up (test to attempt to stop calipers failing to start if not used for a while - 10Feb24)
      refreshCalipers(5);
    }

  // zero all DRO readings
    for (int c=0; c < caliperCount; c++) {
      log_system_message("Axis " + calipers[c].title + " initial reading: " + String(calipers[c].reading));
      for (int i=0; i < noOfCoordinates; i++) {
        calipers[c].adj[i] = calipers[c].reading;
      }
    }

  drawScreen(1);     // display page 1 on CYD

  // report number of button widgets 
    if (widgetCount == 0) log_system_message("Button widget count is correct");
    if (widgetCount < 0)  log_system_message("ERROR: " + String(-widgetCount) + " more button widgets required!");
    if (widgetCount > 0)  log_system_message( String(widgetCount) + "Note: too many button widgets have been created");
}


// ----------------------------------------------------------------
//   -LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP
// ----------------------------------------------------------------

void loop() {

  if(wifiEnabled) server.handleClient();                  // service any web page requests

  refreshCalipers(1);                                     // refresh readings from calipers  

  // Touch screen
    bool st = ts.touched();                               // discover if screen is pressed
    if (st && !sTouched) {                                // if touchscreen is pressed and is not already flagged as pressed
      actionScreenTouch(ts.getPoint());                   // call procedure for when screen has been pressed
      sTouched = 1;                                       // flag that the screen is curently pressed
    } 
    if (!st && sTouched) {                                // if touchscreen has been released
      actionScreenRelease(ts.getPoint());                 // call procedure for when screen has been released
      sTouched = 0;                                       // clear screen pressed flag
    }

    // housekeeping
      static repeatTimer checkTimer;                      // set up a repeat timer (see standard.h)
      if (checkTimer.check(10000)) {                      // repeat at set interval (ms)
        esp_task_wdt_reset();                             // reset watchdog timer 
        WIFIcheck();                                      // check wifi connection is ok
        timeStatus();                                     // check if NTP time is ok
        time_t t=now();                                   // read current time to ensure NTP auto refresh keeps triggering (otherwise only triggers when time is required causing a delay in response)
      }

    delay(2);    
}


// ----------------------------------------------------------------
//                      -refresh calipers
// ----------------------------------------------------------------
// refresh readings from the digital calipers and update display if anything has changed (int = number of times to retry)
// returns 1 if data received ok from all calipers

bool refreshCalipers(int cRetry) {

  bool refreshDisplayFlag = 0;                                   // display will be refreshed if this flag is set
  int tCount                           ;                         // temp variables
  bool tOK[caliperCount] = {0};                                  // flag if data received form caliper ok

  // Digital Calipers
    for (int c=0; c < caliperCount; c++) {
      if (calipers[c].enabled) {                                 // if caliper is active
        tCount = cRetry;                                         // reset try counter 
        while (tCount > 0 && tOK[c] == 0) {
          float tRead = readCaliper(calipers[c].clockPIN, calipers[c].dataPIN, calipers[c].direction);   // read data from caliper (reading over 9999 indicates fail)
          if (tRead != calipers[c].reading) { // if reading has changed
            if (tRead < DROerrorCode || showDROerrors) {           // if reading is not error or show errors is enabled
              calipers[c].reading = tRead;                         // store result in global variable 
              refreshDisplayFlag = 1;                              // flag DRO display to refresh
              calipers[c].lastReadTime = millis();                 // log time of last reading
              tOK[c] = 1;                                          // flag reading received ok
            }
          }
          if (tRead >= DROerrorCode) {    
            if (serialDebug) Serial.println(calipers[c].title + " Caliper read failed: " + String(tRead));    
          }   
          tCount--;                                              // decrement try counter
          if (tOK[c] == 0) delay(2);                             // delay before retrying
        }  // while
      } else tOK[c] = 1;                                         // caliper is disabled so skip it
    }  

    if (refreshDisplayFlag) displayReadings();                   // display caliper readings 
    
    bool tOKres = 1;
    for (int c=0; c < caliperCount; c++) if (tOK[c] == 0) tOKres = 0;
    return tOKres;
}    


// ----------------------------------------------------------------
//       -root web page requested    i.e. http://x.x.x.x/
// ----------------------------------------------------------------

void handleRoot() {

  WiFiClient client = server.client();             // open link with client
  webheader(client);                               // html page header  (with extra formatting)

// set the web page's title?
//    client.printf("\n<script>document.title = \"%s\";</script>\n", stitle);

  // log page request including clients IP
    IPAddress cip = client.remoteIP();
    String clientIP = decodeIP(cip.toString());   // check for known IP addresses
    log_system_message("Root page requested from: " + clientIP);

  rootUserInput(client);     // Action any user input from this page

  // Build the HTML
    client.printf("<FORM action='%s' method='post'>\n", HomeLink);     // used by the buttons


// ---------------------------------------------------------------------------------------------
//  info which is periodically updated using AJAX - https://www.w3schools.com/xml/ajax_intro.asp

  // insert area which is later populated via vbscript with live data from http://x.x.x.x/data 
    client.println("<br><span id='uline' style='display: inline-block; min-height: 110px;'></span><br>");


  // Javascript - to periodically update the above info lines from http://x.x.x.x/data
    // get from http://x.x.x.x/data and populate the blank lines in html above

  // this is the below section compacted via https://www.textfixer.com/html/compress-html-compression.php
    client.printf(R"=====( <script> function getData() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById('uline').innerHTML = this.responseText; } }; xhttp.open('GET', 'data', true); xhttp.send();} getData(); setInterval(function() { getData(); }, %d); </script> )=====", dataRefresh * 1000);

/*
    client.printf(R"=====(
      <script>
          function getData() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              document.getElementById('uline').innerHTML = this.responseText;
            }
          };
          xhttp.open('GET', 'data', true);
          xhttp.send();}
          getData();
          setInterval(function() { getData(); }, %d);
      </script>
    )=====", dataRefresh * 1000);
*/

// ---------------------------------------------------------------------------------------------


    // textbox for entering gcode
      client.println("<br>STORED POSITIONS<br>");
      client.println("Enter list of positions you wish to step through in the format: X10 Y20 Z30<br>");
      client.println("or paste gcode &ensp; <a href='https://www.intuwiz.com/drilling.html' target='_new'>GCODE GENERATOR</a><br>");  
      client.println("<textarea id='gcode' name='gcode' rows='" + String(gcodeTextHeight) + "' cols='" + String(gcodeTextWidth) + "'></textarea><br>"); 

    // coordinate selection checkboxes
      client.println("Coordinates to process: ");
      for (int c=0; c < caliperCount; c++) {
        if (calipers[c].enabled) client.print(calipers[c].title + "<INPUT type='checkbox' name='GA" + calipers[c].title + "' value='1'>&ensp;");
      } 

    // misc option radio buttons
      client.println(R"=====(
        <INPUT type='submit' value='Action'><INPUT type='reset'><br>
        Toggle show caliper read fails<INPUT type='radio' name='RADIO1' value='1'>&ensp;
        Toggle show screen presses<INPUT type='radio' name='RADIO1' value='2'>&ensp;
        Clear stored positions<INPUT type='radio' name='RADIO1' value='3'><br>
      )=====");

    // link to github
      client.println("<br><a href='https://github.com/alanesq/DRO' target='_new'>PROJECT GITHUB</a>");

    // close page
      client.println("</P>");           // end of section
      client.println("</form>");        // end form section (used by buttons etc.)
      webfooter(client);                // html page footer
      delay(3);
      client.stop();
}


// ----------------------------------------------------------------
//            -Action any user input on root web page
// ----------------------------------------------------------------

void rootUserInput(WiFiClient &client) {

  // Misc option radio1 buttons
    if (server.hasArg("RADIO1")) {
      String RADIOvalue = server.arg("RADIO1");   // read value of the "RADIO1" argument

      // Show DRO errors
      if (RADIOvalue == "1") {
        showDROerrors = !showDROerrors;
        log_system_message("DRO reading error displaying set to :" + String(showDROerrors));
      }  

      // show touch press position 
      if (RADIOvalue == "2") {
        showPress = !showPress;
        log_system_message("Show screen presses set to :" + String(showPress));
      }  

      // clear stored positions
      if (RADIOvalue == "3") {
        clearGcode();
      }  
    }

  // gcode / stored positions
    if (server.hasArg("gcode")) {
      enteredGcode = server.arg("gcode");          // read gcode text
      if (enteredGcode != "") {      
        // set coordinates to use from radio buttons
          bool tFlag = 0;
          for (int c=0; c < caliperCount; c++) {
            if (server.arg("GA" + calipers[c].title) == "1") {    // if this axis is selected
              inc[c] = 1;
              tFlag = 1;
            } else {
              inc[c] = 0;
            }
          }
        // process the gcode 
          if (tFlag) {                                            // if at least one axis has been selected
            enteredGcode += "\n";
            processGCode(enteredGcode);                           // process the gcode and put resulting coordinates in to global arrays
          }
      }
    }
}


// ----------------------------------------------------------------
//     -data web page requested     i.e. http://x.x.x.x/data
// ----------------------------------------------------------------
// the root web page requests this periodically via Javascript in order to display updating information.

void handleData(){

   String reply; reply.reserve(600); reply = "";                                // reserve space to cut down on fragmentation
   char buff[200];                                                              // used for formatting of caliper readings

   // current time
    String cTime = currentTime();
    if (cTime != "Time Unknown") reply += "<br>Current time: " + cTime + "<br><br>";

   // DRO readings
    String spa = "%0" + String(DROnoOfDigits1 + DROnoOfDigits2 + 1) + ".2f";    // create sprintf argument for number format (in the format "%07.2f")
    reply += "DRO READINGS";
    for (int c=0; c < caliperCount; c++) {
      if (calipers[c].enabled) {
        reply += "<br>" + calipers[c].title + ":&ensp;";
        for (int i=0; i < noOfCoordinates; i++) {                               // step through all the coordinate systems
          if (i == currentCoord) reply += String(colBlue);                      // active coordinate
          sprintf(buff, spa.c_str(), calipers[c].reading - calipers[c].adj[i]);  
          reply += "C" + String(i+1) + ":" + String(buff) + "&ensp; &ensp;";
          if (i == currentCoord) reply += String(colEnd);   
        }
        sprintf(buff, spa.c_str(), calipers[c].reading);                        // the calipers actual reading
        reply += " Raw:" + String(buff);      
      }
    }
    reply += "<br><br>";

    // misc info.
      if (showPress) reply += String(colRed) + "<br>Touch screen data will be displayed" + String(colEnd);
      if (showDROerrors) reply += String(colRed) + "<br>Axis read fails will be displayed (code: " + String(DROerrorCode, 0) + ")" + String(colEnd);
      if (OTAEnabled) reply += String(colRed) + "<br>OTA is enabled" + String(colEnd);
      if (gcodeLineCount != 0) reply += "<br>There are " + String(gcodeLineCount) + " stored positions in memory";

   server.send(200, "text/plane", reply);                                      // Send millis value only to client ajax request
}


// ----------------------------------------------------------------
//      -ping web page requested     i.e. http://x.x.x.x/ping
// ----------------------------------------------------------------

void handlePing(){

  WiFiClient client = server.client();             // open link with client

  // log page request including clients IP address
    IPAddress cip = client.remoteIP();
    String clientIP = decodeIP(cip.toString());   // check for known IP addresses
    log_system_message("Ping page requested from: " + clientIP);

  server.send(200, "text/plain", "ok");   // send reply as plain text
}


// ----------------------------------------------------------------
//                    -settings stored in eeprom  (not used at present)
// ----------------------------------------------------------------
// @param   eDirection   1=write, 0=read
// Note on esp32 this has now been replaced by preferences.h although I still prefer this method - see: by https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/

void settingsEeprom(bool eDirection) {

    const int dataRequired = 60;   // total size of eeprom space required (bytes)

    int currentEPos = 0;           // current position in eeprom
    float ts;                      // temp stores
    byte tts;

    EEPROM.begin(dataRequired);

    if (eDirection == 0) {         // read settings

      // read number of coordinates and veryfy this is correct
        EEPROM.get(currentEPos,  tts);   
        if (tts != noOfCoordinates) {
          log_system_message("Error: invalid data read from eeprom");
          return;     
        }
        currentEPos += sizeof(tts);          

      // step through the coordinate systems
        for (int c = 0; c < noOfCoordinates; c++) {
          for (int i=0; i < caliperCount; i++) {
              EEPROM.get(currentEPos,  ts);                      // read data
              calipers[i].adj[c] = calipers[i].reading + ts;     // store data in variable
              currentEPos += sizeof(ts);                         // increment to next free position in eeprom         
          }
        }

        // current coordinate system in use
          EEPROM.get(currentEPos, currentCoord);     
          if (currentCoord < 0 || currentCoord > 2) currentCoord = 0;
          currentEPos += sizeof(currentCoord);  

      if (currentEPos > dataRequired) log_system_message("ERROR: Not enough space reserved for eeprom");
      else log_system_message("Data read from eeprom");

    } else {                      // store settings

      // number of coordinates to store
          EEPROM.put(currentEPos, noOfCoordinates);     // byte
          currentEPos += sizeof(noOfCoordinates);         

      // step through the coordinate systems
        for (int c = 0; c < noOfCoordinates; c++) {
          for (int i=0; i < caliperCount; i++) {
            ts = calipers[i].adj[c] - calipers[i].reading;
            EEPROM.put(currentEPos, ts);              // write to Eeprom
            currentEPos += sizeof(ts);                // increment to next free position in eeprom
          }
        }

        // current coordinate system in use
          EEPROM.put(currentEPos, currentCoord);
          currentEPos += sizeof(currentCoord);     

      EEPROM.commit();                                // write the data out (required on esp devices as they simulate eeprom)
      if (currentEPos > dataRequired) log_system_message("ERROR: Not enough space reserved to store to eeprom");
      else log_system_message("Data written to eeprom");
    }
}


// ----------------------------------------------------------------
//                     -page specific items
// ----------------------------------------------------------------

void pageSpecificOperations() {

  // some values to use for display positions
    const int lineSpace = 18;           // standard line spacing
    const int belowSmallDRO = 100;      // position just below DRO readouts
    const int rightOfSmallDRO = 100;    // position to right of DRO readouts

  // -------------------------------------------------------

  // page 2: Misc buttons

  if (displayingPage == 2) {

    // hide "enable wifi" button if wifi is enabled
      for (uint8_t b = 0; b < buttonCount; b++) {
        if (String(screenButtons[b].label) == "En. Wifi") {  
          if (screenButtons[b].enabled && wifiEnabled) {
            screenButtons[b].enabled = 0;
            drawScreen(2);         // not sure if this a good idea?
          }
        }
      }

    // show time and ip address
      tft.setFreeFont(FM9);         // standard Free Mono font - available sizes: 9, 12, 18 or 24   
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextSize(1);
      if (wifiEnabled) {
        String tIP;
        if (wifiType != 3) {
          // connected to a wifi
            tft.drawString(currentTime() ,0, belowSmallDRO);      
            tIP = WiFi.localIP().toString();  
        } else {
          // access point
          tIP =  WiFi.softAPIP().toString() + ":" + String(AP_SSID);  
        }
        tft.drawString("IP:" + tIP ,0, belowSmallDRO + lineSpace);    
      }   
  }    

  // -------------------------------------------------------

  // page 3: keypad

    // display number ented via keypad
      if (displayingPage == 3) {
        tft.setFreeFont(&sevenSeg16pt7b);      // 7 seg font (small)
        tft.setTextColor(TFT_BLUE, TFT_BLACK);
        tft.setTextSize(1);
        tft.setTextPadding( tft.textWidth("8") * noDigitsOnNumEntry );  
        tft.drawString(keyEnteredNumber, keyX, keyY - 30 );       
      }

  // -------------------------------------------------------

  // page 4: step through gcode coordinates

    // reset gcode DRO adjustments if no longer on page 4
      if (displayingPage != 4)  {
        for (int c=0; c < caliperCount; c++) gcodeDROadj[c] = 0;
      }

    if (displayingPage == 4) {

      // set position coordinates on DRO 
        for (int c=0; c < caliperCount; c++) {
          gcodeDROadj[c] = 0;
          if (inc[c]) gcodeDROadj[c] = gcode[c][gcodeStepPosition - 1];  
        }

      // page title
        tft.setFreeFont(FM9);                // standard Free Mono font - available sizes: 9, 12, 18 or 24   
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextSize(1);
        tft.drawString("  STEP THROUGH" , rightOfSmallDRO, lineSpace * 0);
        tft.drawString("  COORDINATES" , rightOfSmallDRO, lineSpace * 1);

      // display coordinates
        if (gcodeLineCount > 0) {                                     // if there is data to use
          tft.drawString(" Position: " + String(gcodeStepPosition) + " of " + String(gcodeLineCount) + "  " , 0, belowSmallDRO + lineSpace * 1);
          // display coordinates
            String tRes = "";
            for (int c=0; c < caliperCount; c++) {
              if (inc[c]) tRes += " " + calipers[c].title + ":" + String(gcode[c][gcodeStepPosition - 1], DROnoOfDigits2);
            }
            tft.setTextPadding(SCREEN_WIDTH - pageButtonWidth);       // this clears the previous text 
            tft.drawString(tRes, 0, belowSmallDRO + lineSpace * 3);
            tft.setTextPadding(0);                                    // reset padding
        } else {                                                      // no data to use
            tft.drawString(" No data to display" , 0, belowSmallDRO + lineSpace * 1);
            if (wifiEnabled) {
              tft.drawString(" To enter data visit", 0, belowSmallDRO + lineSpace * 2);
              String tIP = WiFi.localIP().toString();                                            // wifi mode
              if (tIP == "0.0.0.0") tIP =  WiFi.softAPIP().toString() + ":" + String(AP_SSID);   // access point mode              
              tft.drawString(" http://" + tIP, 0, belowSmallDRO + lineSpace * 3);
            } else {
              tft.drawString(" Enable wifi to use", 0, belowSmallDRO + lineSpace * 2);
            }
        }
      } 
}


// ----------------------------------------------------------------
//                -Touch screen has been released
// ----------------------------------------------------------------

void actionScreenRelease(TS_Point p) {

  tft.setFreeFont(MENU_FONT);      
  tft.setTextSize(MENU_SIZE);  

  if (serialDebug) Serial.println("button released");

  // flag all buttons released and re-draw them
    for (uint8_t b = 0; b < buttonCount; b++) {
      screenButtons[b].btn->press(false);  // flag button released
      if (screenButtons[b].enabled == 1 && ( screenButtons[b].page == displayingPage || screenButtons[b].page == 0) ) {
        screenButtons[b].btn->press(false);      
        screenButtons[b].btn->drawSmoothButton(false);      // draw button
      }
    }

  highlightActiveButtons();      // indicate which page is being displayed
}


// ----------------------------------------------------------------
//                -Touch screen has been pressed
// ----------------------------------------------------------------

void actionScreenTouch(TS_Point p) {

  // calculate position on screen
    int x = map(p.x, TOUCH_LEFT, TOUCH_RIGHT, 0, SCREEN_WIDTH);  
    int y = map(p.y, TOUCH_TOP, TOUCH_BOTTOM, 0, SCREEN_HEIGHT);  
    if (serialDebug) Serial.println("Touch data: " + String(p.x) + "," + String(p.y) + "," + String(x) + "," + String(y));    

  // check if a button has been pressed
    tft.setFreeFont(MENU_FONT);    
    tft.setTextSize(MENU_SIZE);        
    for (uint8_t b = 0; b < buttonCount; b++) {
      if (screenButtons[b].enabled == 1 && ( screenButtons[b].page == displayingPage || screenButtons[b].page == 0) && screenButtons[b].btn->contains(x, y)) {
        // button is pressed
          screenButtons[b].btn->drawSmoothButton(true);
          if (serialDebug) Serial.println("button " + String(screenButtons[b].label) + " pressed!");
          screenButtons[b].btn->press(true);
          screenButtons[b].btn->pressAction();      // call procedure in buttons.h
          lastTouch = millis();          // flag time a button was pressed   
          break;                         // exit for loop   
      } 
    }    

  pageSpecificOperations();              // draw any page specific items
  displayReadings();                     // redraw DRO readings
  highlightActiveButtons();                 // indicate which page is being displayed

  // draw data on screen if showPress is enabled
    if (showPress) {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextSize(1);
      String temp = "X = " + String(p.x) + "  ";
      tft.drawCentreString(temp, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 10, 1);
      temp = "Y = " + String(p.y) + "  ";
      tft.drawCentreString(temp,  SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 10, 1);
    }

  // send touch data to serial port
  if (serialDebug) {
    Serial.print("Pressure = ");
    Serial.print(p.z);
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.print(p.y);
    Serial.println();  
  }
}


// ----------------------------------------------------------------
//               -highlight active buttons on CYD
// ----------------------------------------------------------------
// i.e. identify which buttons are acive

void highlightActiveButtons() {
  tft.setFreeFont(MENU_FONT);     
  tft.setTextSize(MENU_SIZE);    

  // page buttons
    for (uint8_t b = 0; b < buttonCount; b++) {
      if (String(screenButtons[b].label) == "P" + String(displayingPage)) screenButtons[b].btn->drawSmoothButton(true);
    }    

  // coordinate system buttons
    if (displayingPage == 1) {
      for (uint8_t b = 0; b < buttonCount; b++) {
        if (String(screenButtons[b].label) == "C" + String(currentCoord + 1)) screenButtons[b].btn->drawSmoothButton(true);
      }       
    }
}


// ----------------------------------------------------------------
//                   -draw the screen on CYD
// ----------------------------------------------------------------
// screen = page number to show

void drawScreen(int screen) {

  if (serialDebug) Serial.println("Displaying page " + String(screen));

  displayingPage = screen;      // set current page

  tft.fillScreen(TFT_BLACK);    // clear screen
  tft.setFreeFont(MENU_FONT);   
  tft.setTextSize(MENU_SIZE);

  // draw the buttons
    for (uint8_t b = 0; b < buttonCount; b++) {
        if( screenButtons[b].enabled == 1 && (screenButtons[b].page == screen || screenButtons[b].page == 0) ) {       // 0 = show on all pages
          screenButtons[b].btn->drawSmoothButton(false, screenButtons[b].outlineThickness, TFT_BLACK);      
          //screenButtons[b].btn->press(0);     // flag as not pressed
        }
    } 
  
  pageSpecificOperations();   // draw any page specific items
  displayReadings();          // display DRO readings
  highlightActiveButtons();   // indicate which page is being displayed and which coordinate is active
}


// ----------------------------------------------------------------
//        -wait for gpio pin to be at the specified state 
// ----------------------------------------------------------------
// returns 0 if it times out (used by 'read caliper data')

bool waitPinState(int pin, bool state, int timeout) {
  unsigned long tTimer = millis();  
  while(digitalRead(pin) != state && millis() - tTimer < timeout) { }
  if (millis() - tTimer >= timeout) return 0;
  return 1;
}


// ----------------------------------------------------------------
//                       -read caliper data
// ----------------------------------------------------------------
// The data is sent as 24bits several times a second so we first ensure we are at the start of one of these 
//    streams by measuring how long the clock pin was high for then we read in the the data

float readCaliper(int clockPin, int dataPin, bool reverseDirection) {

  // settings
    const int longPeriod = 500;                                  // length of time which counts as a long period (microseconds)
    const int lTimeout = 200;                                    // timeout when waiting for start of data (ms) 150
    const int sTimeout = 100;                                     // timeout when reading data (ms) 60

  // logic levels on data and clock pins (if using a transistor to level shift then these will be inverted)
    const bool pinLOW = invertCaliperDataSignals;
    const bool pinHIGH = !pinLOW;

  // start of data is preceded by clock being high for long period so verify we are at this point (this is the case most of the time)
    if (!waitPinState(clockPin, pinHIGH, lTimeout)) return DROerrorCode + 0.11;    // if clock is low wait until it is high (9999.x signifies it timed out waiting)
    unsigned long tmpTime=micros();                            // start timer
    if (!waitPinState(clockPin, pinLOW, lTimeout)) return DROerrorCode + 0.22;     // wait for clock pin to go low
    if ((micros() - tmpTime) < longPeriod) return DROerrorCode + 0.33;             // verify it was high for a long period signifying start of data 

  // start of data confirmed so now read in the data
    int sign = 1;                                              // if the reading is negative (1 or -1)
    int inches = 0;                                            // if data is in inches or mm (0 = mm) 
    long value = 0;                                            // the measurement received  
    for(int i=0;i<24;i++) {                                    // step through the data bits 
      if (!waitPinState(clockPin, pinLOW, sTimeout)) return DROerrorCode + 0.44;      // If clock is not low wait until it is
      if (!waitPinState(clockPin, pinHIGH, sTimeout)) return DROerrorCode + 0.55;     // wait for clock pin to go HIGH (this tells us the next data bit is ready to be read)
      if(digitalRead(dataPin) == pinHIGH) {                    // read data bit - if it is 1 then act upon this (0 can be ignored as all bits defult to 0)
          if(i<20) value|=(1<<i);                              // shift the 19 bits in to read value
          if(i==20) sign=-1;                                   // if reading is positive or negative
          if(i==23) inches=1;                                  // if reading is in inches or mm
      } 
    }

  // convert result to measurement
    float tRes;
    if (inches) tRes = (value * sign) / 2000.00;               // inches
    else tRes = (value * sign) / 100.00;                       // mm

  // reverse direction if required
    if (reverseDirection) tRes = -tRes;

  return tRes;
}


// ----------------------------------------------------------------
//                   -display caliper readings
// ----------------------------------------------------------------

void displayReadings() {

    const unsigned long warningTimeLimit = 2000;       // if caliper reading has not updated in this time change display to blue (ms)

    // create sprint argument  (in the format "%07.2f")
      String spa = "%0" + String(DROnoOfDigits1 + DROnoOfDigits2 + 1) + "." + String(DROnoOfDigits2) + "f";    // https://alvinalexander.com/programming/printf-format-cheat-sheet/
   
    // font size
      if (displayingPage == 1) tft.setFreeFont(&sevenSeg35pt7b);         // seven segment style font from sevenSeg.h  
      else tft.setFreeFont(&sevenSeg16pt7b);

    // Cheap Yellow Display
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextSize(1);
      //tft.setTextPadding(DROwidth);           // this clears the previous text 
      tft.setTextPadding( tft.textWidth("8") * (DROnoOfDigits1 + DROnoOfDigits2) );           // this clears the previous text

    char buff[50];

    // calipers
      for (int c=0; c < caliperCount; c++) {
        if (calipers[c].enabled) {
          sprintf(buff, spa.c_str(), calipers[c].reading - calipers[c].adj[currentCoord] - gcodeDROadj[c]); 
          if (strlen(buff) == DROnoOfDigits1 + DROnoOfDigits2 +1) tft.drawString(buff, 0, c * tft.fontHeight());    
          else if (serialDebug) Serial.println("Invalid reading from " + calipers[c].title + " " + String(buff));          
        }
      }
   
    tft.setFreeFont(FM12);        // switch back to standard Free Mono font - available sizes: 9, 12, 18 or 24   
    tft.setTextPadding(0);        // clear the padding setting

    tft.setTextPadding(0);        // turn off text padding
}


// ----------------------------------------------------------------
// -simulate press of touch screen     i.e. http://x.x.x.x/touch?x=310&y=10
// ----------------------------------------------------------------

void handleTouch() {

  WiFiClient client = server.client();          // open link with client

  // read x and y requested
    int x=-1, y=-1;

    if (server.hasArg("x")) {
      String Tvalue = server.arg("x");   // read value
      if (Tvalue != NULL) {
        int val = Tvalue.toInt();     
        if  (val >= 0 && val <= SCREEN_WIDTH) x = val;
      }
    }

    if (server.hasArg("y")) {
      String Tvalue = server.arg("y");   // read value
      if (Tvalue != NULL) {
        int val = Tvalue.toInt();     
        if  (val >= 0 && val <= SCREEN_HEIGHT) y = val;
      }    
    }

  // log page request including clients IP address
    IPAddress cip = client.remoteIP();
    String clientIP = decodeIP(cip.toString());   // check for known IP addresses
    log_system_message("Simulate touchscreen press requeste by " + clientIP + ": x=" + String(x) + ", y=" + String(y));    

  // simulate screen press (screen is 320 x 240)
    if (x >= 0 && y >= 0) {
      // draw circle on screen showing location
        const int radius = 4;
        tft.drawCircle(x, y, radius, TFT_RED);    
        tft.fillCircle(x, y, radius - 1, TFT_YELLOW);
      // map from screen to touch coordinates
        x = map(x, 0, SCREEN_WIDTH, TOUCH_LEFT, TOUCH_RIGHT);
        y = map(y, 0, SCREEN_HEIGHT, TOUCH_TOP, TOUCH_BOTTOM);    
      // send click
        TS_Point click = TS_Point(x, y, 3000);
        actionScreenTouch(click);
    }

  String message = "ok";
  server.send(200, "text/plain", message);   // send reply as plain text    
}

// ----------------------------------------------------------------
//                        -process gcode 
// ----------------------------------------------------------------
// process the entered gcode (from web page) extracting an x,y,z position from each line and store in global array

void processGCode(String &gcodeText) {
  
  if (gcodeText == "") return;            // if text is blank

  float t[caliperCount] = {0};            // extracted position
  float pt[caliperCount] = {99999999.0};  // previous position extracted

  // Split the G-code string into lines and process each line
  int startPos = 0;
  int endPos = 0;
  gcodeLineCount = 0;                                      // global variable storing number of coordinates extracted from the gcode

  while (endPos != -1 && gcodeLineCount < maxGcodeStringLines) {
    endPos = gcodeText.indexOf('\n', startPos);
    if (endPos != -1) {
      String gcodeLine = gcodeText.substring(startPos, endPos);
      processGCodeLine(gcodeLine, t);             // pass the line of gcode which updates position of x,y,z based upon this line (99999999 = not found)
      startPos = endPos + 1;

      // process line if different to previous coordinate
        bool tFlag = 0;   

        for (int c=0; c < caliperCount; c++) { 
          if (inc[c] == 1 && t[c] < 9999999) {                   // if this coordinate is active and a reading for this axis has been extracted from this line
            if (pt[c] != t[c]) {                                 // if reading is different to the previous one
              // new reading 
                pt[c] = t[c];
                // log_system_message("new position for " + calipers[c].title + " = " + String(t[c]) );         // for debugging
                tFlag = 1;          
            }
          }
        }

        if (tFlag == 1) {
          for (int c=0; c < caliperCount; c++) gcode[c][gcodeLineCount] = pt[c];
          gcodeLineCount ++;
        }
    }
  }   // while

  if (gcodeLineCount >= maxGcodeStringLines) log_system_message("Error: gcode exceded maximum length");
  log_system_message(String(gcodeLineCount) + " coordinates extracted from gcode");
}


// ----------------------------------------------------------------
//                     -process a line of gcode
// ----------------------------------------------------------------
// This takes a String containing a line of basic gcode and updates positions -  It simply finds an 'x', 'y' or 'z' in the string and extracts the number following it
// if not found it returns 999.69

void processGCodeLine(String gcodeLine, float Pos[]) {

  // extract values
  gcodeLine.toUpperCase();                                         // convert gcode line to upper case
  for (int c=0; c < caliperCount; c++) {      
    int PosIndex = gcodeLine.indexOf(calipers[c].title);           // search for caliper on the line          
    if (PosIndex != -1) {   
      Pos[c] = gcodeLine.substring(PosIndex + 1).toFloat();        // found so read the value 
    } else Pos[c] = 99999999.0;                                    // not found 
  }

  if (serialDebug) {
  // Print the extracted position
    for (int c=0; c < caliperCount; c++) {
      Serial.print(" " + calipers[c].title +":");
      Serial.print(Pos[c]);
    }
    Serial.println();
  }
}


// ----------------------------------------------------------------
//           -stored coordinates     i.e. http://x.x.x.x/stored
// ----------------------------------------------------------------
// Display the stored coordinates from gcode

void handelStored() {

  WiFiClient client = server.client();          // open link with client

  // log page request including clients IP address
    IPAddress cip = client.remoteIP();
    String clientIP = decodeIP(cip.toString());   // check for known IP addresses
    log_system_message("Stored positions list page requested from: " + clientIP);

  webheader(client);                 // add the standard html header
  client.println("<br><H2>STORED POSITIONS</H2>");

  if (gcodeLineCount == 0) {
    client.println("<br>No stored positions to display<br>");
  } else {
    // Display stored positions
    client.println("There are " + String(gcodeLineCount) + " stored positions");
    for(int l=0; l < gcodeLineCount; l++) {                   // step through all stored positions
      client.println("<br>" + String(l + 1) + ":&ensp;");
      if (l == gcodeStepPosition - 1) client.print(String(colBlue));
      for (int c=0; c < caliperCount; c++) {                  // step through all available axis
        if (inc[c] == 1) {                                    // if this axis is selected for the gcode
          client.println(calipers[c].title + String(String(gcode[c][l])) + "&ensp;"  );
        }
      }
      if (l == gcodeStepPosition - 1) client.print(String(colEnd));
    }
  }
  client.println("<br>");

  // end html page
    webfooter(client);            // add the standard web page footer
    delay(1);
    client.stop();
}


// ----------------------------------------------------------------
//                       -clear stored gcode 
// ----------------------------------------------------------------

void clearGcode() {
    gcodeLineCount = 0;                                      // set number of positions stored to zero
    gcodeStepPosition = 0;                                   // set current position in list to be zero
    for (int c=0; c < caliperCount; c++) gcode[c][0] = 0;    // set first entry to zero
    log_system_message("Stored positions have been cleared");
}


// ----------------------------------------------------------------
//           -testing page     i.e. http://x.x.x.x/test
// ----------------------------------------------------------------
// Use this area for experimenting/testing code

void handleTest(){

  WiFiClient client = server.client();          // open link with client

  // log page request including clients IP address
    IPAddress cip = client.remoteIP();
    String clientIP = decodeIP(cip.toString());   // check for known IP addresses
    //log_system_message("Test page requested from: " + clientIP);

  webheader(client);                 // add the standard html header
  client.println("<br><H2>TEST PAGE</H2><br>");

  


  // ---------------------------- test section here ------------------------------


// // turn backloight off then on
//   digitalWrite(SCREEN_BACKLIGHT, LOW);       // turn backlight on
//   delay(3000);
//   digitalWrite(SCREEN_BACKLIGHT, HIGH);       // turn backlight on


// // simulate half x button press
//   halfXpressed(); 


// // switch between pages
//   if (displayingPage < 3) drawScreen(displayingPage + 1);  
//   else  drawScreen(1);  
//   displayKeypadNo(4);
//   displayKeypadNo(2);


// // demo of how to request a web page
//   // request web page
//     String page = "http://192.168.1.166/ping";   // url to request
//     //String page = "http://192.168.1.176:84/rover.asp?action=devE11on";   // url to request
//     String response;                             // reply will be stored here
//     int httpCode = requestWebPage(&page, &response);
//   // display the reply
//     client.println("Web page requested: '" + page + "' - http code: " + String(httpCode));
//     client.print("<xmp>'");     // enables the html code to be displayed
//     client.print(response);
//     client.println("'</xmp>");
//   // test the reply
//     response.toLowerCase();                    // convert all text in the reply to lower case
//     int tPos = response.indexOf("closed");     // search for text in the reply - see https://www.arduino.cc/en/Tutorial/BuiltInExamples
//     if (tPos != -1) {
//       client.println("rusult contains 'closed' at position " + String(tPos) + "<br>");
//     }




  // -----------------------------------------------------------------------------


  // end html page
    webfooter(client);            // add the standard web page footer
    delay(1);
    client.stop();
}

// --------------------------- E N D -----------------------------
