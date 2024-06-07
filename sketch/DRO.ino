/*******************************************************************************************************************
 *
 *                                              SuperLowBudget-DRO               03jun24
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

#if (!defined ESP32)
  #error This code is for the ESP32 based 'Cheap Yellow Display' only
#endif

#include <Arduino.h>                      // required to use Strings 
String enteredGcode;                      // store for the entered gcode on web page
#include "settings.h"                     // load in settings for the DRO from settings.h file

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
  void displayReadings(bool clearFirst = 0);
  void startTheWifi();
  bool refreshCalipers(int cRetry, bool display = 1); 
  void clearGcode();
  void plotGraph(WiFiClient &client, int axis1, int axis2);
  float mapf(float x, float in_min, float in_max, float out_min, float out_max);
  void loadTestPositions();
  


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

    
    int displayingPage = 1;           // which screen page is live

    typedef void (*buttonActionCallback)(void);   // create a function pointer
    
    // struct to create a button
    struct buttonStruct {
      int page;                       // which screen page it belongs to (0 = show on all pages)
      bool enabled;                   // if this button is enabled
      String label;                   // buttons title
      int16_t x, y;                   // location on screen (top left)
      uint16_t width, height;
      uint16_t outlineColour;
      uint16_t outlineThickness;
      uint16_t fillColour;
      uint16_t textColour;
      ButtonWidget* btn;               // button object (widget)
      buttonActionCallback action;     // procedure called when button pressed
    };


  // // create button widgets - better technique but I think this would need to be in setup?
  //   const int numberOfWidgets = 45;    // the number of buttons required
  //   ButtonWidget butnW[numberOfWidgets] = {nullptr};
  //   for (int i = 0; i < numberOfWidgets; ++i) {
  //     buttonWidgets[i] = new ButtonWidget(&tft);
  //   }    

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
    dro.analogMeter(40, 95, 100.0, "STARTUP", "0", "25", "50", "75", "100");       // start at 0

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

  // ESP32 Watchdog timer -    Note: esp32 board manager v3.x.x requires different code
    #if defined ESP32
      esp_task_wdt_deinit();                  // ensure a watchdog is not already configured
      #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR == 3  
        // v3 board manager detected
        // Create and initialize the watchdog timer(WDT) configuration structure
          if (serialDebug) Serial.println("v3 esp32 board manager detected");
          esp_task_wdt_config_t wdt_config = {
              .timeout_ms = WDT_TIMEOUT * 1000, // Convert seconds to milliseconds
              .idle_core_mask = 1 << 1,         // Monitor core 1 only
              .trigger_panic = true             // Enable panic
          };
        // Initialize the WDT with the configuration structure
          esp_task_wdt_init(&wdt_config);       // Pass the pointer to the configuration structure
          esp_task_wdt_add(NULL);               // Add current thread to WDT watch    
          esp_task_wdt_reset();                 // reset timer
          if (serialDebug) Serial.println("Watchdog Timer initialized at WDT_TIMEOUT seconds");
      #else
        // pre v3 board manager assumed
          if (serialDebug) Serial.println("older esp32 board manager assumed");
          esp_task_wdt_init(WDT_TIMEOUT, true);                      //enable panic so ESP32 restarts
          esp_task_wdt_add(NULL);                                    //add current thread to WDT watch   
      #endif
    #endif    

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
 
  dro.updateNeedle(75, MeterSpeed);     // move dial to 75
  //delay(1000); 
  //tft.fillScreen(TFT_BLACK);             // Clear the screen 
  
  // caliper gpio pins
    for (int x=0; x < caliperCount; x++) {
      if (calipers[x].enabled) {
        pinMode(calipers[x].clockPIN, INPUT);
        pinMode(calipers[x].dataPIN, INPUT);
      }
    }
  
  // get readings from calipers     
    int tCount = 4;                      // Maximum number of attempts
    bool q = 0;
    while (tCount > 0 && q == 0) {
      q = refreshCalipers(3,0);          // Read calipers, returns TRUE if all calipers returned valid data (max 3 attempts per caliper and do not display result on screen)
      if (q == 0) delay(600);            // if not all calipers have returned a valid reading
      tCount --;
    }
    if (q ==0) log_system_message("Error: problem reading callipers");

  // zero all DRO readings
    for (int c=0; c < caliperCount; c++) {
      if (calipers[c].enabled) log_system_message("Axis " + calipers[c].title + " initial reading: " + String(calipers[c].reading));
      for (int i=0; i < noOfCoordinates; i++) {
        calipers[c].adj[i] = calipers[c].reading;
      }
    }

  dro.updateNeedle(100, MeterSpeed);    // move dial to 100 
  drawScreen(1);                        // display page 1 on CYD

  // report number of button widgets (as no way to auto create the correct number as far as I am aware?)
    if (widgetCount == 0) log_system_message("Button widget count is correct");
    if (widgetCount < 0)  log_system_message("ERROR: " + String(-widgetCount) + " more button widgets required!");
    if (widgetCount > 0)  log_system_message( "Note: " + String(widgetCount) + " too many button widgets have been created");
}


// ----------------------------------------------------------------
//   -LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP
// ----------------------------------------------------------------

void loop() {

  if(wifiEnabled) server.handleClient();                  // service any web page requests

  refreshCalipers(2);                                     // refresh readings from calipers 

  // Touch screen
    bool st = ts.touched();                               // discover if touch screen is pressed
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
      if (checkTimer.check(8000)) {                       // repeat at set interval (ms)
        esp_task_wdt_reset();                             // reset watchdog timer 
        WIFIcheck();                                      // check wifi connection is ok
        timeStatus();                                     // check if NTP time is ok
        time_t t=now();                                   // read current time to ensure NTP auto refresh keeps triggering (otherwise only triggers when time is required causing a delay in response)
      }  
}


// ----------------------------------------------------------------
//                      -refresh calipers
// ----------------------------------------------------------------
// refresh readings from the digital calipers and update display if anything has changed (int = number of times to retry reading caliper)
// returns 1 if data received ok from all calipers.  
// if display set to false the screen is not updated but the reading is stored even if in error state

bool refreshCalipers(int cRetry, bool display) {

  bool refreshDisplayFlag = 0;                                     // display will be refreshed if this flag is set
  int tCount                           ;                           // temp variables
  bool tOK[caliperCount];                                          // flag if data received form caliper ok
  for (int i=0; i < caliperCount; i++) tOK[i] = 0;

  // Digital Calipers
    for (int c=0; c < caliperCount; c++) {
      if (calipers[c].enabled) {                                   // if caliper is active
        tCount = cRetry;                                           // reset try counter 
        while (tCount > 0 && tOK[c] == 0) {
          float tRead = readCaliper(c);                            // read data from caliper 
          if (tRead < lowestAllowedReading || tRead > highestAllowedReading) calipers[c].error = 1;     // verify reading is in valid range
          if (calipers[c].error == 0) {                            // if caliper read data ok 
            tOK[c] = 1;                                            // flag reading received 
            if (tRead != calipers[c].reading  || calipers[c].lastReadTime == 0) {                       // if reading has changed or this is forst reading
              calipers[c].reading = tRead;                         // store result in global variable 
              refreshDisplayFlag = 1;                              // flag DRO display to update
              calipers[c].lastReadTime = millis();                 // log time of last reading received
            }
          } else {
            // read error
              if (serialDebug) Serial.println(calipers[c].title + " Caliper read failed: " + String(tRead));    
              if (showDROerrors) refreshDisplayFlag = 1;           // if show errors set then flag to display the error
          }   
          tCount --;                                               // decrement try counter
          if (tOK[c] == 0) delay(11);                              // delay before retrying
        }  // while
      } else tOK[c] = 1;                                           // caliper is disabled so skip it
    }  

    if (refreshDisplayFlag && display) displayReadings();          // display caliper readings 
    
    bool tOKres = 1;
    for (int c=0; c < caliperCount; c++) if (tOK[c] == 0 && calipers[c].enabled) tOKres = 0;
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
      client.println("or paste gcode &ensp; <a href='https://www.intuwiz.com/drilling.html' target='_new'>INTUWIZ ONLINE GCODE GENERATOR</a><br>");  
      client.println("<textarea id='gcode' name='gcode' rows='" + String(gcodeTextHeight) + "' cols='" + String(gcodeTextWidth) + "'></textarea><br>"); 

    // coordinate selection checkboxes
      client.println("Coordinates to process: ");
      for (int c=0; c < caliperCount; c++) {
        if (calipers[c].enabled) client.print(calipers[c].title + "<INPUT type='checkbox' name='GA" + calipers[c].title + "' value='1'>&ensp;");
      } 

    // misc option radio buttons
      client.println(R"=====(
        <INPUT type='submit' value='Action'>&ensp;<INPUT type='reset'><br>
        Toggle show read errors<INPUT type='radio' name='RADIO1' value='1'>&ensp;
        Toggle show screen presses<INPUT type='radio' name='RADIO1' value='2'>&ensp;
        Load demo positions<INPUT type='radio' name='RADIO1' value='3'>&ensp;
        Clear stored positions<INPUT type='radio' name='RADIO1' value='4'><br>
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
        loadTestPositions();
      }  

      // clear stored positions
      if (RADIOvalue == "4") {
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
    // create html table
      int cWidth = 100 * (noOfCoordinates + 1);                                 // table width
      reply += "<br><table style='text-align: centre; margin-left: auto; margin-right: auto; background-color: #D0D000; width: " + String(cWidth) + "px; table-layout: fixed;'>"; 
    // table column titles
      reply += "<tr><th></th>";
      for (int i=0; i < noOfCoordinates; i++) {   
        reply += "<th>C" + String(i + 1) + "</th>";
      }
      reply += "<th>Raw</th></tr><tr>";

    // display readings  
      for (int c=0; c < caliperCount; c++) {                                    // step through each caliper
        if (calipers[c].enabled) {
          reply += "<th>" + calipers[c].title + "</th>";
          for (int i=0; i < noOfCoordinates; i++) {                             // step through each coordinate systems
            reply += "</td><td>";                                               // next table column
            float tReading = calipers[c].reading - calipers[c].adj[i];          // calculate current reading
            sprintf(buff, spa.c_str(), tReading);                               // format the reading for display
            // display the reading 
              if (i == currentCoord) reply += String(colBlue);                  // if the active coordinate
              if ( (tReading >= lowestAllowedReading && tReading <= highestAllowedReading && calipers[c].lastReadTime != 0) || showDROerrors) 
                reply += String(buff);
              if (i == currentCoord) reply += String(colEnd);     
          }
          reply += "</td><td>";
          if (calipers[c].lastReadTime != 0) reply += String(calipers[c].reading);      // raw caliper reading
          else reply += "No-data";
          if (showDROerrors && calipers[c].error != 0) reply += ":Error" + String(calipers[c].error);
          reply += "</td></tr>";                                                // next table row
        }
      }
    reply += "</table>";                                                        // end of html table

    // misc info.
      if (showPress) reply += String(colRed) + "<br>Touch screen data will be displayed" + String(colEnd);
      if (showDROerrors) reply += String(colRed) + "<br>Axis read errors will be displayed" + String(colEnd);
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
          if (inc[c]) gcodeDROadj[c] = gcode[c][gcodeStepPosition - 1];           // if this axis is selected for the gcode
        }

      // page title
        tft.setFreeFont(FM9);                // standard Free Mono font - available sizes: 9, 12, 18 or 24   
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextSize(1);
        tft.drawString("  STEP THROUGH" , rightOfSmallDRO, lineSpace * 0);
        tft.drawString("  STORED" , rightOfSmallDRO, lineSpace * 1);
        tft.drawString("  POSITIONS" , rightOfSmallDRO, lineSpace * 2);

      // display coordinates
        if (gcodeLineCount > 0) {                                     // if there is data to use
          tft.drawString(" Position: " + String(gcodeStepPosition) + " of " + String(gcodeLineCount) + "   " , 0, belowSmallDRO + lineSpace * 1);
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
        displayReadings(1);     // display readings with clear display flag set (so invalid readings leave a blank)
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
  unsigned long timeoutMicros = micros() + timeout;      // what micros() will be when timeout is reached
  while(digitalRead(pin) != state && micros() < timeoutMicros) { }
  if (micros() >= timeoutMicros) return 0;
  return 1;
}


// ----------------------------------------------------------------
//                       -read caliper data
// ----------------------------------------------------------------
// The data is sent continually as a 24bit data stream.  The data takes 9ms with a 115ms gap between (i.e. around 8 times a second) 
//    streams by measuring how long the clock pin was high for then we read in the the data

float readCaliper(int caliperNumber) {

  // data timings (microseconds)
    const int longPeriod = 900;                                // minimum length of time which counts as a long period  
    const int lTimeout = 125000;                               // timeout when waiting for start of data  
    const int sTimeout = 800;                                  // timeout when reading data bits 

  // logic levels on data and clock pins (if using a transistor to level shift then these will be inverted)
    const bool pinLOW = invertCaliperDataSignals;
    const bool pinHIGH = !invertCaliperDataSignals;

  calipers[caliperNumber].error = 0;                           // reset caliper read error flag

  // start of data is preceded by clock being high for long period so verify we are at this point (this is the case most of the time)
    if (!waitPinState(calipers[caliperNumber].clockPIN, pinHIGH, lTimeout)) {       // if clock is low wait until it is high
      calipers[caliperNumber].error = 2;
      return calipers[caliperNumber].reading;                                       // return previous reading  
    }
    unsigned long tmpTime=micros();                            // start timer
    if (!waitPinState(calipers[caliperNumber].clockPIN, pinLOW, lTimeout)) {        // wait for clock pin to go low
        calipers[caliperNumber].error = 3;
        return calipers[caliperNumber].reading;                                     // return previous reading  
    }
    if ((micros() - tmpTime) < longPeriod) {                   // verify it was high for a long period signifying start of data 
      calipers[caliperNumber].error = 4;
      return calipers[caliperNumber].reading;                          // return previous reading     
    }

  // start of data confirmed so now read in the data
    int sign = 1;                                              // if the reading is negative (1 or -1)
    int inches = 0;                                            // if data is in inches or mm (0 = mm) 
    long value = 0;                                            // the measurement received  
    for(int i=0;i<24;i++) {                                    // step through the data bits 
      if (!waitPinState(calipers[caliperNumber].clockPIN, pinLOW, sTimeout)) {      // If clock is not low wait until it is
        calipers[caliperNumber].error = 5;
        return calipers[caliperNumber].reading;                                             // return previous reading 
      }      
      if (!waitPinState(calipers[caliperNumber].clockPIN, pinHIGH, sTimeout)) {     // wait for clock pin to go HIGH (this tells us the next data bit is ready to be read)
        calipers[caliperNumber].error = 6;
        return calipers[caliperNumber].reading;                                             // return previous reading 
      }        
      if(digitalRead(calipers[caliperNumber].dataPIN) == pinHIGH) {                 // read data bit - if it is 1 then act upon this (0 can be ignored as all bits defult to 0)
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
    if (calipers[caliperNumber].direction) tRes = -tRes;

  return tRes;
}


// ----------------------------------------------------------------
//                   -display caliper readings
// ----------------------------------------------------------------

void displayReadings(bool clearFirst) {

    const unsigned long warningTimeLimit = 2000;    // if caliper reading has not updated in this time change display to blue (ms)

    // create sprint argument  (in the format "%07.2f")
      String spa = "%0" + String(DROnoOfDigits1 + DROnoOfDigits2 + 1) + "." + String(DROnoOfDigits2) + "f";    // https://alvinalexander.com/programming/printf-format-cheat-sheet/
   
    // font size
      if (displayingPage == 1) tft.setFreeFont(&sevenSeg35pt7b);                                   // seven segment style font from sevenSeg.h  
      else tft.setFreeFont(&sevenSeg16pt7b);

    // Cheap Yellow Display
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextSize(1);
      tft.setTextPadding( tft.textWidth("8") * (DROnoOfDigits1 + DROnoOfDigits2) );                // this clears the previous text

    char buff[50];

    // calipers
      for (int c=0; c < caliperCount; c++) {
        if (clearFirst) tft.drawString(" ", 0, c * tft.fontHeight());                             // clear display first if requested
        if (calipers[c].enabled) {
          float tReading = calipers[c].reading - calipers[c].adj[currentCoord] - gcodeDROadj[c];  // calculate current reading
          sprintf(buff, spa.c_str(), tReading);                                                   // format the reading for display

          // display the most recent reading if it is valid
            if (tReading >= lowestAllowedReading && tReading <= highestAllowedReading && calipers[c].lastReadTime != 0) {
              tft.drawString(buff, 0, c * tft.fontHeight());    
            } 

          // if show errors is set
            if (showDROerrors) {                                            // if a read error is flagged
              if (calipers[c].error != 0) {
                  String tErr = "Error" + String(calipers[c].error);
                  tft.drawString(tErr.c_str(), 0, c * tft.fontHeight());    // show error code            
              } else if (tReading >= lowestAllowedReading && tReading <= highestAllowedReading) {
                tft.drawString(buff, 0, c * tft.fontHeight());              // show the invalid reading
              }
              if (serialDebug) Serial.println("Invalid reading from " + calipers[c].title + " " + String(buff));          
            }
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
  
  if (gcodeText == "") return;              // if text is blank

  float t[caliperCount] = {};               // extracted position
  float pt[caliperCount] = {};              // previous position extracted
  for (int c=0; c < caliperCount; c++) pt[c] = highestAllowedReading * 10.0;     // set initial state to invalid reading

  // Split the G-code string into lines and process each line
    int startPos = 0;
    int endPos = 0;
    gcodeLineCount = 0;                       // global variable storing number of coordinates extracted from the gcode

    while (endPos != -1 && gcodeLineCount < maxGcodeStringLines) {
      endPos = gcodeText.indexOf('\n', startPos);       // find next line
      if (endPos != -1) {                               // if not end of text
        String gcodeLine = gcodeText.substring(startPos, endPos);    // extract the line
        processGCodeLine(gcodeLine, t);                 // pass the line of gcode which updates position of x,y,z based upon this line (99999999 = not found)
        startPos = endPos + 1;                          // move starting mosition to next line

        // process line if different to previous coordinate
          bool tFlag = 0;   
          for (int c=0; c < caliperCount; c++) { 
            if (inc[c] == 1) {                          // if this coordinate is active 
              if (pt[c] != t[c] && t[c] < highestAllowedReading && t[c] > lowestAllowedReading) {   // if reading is valid and different to the previous one
                // new reading 
                  pt[c] = t[c];
                  // log_system_message("new position for " + calipers[c].title + " = " + String(t[c]) );         // temp line for debugging
                  tFlag = 1;          
              }
            }
          }

          if (tFlag == 1) {        // add data to list of positions
            for (int c=0; c < caliperCount; c++) gcode[c][gcodeLineCount] = pt[c];
            gcodeLineCount ++;
          }
      }
    }   // while

  if (gcodeLineCount >= maxGcodeStringLines) log_system_message("Error: gcode exceded maximum line length");
  log_system_message(String(gcodeLineCount) + " coordinates extracted from gcode");
}


// ----------------------------------------------------------------
//                     -process a line of gcode
// ----------------------------------------------------------------
// This takes a String containing a line of basic gcode and updates positions -  It simply finds an 'x', 'y' or 'z' in the string and extracts the number following it

void processGCodeLine(String &gcodeLine, float Pos[]) {

  // extract values
  gcodeLine.toUpperCase();                                         // convert gcode line to upper case
  for (int c=0; c < caliperCount; c++) {      
    Pos[c] = 0.0;
    int PosIndex = gcodeLine.indexOf(calipers[c].title);           // search for axis title in the line          
    if (PosIndex != -1) {  
      // if axis was found
        float tRead = gcodeLine.substring(PosIndex + 1).toFloat(); // read the value 
        Pos[c] = gcodeLine.substring(PosIndex + 1).toFloat();      // convert the text to a float
    } else {
      // axis not found
        Pos[c] = highestAllowedReading * 10.0;                     // invalid reading
    }
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

  if (gcodeLineCount == 0) {
    client.println("<H2>STORED POSITIONS</H2>");
    client.println("<br>No stored positions to display<br><br>");
    webfooter(client);            // add the standard web page footer
    delay(1);
    client.stop();
    return;
  } 

  client.println("<H2>There are " + String(gcodeLineCount) + " stored positions</h2>");

  // deside which axis can be plotted (max of 2)
    int axis1=-1, axis2=-1;       // which axis will be plotted
    for (int c=0; c < caliperCount; c++) {
      if (inc[c] == 1) {
        if (axis1 == -1) axis1 = c;
        else if (axis2 == -1) {
          axis2 = c;
          break;   // exit loop as both axis now assigned
        }
      }
    }
    plotGraph(client, axis1, axis2);    // plot a graph of the stored positions 

  // display positions as a list
    // create html table
      const int cWidth = 280;                                   // table width
      client.println("<br><table style='text-align: centre; margin-left: auto; margin-right: auto; background-color: #D0D000; width: " + String(cWidth) + "px; table-layout: fixed;'>"); 
    for(int l=0; l < gcodeLineCount; l++) {                     // step through all stored positions                
        client.println("<tr><td>" + String(l + 1) + "</td>");   // position number
        for (int c=0; c < caliperCount; c++) {                  // step through all available axis
          if (inc[c] == 1) client.println("<td>" + calipers[c].title + String(String(gcode[c][l])) + "</td>");
        }
        client.println("</tr>");                                // end of row
    }
    client.println("</table>");                                 // end of html table

  // end html page
    webfooter(client);                  // add the standard web page footer
    delay(1);
    client.stop();
}


//
// plot a graph of the stored locations
//
void plotGraph(WiFiClient &client, int axis1, int axis2) {
    if (axis1 == -1) return;                   // no axis selected for plotting

    const bool plotAsText = 1;                 // If points on graph are plotted as text or dots
    int canvasWid = 400, canvasHei = 400;      // size of canvas (best to have them both the same or it distorts the scale)
    const int pixelSize = 5;                   // size of plotted points
    const int textSize = 5;                    // size of text
    if (axis2 == -1) canvasHei = 40;           // if only plotting one axis

    // find the min and max values
    float resMin = 999999, resMax = -999999;
    for (int l = 0; l < gcodeLineCount; l++) {
        if (axis1 != -1) {
            if (gcode[axis1][l] < resMin) resMin = gcode[axis1][l];
            if (gcode[axis1][l] > resMax) resMax = gcode[axis1][l];
        }
        if (axis2 != -1) {
            if (gcode[axis2][l] < resMin) resMin = gcode[axis2][l];
            if (gcode[axis2][l] > resMax) resMax = gcode[axis2][l];
        }
    }    

    // Axis assignment info.
      if (axis2 == -1) client.print(calipers[axis1].title);
      else client.print(calipers[axis1].title + " = Horizontal, " + calipers[axis2].title + " = Vertical");
      client.println("<br>");

    // create html canvas and show first axis
    client.println("<div><canvas id='graph' width='" + String (canvasWid + 20) + "' height='" + String (canvasHei) + "'></canvas></div>");
    client.printf(R"=====(
        <script>
            var canvas = document.getElementById("graph");
            var ctx = canvas.getContext("2d");
            ctx.fillStyle = "#FFFFFF";
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            ctx.fillStyle = "#0000FF";
            ctx.textAlign = "center";
            ctx.fillText(%.2f, 30, canvas.height - 17);
            ctx.fillText(%.2f, canvas.width - 20, canvas.height - 17);            
    )=====", resMin, resMax);

    // if plotting a second axis 
    if (axis2 != -1) client.printf(R"=====(
            ctx.fillText(%.2f, 30, 20);     
            ctx.fillStyle = "#000000"; 
    )=====", resMax);
/*
            // code to draw x and y axis lines
              ctx.beginPath();
                ctx.moveTo(30, 15);
                ctx.lineTo(30, canvas.height - 20);
                ctx.lineTo(canvas.width - 30, canvas.height - 20);
              ctx.stroke();
*/        

    // Plot positions on html canvas
    float tempX, tempY;
    client.println("ctx.fillStyle = \"#FF0000\";");         // set colour of position
    for (int l = 0; l < gcodeLineCount; l++) {
        tempX = 0; tempY = textSize;
        if (axis1 != -1) tempX = mapf(gcode[axis1][l], resMin, resMax, 30, canvasWid);
        if (axis2 != -1) tempY = mapf(gcode[axis2][l], resMin, resMax, canvasHei - 20, 10);
        if (plotAsText) {
          client.println("ctx.fillText(" + String(l + 1) + "," + String(tempX) + "," + String(tempY + textSize) + ");");  // number
        } else {
          client.println("ctx.fillRect(" + String(tempX) + "," + String(tempY + pixelSize) + "," + String(pixelSize) + "," + String(pixelSize) + ");");  // box
        }
    }
    client.println("</script>");

}


//
// Map function for float
//
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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
//           -load demonstartion gcode positions
// ----------------------------------------------------------------

void loadTestPositions() {

  if (caliperCount < 2) {
    log_system_message("Error: not enough axes available to use the demo data");
    return;
  }

  // set first 2 axes as active
    for (int c=0; c < caliperCount; c++) inc[c] = 0;        
    inc[0] = 1;  inc[1] = 1;

  // load demo data in to system (18 point circle)
    enteredGcode = R"=====(
      X30.00 Y0.00
      X28.19 Y10.26
      X22.98 Y19.28
      X15.00 Y25.98
      X5.21 Y29.54
      X-5.21 Y29.54
      X-15.00 Y25.98
      X-22.98 Y19.28
      X-28.19 Y10.26
      X-30.00 Y0.00
      X-28.19 Y-10.26
      X-22.98 Y-19.28
      X-15.00 Y-25.98
      X-5.21 Y-29.54
      X5.21 Y-29.54
      X15.00 Y-25.98
      X22.98 Y-19.28
      X28.19 Y-10.26
      X0.00 Y0.00   

    )=====";

  // process the data
    processGCode(enteredGcode); 

  log_system_message("Demo positions loaded in to memory");
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



// time reading of calipers
    unsigned long qt = micros();
    bool q = refreshCalipers(1,0);
    qt = micros() - qt;      
    if (q) client.print("<br>All calipers read ok, time taken to read them was " + String(qt) + " microseconds");
    else client.print("<br>Reading of calipers failed");
    client.println("<br>");


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
