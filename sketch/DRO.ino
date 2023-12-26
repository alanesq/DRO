/*******************************************************************************************************************
 *
 *                                                 SuperLowBudget-DRO
 *                                                 ------------------
 *
 *              DRO for milling machine/lathe etc. using a CheapYellowDisplay and cheap digital calipers
 *                                      See: https://github.com/alanesq/DRO
 *
 *
 *                     Tested in Arduino IDE 2.2.1 with board managers ESP32 2.0.14 & ESP8266 3.1.2
 *
 *                   Included files: standard.h, ota.h, sevenSeg.h, Free_Fonts.h, buttons.h & wifi.h
 *
 *
 *      Note:  To add ESP8266/32 ability to the Arduino IDE enter the below two lines in to FILE/PREFERENCES/BOARDS MANAGER
 *                                            http://arduino.esp8266.com/stable/package_esp8266com_index.json
 *                                            https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *             You can then add them in BOARDS MANAGER (search for esp8266 or ESP32)
 *
 *      First time the ESP starts it will create an access point "ESPDRO" which you need to connect to in order to enter your wifi details.
 *             default password = "password"   (change this in wifi.h)
 *             see: https://randomnerdtutorials.com/wifimanager-with-esp8266-autoconnect-custom-parameter-and-manage-your-ssid-and-password
 *
 *
 *            Distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
 *                   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 ********************************************************************************************************************

  3 Channel DRO using cheap digital calipers and a ESP32-2432S028R  (Cheap Yellow Display)
  see: https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display

  
  Libraries used:     https://github.com/PaulStoffregen/XPT2046_Touchscreen
                      https://github.com/Bodmer/TFT_eSPI
                      https://github.com/Bodmer/TFT_eWidget
                      https://github.com/tzapu/WiFiManager


    Ribbon cable (soldered to one side of the esp32 module, 3 colour LED removed)
    https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/PINS.md
    Note: If using Z then the serial port can not be used and make sure 'serialDebug' is set to 0 (Note: you can still update sketch via OTA)
      3.3v      (from nearby connector)
      gpio 4                  Caliper X clock
      gnd       (from nearby connector)
      gpio 0    (Not sure if this pin can be used)
      gpio 16                 Caliper X data
      gpio 17                 Caliper Y clock
      gpio 5                  Caliper Y data
      gpio 3    (Serial Rx)    
      gpio 1    (Serial Tx)   Caliper Z clock
      gpio 22                 Caliper Z data

    Other possible GPIO pins to use
      35 - On connector P3 - Input only
      22 - On connector CN1
      27 - On connector CN1
      04 - onboard LED
      16 - onboard LED
      17 - onboard LED
      05, 18, 19, 23 - SD card
      34 - Light sensor 
      00 - Onboard button


  Test Points
      S1	GND	  near USB-SERIAL
      S2	3.3v	for ESP32
      S3	5v	  near USB-SERIAL
      S4	GND	  for ESP32
      S5	3.3v	for TFT
      JP0 (pad nearest USB socket)	5v	TFT LDO
      JP0	3.3v	TFT LDO
      JP3 (pad nearest USB socket)	5v	ESP32 LDO
      JP3	3.3v	ESP32 LDO    


    Digital Caliper pinout: 
          GND   
          DATA
          CLOCK 
          VCC 1.5V  (I use a simple voltage divider of 100ohm resistors to supply 1.65v which should be ok)


    Colours available to use: 
          TFT_BLACK,TFT_NAVY,TFT_DARKGREEN,TFT_DARKCYAN,TFT_MAROON,TFT_PURPLE,TFT_OLIVE,TFT_LIGHTGREY,TFT_DARKGREY,TFT_BLACK,TFT_GREEN,TFT_CYAN,
          TFT_RED,TFT_MAGENTATFT_YELLOW,TFT_WHITE,TFT_ORANGE,TFT_GREENYELLOW,TFT_PINK,TFT_BROWN,TFT_GOLD,TFT_SILVER,TFT_SKYBLUE,TFT_VIOLET


    Wifi features:
      Simulate pressing the screen with: http://x.x.x.x/touch?x=100&y=50
      Restart the esp32 with: http://x.x.x.x/reboot
      Check if it is live with: http://x.x.x.x/ping
      Log of activity: http://x.x.x.x/log
      Testing code: see 'handleTest()' at bottom of page, access with http://x.x.x.x/test
      Update via OTA: http://x.x.x.x/ota      (default password is 'password')

*/

#if (!defined ESP32)
  #error This code is for the ESP32 based 'Cheap Yellow Display' only
#endif

// watchdog timer  
  #if (defined ESP32)  
    #include <esp_task_wdt.h>             // watchdog timer   - see: https://iotassistant.io/esp32/enable-hardware-watchdog-timer-esp32-arduino-ide/
  #endif
    
    
// ---------------------------------------------------------------

  #include <Arduino.h>                      // required by PlatformIO
  #include <WiFi.h>

  // forward declarations
    void log_system_message(String smes);   // in standard.h
    void displayMessage(String, String);    // in oled.h
    void handleRoot();
    void handleData();
    void handlePing();
    void settingsEeprom(bool eDirection);
    void handleTest();
    void drawScreen(int);      


// ---------------------------------------------------------------
//                          -SETTINGS
// ---------------------------------------------------------------   

  const char* stitle = "SuperLowBudget-DRO";             // title of this sketch

  const char* sversion = "26Dec23";                      // version of this sketch

  const bool wifiEnabled = 1;                            // if wifi to be used
  
  const bool serialDebug = 1;                            // provide debug info on serial port  (disable if using Tx or Rx gpio pins for caliper)
  const int serialSpeed = 115200;                        // Serial data speed to use  
  
  const bool invertCaliperDataSignals = 1;               // If using transistors on the data and clock pins the signals will be inverted so set this to 1
  
  const int checkDROreadings = 200;                      // how often to refresh DRO readings (ms)  
  

  // Display
    #define SCREEN_ROTATION 1
    #define SCREEN_WIDTH 320
    #define SCREEN_HEIGHT 240


  // Digital Caliper GPIO pins   ( -1 = not in use )
    #define CLOCK_PIN_X 16
    #define DATA_PIN_X  17
    
    #define CLOCK_PIN_Y 4
    #define DATA_PIN_Y  5
    
    #define CLOCK_PIN_Z -1
    #define DATA_PIN_Z  -1  


  // Touchscreen 
    #define TOUCH_LEFT 200         // positions on screen (may vary between screens?)
    #define TOUCH_RIGHT 3700
    #define TOUCH_TOP 200
    #define TOUCH_BOTTOM 3850
    #define XPT2046_IRQ 36
    #define XPT2046_MOSI 32
    #define XPT2046_MISO 39
    #define XPT2046_CLK 25
    #define XPT2046_CS 33        
    

  // Menu/screen settings
  
    // Page 1 - DRO screen size/position settings 
        const int DROdisplaySpacing = 10;                  // DRO reading vertical space between displays
        const int DROnoOfDigits = 6;                       // number of digits to display in caliper readings
        const int DROwidth = 200;                          // width of DRO reading display   i.e. x position of zero buttons  
        const int DROheight = 200;                         // Height of the DRO reading display for placement of lower buttons  
        const int DRObuttonheight = 30;                    // buttons around DRO readings              
        const int DRObuttonWidth = 35;                     
        const int DROdbuttonPlacement = 70;                // vertical spacing of the DRO buttons  

    //page 2


    // page 3 - numeric keypad
        const int keyWidth = 45;
        const int keyHeight = 35;
        const int keySpacing = 12;
        const int keyX = 155;                               // position on screen (top left)
        const int keyY = 60;
      
  #define WDT_TIMEOUT 60                                 // timeout of watchdog timer for esp32 (seconds) - i.e. auto restart if it stops responding

  #define ENABLE_EEPROM 0                                // if some settings are to be stored in eeprom  (not used in this sketch)

  #define ENABLE_OTA 1                                   // Enable Over The Air updates (OTA)
  const String OTAPassword = "password";                 // Password to enable OTA service (supplied as - http://<ip address>?pwd=xxxx )

  const char* HomeLink = "/";                            // Where home button on web pages links to (usually "/")
  const uint16_t ServerPort = 80;                        // ip port to serve web pages on (usually 80)

  int dataRefresh = 2;                                   // Refresh rate of the updating data on main web page (seconds)

  const byte LogNumber = 30;                             // number of entries in the system log

  const byte onboardLED = 25;                            // indicator LED pin?

  const byte onboardButton = 0;                          // onboard button gpio (FLASH)

  const bool ledBlinkEnabled = 1;                        // enable blinking status LED
  const uint16_t ledBlinkRate = 1500;                    // Speed to blink the status LED (milliseconds) - also perform some system tasks

  unsigned long wifiRetryTime = 30;                      // how often to try to reconnect to wifi when it is down (seconds)
  unsigned long wifiBackupRetryTime = 300;               // how often to try to reconnect to main wifi when using backup (seconds) - when using wifi2.h

  
  // -------------------------------- display ------------------------------------ 

  
    #include <SPI.h>
    #include <XPT2046_Touchscreen.h>

    SPIClass mySpi = SPIClass(HSPI);              // note: I had to change VSPI to HSPI
    XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);  

  // Cheap Yellow Display
    #include <TFT_eSPI.h>                         // display driver
    #include <TFT_eWidget.h>                      // Widget library
    #include "Free_Fonts.h"                       // Standard fonts info
    #include "sevenSeg.h"                         // seven segment style font
    TFT_eSPI tft = TFT_eSPI();
    
  MeterWidget   dro  = MeterWidget(&tft);         // demo meter used at startup

  bool displayRefreshX = 0;    // flag the caliper reading displays to refresh on the screen
  bool displayRefreshY = 0; 
  bool displayRefreshZ = 0; 

  // store for the current dro readings 
    float xReading, yReading, zReading;

  // zero adjustments for calipers (for the 3 coordinate systems)
    float xAdj[3] = {0};
    float yAdj[3] = {0};
    float zAdj[3] = {0};
    int currentCoord = 0;     // current one in use
  
  
  
  // ---------------------------- screen buttons ---------------------------------
  
  
    int displayingPage = 1;        // which screen page is being displayed
    typedef void (*buttonActionCallback)(void);   
    
    // struct to create a button
    struct buttonStruct {
      int page;                    // which screen page it belongs to (0 = show on all pages)
      bool enabled;                // if this button is enabled
      char* label;                 // buttons title
      int16_t x, y;                // location on screen (top left)
      uint16_t width, height;
      uint16_t outlineColour;
      uint16_t outlineThickness;
      uint16_t fillColour;
      uint16_t textColour;
      ButtonWidget* btn;            // button object (widget)
      buttonActionCallback action;  // procedure called when button pressed
    };
    
    // create the screen buttons

      // page 1
        ButtonWidget zeroX = ButtonWidget(&tft);  // zero
        ButtonWidget zeroY = ButtonWidget(&tft);  
        ButtonWidget zeroZ = ButtonWidget(&tft);  
        ButtonWidget zAll = ButtonWidget(&tft);  

        ButtonWidget halfX = ButtonWidget(&tft);  // half
        ButtonWidget halfY = ButtonWidget(&tft);  
        ButtonWidget halfZ = ButtonWidget(&tft);  

        ButtonWidget coord1 = ButtonWidget(&tft);  // coordinates
        ButtonWidget coord2 = ButtonWidget(&tft);  
        ButtonWidget coord3 = ButtonWidget(&tft);  

        ButtonWidget but4 = ButtonWidget(&tft);    // misc

      // page 2
        ButtonWidget but5 = ButtonWidget(&tft); 
        ButtonWidget but6 = ButtonWidget(&tft); 

      // page 3
        // number keypad
          String keyEnteredNumber = "";      // the number entered on the keypad
          ButtonWidget key1 = ButtonWidget(&tft); 
          ButtonWidget key2 = ButtonWidget(&tft); 
          ButtonWidget key3 = ButtonWidget(&tft); 
          ButtonWidget key4 = ButtonWidget(&tft); 
          ButtonWidget key5 = ButtonWidget(&tft); 
          ButtonWidget key6 = ButtonWidget(&tft); 
          ButtonWidget key7 = ButtonWidget(&tft); 
          ButtonWidget key8 = ButtonWidget(&tft); 
          ButtonWidget key9 = ButtonWidget(&tft); 
          ButtonWidget key0 = ButtonWidget(&tft); 
          ButtonWidget keyEnter = ButtonWidget(&tft); 


  // ------------------------- -define the button widgets -------------------------

    #include "buttons.h"             // the procedures which are triggered when a button is pressed

    const int buttonSpacing = 6;     // spacing used when placing the buttons on display
    
      buttonStruct screenButtons[] {               

      // Page 1
      // page, enabled, title, x, y, width, height, border colour, burder thickness, fill colour, pressed colour, button, procedure to run when button is pressed

        // Zero buttons
        {1, 1, "Z", DROwidth, 0, DRObuttonWidth, DRObuttonheight * 2 - buttonSpacing, TFT_WHITE, 1, TFT_DARKGREEN, TFT_BLACK, &zeroX, &zeroXpressed},
        {1, 1, "Z", DROwidth, DROdbuttonPlacement * 1, DRObuttonWidth,  DRObuttonheight * 2 - buttonSpacing, TFT_WHITE, 1, TFT_DARKGREEN, TFT_BLACK, &zeroY, &zeroYpressed},
        {1, 1, "Z", DROwidth, DROdbuttonPlacement * 2, DRObuttonWidth,  DRObuttonheight * 2 - buttonSpacing, TFT_WHITE, 1, TFT_DARKGREEN, TFT_BLACK, &zeroZ, &zeroZpressed},
        {1, 1, "Z", DROwidth, DROheight, DRObuttonWidth * 2 + buttonSpacing,  DRObuttonheight, TFT_WHITE, 1, TFT_DARKGREEN, TFT_BLACK, &zAll, &zAllpressed},

        // half buttons
        {1, 1, "1/2", DROwidth + DRObuttonWidth + buttonSpacing, 0, DRObuttonWidth,  DRObuttonheight * 2 - buttonSpacing, TFT_WHITE, 1, TFT_DARKCYAN, TFT_BLACK, &halfX, &halfXpressed},
        {1, 1, "1/2", DROwidth + DRObuttonWidth + buttonSpacing, DROdbuttonPlacement * 1, DRObuttonWidth,  DRObuttonheight * 2 - buttonSpacing, TFT_WHITE, 1, TFT_DARKCYAN, TFT_BLACK, &halfY, &halfYpressed},
        {1, 1, "1/2", DROwidth + DRObuttonWidth + buttonSpacing, DROdbuttonPlacement * 2, DRObuttonWidth,  DRObuttonheight * 2 - buttonSpacing, TFT_WHITE, 1, TFT_DARKCYAN, TFT_BLACK, &halfZ, &halfZpressed},
        
        // Coordinate select buttons
        {1, 1, "C1", 0, DROheight, DRObuttonWidth,  DRObuttonheight, TFT_WHITE, 1, TFT_MAROON, TFT_BLACK, &coord1, &coord1pressed},
        {1, 1, "C2", (DRObuttonWidth + buttonSpacing) * 1, DROheight, DRObuttonWidth,  DRObuttonheight, TFT_WHITE, 1, TFT_MAROON, TFT_BLACK, &coord2, &coord2pressed},
        {1, 1, "C3", (DRObuttonWidth + buttonSpacing) * 2, DROheight, DRObuttonWidth,  DRObuttonheight, TFT_WHITE, 1, TFT_MAROON, TFT_BLACK, &coord3, &coord3pressed},

        // Misc
        {1, 1, "P2", DROwidth + ( (DRObuttonWidth + buttonSpacing) * 2), 0, DRObuttonWidth, DRObuttonheight * 3 - buttonSpacing, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &but4, &button4Pressed},

      // page 2
        {2, 1, "Page1", 20, 140, 100, DRObuttonheight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &but5, &button5Pressed},

        // Misc
        {2, 1, "Reboot", 200, 140, 100, DRObuttonheight, TFT_WHITE, 1, TFT_RED, TFT_BLACK, &but6, &button6Pressed},

      // page 3 (keypad)
        {3, 1, "1", keyX + 0 * (keyWidth + keySpacing), keyY + 2 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &key1, &buttonKey1Pressed},
        {3, 1, "2", keyX + 1 * (keyWidth + keySpacing), keyY + 2 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &key2, &buttonKey2Pressed},
        {3, 1, "3", keyX + 2 * (keyWidth + keySpacing), keyY + 2 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &key3, &buttonKey3Pressed},
        {3, 1, "4", keyX + 0 * (keyWidth + keySpacing), keyY + 1 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &key4, &buttonKey4Pressed},
        {3, 1, "5", keyX + 1 * (keyWidth + keySpacing), keyY + 1 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &key5, &buttonKey5Pressed},
        {3, 1, "6", keyX + 2 * (keyWidth + keySpacing), keyY + 1 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &key6, &buttonKey6Pressed},
        {3, 1, "7", keyX + 0 * (keyWidth + keySpacing), keyY + 0 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &key7, &buttonKey7Pressed},
        {3, 1, "8", keyX + 1 * (keyWidth + keySpacing), keyY + 0 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &key8, &buttonKey8Pressed},
        {3, 1, "9", keyX + 2 * (keyWidth + keySpacing), keyY + 0 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &key9, &buttonKey9Pressed},
        {3, 1, "0", keyX + 0 * (keyWidth + keySpacing), keyY + 3 * (keyHeight + keySpacing), keyWidth, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &key0, &buttonKey0Pressed},
        {3, 1, "ENTER", keyX + 1 * (keyWidth + keySpacing), keyY + 3 * (keyHeight + keySpacing), keyWidth * 2 + keySpacing, keyHeight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &keyEnter, &buttonKeyEnterPressed},

    };
    uint8_t buttonCount = sizeof(screenButtons) / sizeof(*screenButtons);     // number of buttons created

    
  // -----------------------------------------------------------------------------
    
    
// caliper data
  int sign;         // if the reading is negative
  int inches;       // flag if data is in inches (0 = mm)
  long value;       // reading from caliper

int _TEMPVARIABLE_ = 1;                 // Temporary variable used in demo radio buttons on root web page

bool OTAEnabled = 0;                    // flag to show if OTA has been enabled (via supply of password in http://x.x.x.x/ota)
int wifiok = 0;                         // flag to show if wifi connection is ok 
unsigned long wifiDownTime = 0;         // if wifi connection is lost this records at what time it was lost

#include "wifi.h"                       // Load the Wifi / NTP stuff
#include "standard.h"                   // Some standard procedures

Led statusLed1(onboardLED, LOW);        // set up onboard LED - see standard.h
Button button1(onboardButton, HIGH);    // set up the onboard 'flash' button - see standard.h

#if ENABLE_EEPROM
  #include <EEPROM.h>                   // for storing settings in eeprom (not used at present)
#endif

#if ENABLE_OTA
  #include "ota.h"                      // Over The Air updates (OTA)
#endif


// ---------------------------------------------------------------
//    -SETUP     SETUP     SETUP     SETUP     SETUP     SETUP
// ---------------------------------------------------------------
//
// setup section (runs once at startup)

void setup() {

  if (serialDebug) {       // if serial debug information is enabled
    Serial.begin(serialSpeed); while (!Serial); delay(50);       // start serial comms
    //delay(2000);      // gives platformio chance to start serial monitor

    Serial.println("\n\n\n");                                    // some line feeds
    Serial.println("-----------------------------------");
    Serial.printf("Starting - %s - %s \n", stitle, sversion);
    Serial.println("-----------------------------------");
    Serial.println( "Device type: " + String(ARDUINO_BOARD) + ", chip id: " + String(ESP_getChipId(), HEX));
    // Serial.println(ESP.getFreeSketchSpace());
    #if defined(ESP8266)
        Serial.println("Chip ID: " + ESP.getChipId());
        rst_info *rinfo = ESP.getResetInfoPtr();
        Serial.println("ResetInfo: " + String((*rinfo).reason) + ": " + ESP.getResetReason());
        Serial.printf("Flash chip size: %d (bytes)\n", ESP.getFlashChipRealSize());
        Serial.printf("Flash chip frequency: %d (Hz)\n", ESP.getFlashChipSpeed());
        Serial.printf("\n");
    #endif
  }

  // Cheap Yellow Display
  tft.init();
  tft.setRotation(SCREEN_ROTATION);           // set display to landscape
  tft.fillScreen(TFT_BLACK);    // Clear the screen
  tft.setFreeFont(FM9);        // standard Free Mono font - available sizes: 9, 12, 18 or 24
  tft.setTextSize(1);           // 1 or 2
  const int sSpacing = 22;      // line spacing
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString(String(stitle), 5, 0);   
  tft.drawString(String(sversion), 220, sSpacing);  
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.drawString("See: github.com/alanesq/DRO", 5, sSpacing * 2); 

  // display a meter    see: https://github.com/Bodmer/TFT_eWidget/blob/main/examples/Meters/Analogue_meters/Analogue_meters.ino
    dro.setZones(75, 100, 50, 75, 25, 50, 0, 25);   
    dro.analogMeter(40, 95, 100.0, "STARTUP", "0", "25", "50", "75", "100"); 

  // reserve memory for the log to reduce stack fragmentation (yes, I use Strings - deal with it ;-)
    for (int i=0; i < LogNumber; i++) { 
      system_message[i].reserve(maxLogEntryLength);
    }

  // if (serialDebug) Serial.setDebugOutput(true);         // to enable extra diagnostic info

  statusLed1.on();                                         // turn status led on until wifi has connected (see standard.h)

  #if ENABLE_EEPROM
    settingsEeprom(0);       // read stored settings from eeprom
  #endif


  // wifi stuff

    if (wifiEnabled) {
      // update screen
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("Starting wifi ...", 5, sSpacing * 3);   
        dro.updateNeedle(25, 20);                                // move dial to 25

      startWifiManager();                                        // Connect to wifi using wifi manager (see wifi.h)
      //startWifi();                                             // Connect to wifi using fixed details (see wifi.h)

      WiFi.mode(WIFI_STA);     // turn off access point - options are WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF

    //  // configure as access point that other esp units can use if main wifi is down
    //    WiFi.mode(WIFI_STA);                                   // turn off access point in case wifimanager still has one active
    //    if (serialDebug) Serial.print("Starting access point: ");
    //    if (serialDebug) Serial.print(APSSID);
    //    WiFi.softAP(APSSID, APPWD, 1, 1, 8);                   // access point settings
    //    delay(150);
    //    if ( WiFi.softAPConfig(Ip, Ip, NMask) ) {
    //      WiFi.mode(WIFI_AP_STA);                              // enable as both Station and access point - options are WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF
    //      log_system_message("Access Point wifi started: " + WiFi.softAPIP().toString());
    //    } else {
    //      log_system_message("Error: unable to start Access Point wifi");
    //    }  

      // set up web page request handling
        server.on(HomeLink, handleRoot);         // root page
        server.on("/data", handleData);          // supplies information to update root page via Ajax)
        server.on("/ping", handlePing);          // ping requested
        server.on("/log", handleLogpage);        // system log (in standard.h)
        server.on("/test", handleTest);          // testing page
        server.on("/reboot", handleReboot);      // reboot the esp
        server.on("/touch", handleTouch);        // for simulating a press on the touchscreen
        //server.onNotFound(handleNotFound);       // invalid page requested
        #if ENABLE_OTA
          server.on("/ota", handleOTA);          // ota updates web page
        #endif

      // start web server
        if (serialDebug) Serial.println("Starting web server");  
      // update screen   
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("Starting web server ...", 5, sSpacing * 3);      // Left Aligned  
        dro.updateNeedle(50, 20);                                        // move dial to 50

      server.begin();

      // Stop wifi going to sleep (if enabled it can cause wifi to drop out randomly especially on esp8266 boards)
          WiFi.setSleep(false);
   
      // Finished connecting to network
        statusLed1.off();                                              // turn status led off
        log_system_message("Started, ip=" + WiFi.localIP().toString());
        // update screen
          tft.setTextColor(TFT_WHITE, TFT_BLACK);        
          tft.drawString("IP = " + WiFi.localIP().toString() + "      ", 5, sSpacing * 3);    
          dro.updateNeedle(75, 20);  
    }    // wifi stuff

    #if (defined ESP32)  
      // watchdog timer (esp32)
        if (serialDebug) Serial.println("Configuring watchdog timer");
        esp_task_wdt_init(WDT_TIMEOUT, true);                      //enable panic so ESP32 restarts
        esp_task_wdt_add(NULL);                                    //add current thread to WDT watch    
    #endif    

  // Start the SPI for the touch screen and init the TS library
    mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(mySpi);
    ts.setRotation(SCREEN_ROTATION);  

  // caliper gpio pins
    pinMode(CLOCK_PIN_X, INPUT);
    pinMode(DATA_PIN_X, INPUT);
    pinMode(CLOCK_PIN_Y, INPUT);
    pinMode(DATA_PIN_Y, INPUT);
    pinMode(CLOCK_PIN_Z, INPUT);
    pinMode(DATA_PIN_Z, INPUT);    

  // initialise buttons
    for (uint8_t b = 0; b < buttonCount; b++) {
      screenButtons[b].btn->initButtonUL(screenButtons[b].x, screenButtons[b].y, screenButtons[b].width, screenButtons[b].height, screenButtons[b].outlineColour, screenButtons[b].fillColour, screenButtons[b].textColour, screenButtons[b].label, 1);
      screenButtons[b].btn->setPressAction(screenButtons[b].action);
      //screenButtons[b].btn->setReleasAction(xxxx);
    }
  
  // update screen    
    dro.updateNeedle(100, 20);                                 // move dial to 100

  if (wifiEnabled) delay(2000);                                // give time to see ip address
  drawScreen(1);     // draw the screen
}


// ----------------------------------------------------------------
//   -LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP
// ----------------------------------------------------------------

void loop(){

  if(wifiEnabled) server.handleClient();                // service any web page requests

  // Touch screen
    static repeatTimer touchTimer;                      // set up a repeat timer 
    if (touchTimer.check(100)) {                        // repeat at set interval (ms)  
      if (ts.tirqTouched() && ts.touched()) {
        TS_Point p = ts.getPoint();
        actionScreenTouch(p);
      }
    }

  // Digital Caliper - X                                     // Note: I have kept the three caliper codes seperate here in case any require custom code
    if (DATA_PIN_X != -1) {                                  // if caliper is present 
      static repeatTimer caliperTimerX;                      // set up a repeat timer  (see standard.h)
      if (caliperTimerX.check(checkDROreadings)) {           // repeat at set interval (ms)
        float tRead = 1.0;  
        if (!waitPinState(CLOCK_PIN_X, HIGH)) tRead = 8888.88;   // wait for gpio pin to change (8888.88 signifies failed)
        unsigned long tmpTime=micros();
        if (!waitPinState(CLOCK_PIN_X, LOW)) tRead = 8888.88;
        if((micros()-tmpTime) > 500) {
          if (tRead == 1.0) tRead = readCaliper(CLOCK_PIN_X, DATA_PIN_X);  // read data from caliper 
          if (tRead != xReading || displayRefreshX == 1) {     // if reading has changed or flag to refresh is set update display
            xReading = tRead;                                  // store result in global variable
            displayReadings();                                 // display caliper readings 
            displayRefreshX = 0;
          }
        }
      }
    } 

  // Digital Caliper - Y      
    if (DATA_PIN_Y != -1) {                                  // if caliper is present 
      static repeatTimer caliperTimerY;                      // set up a repeat timer 
      if (caliperTimerY.check(checkDROreadings)) {           // repeat at set interval (ms)
        float tRead = 1.0;  
        if (!waitPinState(CLOCK_PIN_Y, HIGH)) tRead = 8888.88;   // wait for gpio pin to change (8888.88 signifies failed)
        unsigned long tmpTime=micros();
        if (!waitPinState(CLOCK_PIN_Y, LOW)) tRead = 8888.88;
        if((micros()-tmpTime) > 500) {
          if (tRead == 1.0) tRead = readCaliper(CLOCK_PIN_Y, DATA_PIN_Y);  // read data from caliper 
          if (tRead != yReading || displayRefreshY == 1) {     // if reading has changed or flag to refresh is set update display
            xReading = tRead;                                  // store result in global variable
            displayReadings();                                 // display caliper readings 
            displayRefreshY = 0;
          }
        }
      }
    } 

  // Digital Caliper - Z 
    if (DATA_PIN_Z != -1) {                                  // if caliper is present 
      static repeatTimer caliperTimerZ;                      // set up a repeat timer 
      if (caliperTimerZ.check(checkDROreadings)) {           // repeat at set interval (ms)
        float tRead = 1.0;  
        if (!waitPinState(CLOCK_PIN_Z, HIGH)) tRead = 8888.88;   // wait for gpio pin to change (8888.88 signifies failed)
        unsigned long tmpTime=micros();
        if (!waitPinState(CLOCK_PIN_Z, LOW)) tRead = 8888.88;
        if((micros()-tmpTime) > 500) {
          if (tRead == 1.0) tRead = readCaliper(CLOCK_PIN_Z, DATA_PIN_Z);  // read data from caliper 
          if (tRead != zReading || displayRefreshZ == 1) {     // if reading has changed or flag to refresh is set update display
            xReading = tRead;                                  // store result in global variable
            displayReadings();                                 // display caliper readings 
            displayRefreshZ = 0;
          }
        }
      }
    }     

    // Periodically change the LED status to indicate all is well
      static repeatTimer ledTimer;                          // set up a repeat timer (see standard.h)
      if (ledTimer.check(ledBlinkRate)) {                   // repeat at set interval (ms)
          #if (defined ESP32)  
            esp_task_wdt_reset();                           // reset watchdog timer 
          #endif
          bool allOK = 1;                                   // if all checks leave this as 1 then the LED is flashed
          if (!WIFIcheck()) allOK = 0;                      // if wifi connection is not ok
          if (timeStatus() != timeSet) allOK = 0;           // if NTP time is not updating ok
          time_t t=now();                                   // read current time to ensure NTP auto refresh keeps triggering (otherwise only triggers when time is required causing a delay in response)
          if (t == 0) t=0;                                  // pointless line to stop PlatformIO getting upset that the variable is not used
          // blink status led
            if (ledBlinkEnabled && allOK) {
              statusLed1.flip(); // invert the LED status if all OK (see standard.h)
            } else {
              statusLed1.off();
            }
      }
}


// ----------------------------------------------------------------
//       -root web page requested    i.e. http://x.x.x.x/
// ----------------------------------------------------------------

void handleRoot() {

  int rNoLines = 4;      // number of updating information lines to be populated via http://x.x.x.x/data

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

// insert empty lines which are later populated via vbscript with live data from http://x.x.x.x/data in the form of comma separated text
  for (int i = 0; i < rNoLines; i++) {
    client.println("<span id='uline" + String(i) + "'></span><br>");
  }

// Javascript - to periodically update the above info lines from http://x.x.x.x/data
// This is the below javascript code compacted to save flash memory via https://www.textfixer.com/html/compress-html-compression.php
   client.printf(R"=====(  <script> function getData() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { var receivedArr = this.responseText.split(','); for (let i = 0; i < receivedArr.length; i++) { document.getElementById('uline' + i).innerHTML = receivedArr[i]; } } }; xhttp.open('GET', 'data', true); xhttp.send();} getData(); setInterval(function() { getData(); }, %d); </script> )=====", dataRefresh * 1000);
/*
  // get a comma seperated list from http://x.x.x.x/data and populate the blank lines in html above
  client.printf(R"=====(
     <script>
        function getData() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            var receivedArr = this.responseText.split(',');
            for (let i = 0; i < receivedArr.length; i++) {
              document.getElementById('uline' + i).innerHTML = receivedArr[i];
            }
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


    // // demo enter a value
    //   client.println("<br><br>Enter a value: <input type='number' style='width: 40px' name='value1' title='additional info which pops up'> ");
    //   client.println("<input type='submit' name='submit'>");

    // // demo radio buttons - "RADIO1"
    //   client.print(R"=====(
    //     <br><br>Demo radio buttons
    //     <br>Radio1 button1<INPUT type='radio' name='RADIO1' value='1'>
    //     <br>Radio1 button2<INPUT type='radio' name='RADIO1' value='2'>
    //     <br>Radio1 button3<INPUT type='radio' name='RADIO1' value='3'>
    //     <br><INPUT type='reset'> <INPUT type='submit' value='Action'>
    //   )=====");

    // // demo standard button
    // //    'name' is what is tested for above to detect when button is pressed, 'value' is the text displayed on the button
    //   client.println("<br><br><input style='");
    //   // if ( x == 1 ) client.println("background-color:red; ");      // to change button color depending on state
    //   client.println("height: 30px;' name='demobutton' value='Demonstration Button' type='submit'>\n");


    // demo of how to display an updating image   e.g. when using esp32cam
    /*
      client.write("<br><a href='/jpg'>");                                              // make it a link
      client.write("<img id='image1' src='/jpg' width='320' height='240' /> </a>");     // show image from http://x.x.x.x/jpg
      // javascript to refresh the image periodically
        int imagerefresh = 2;                                                           // how often to update (seconds)
        client.printf(R"=====(
           <script>
             function refreshImage(){
                 var timestamp = new Date().getTime();
                 var el = document.getElementById('image1');
                 var queryString = '?t=' + timestamp;
                 el.src = '/jpg' + queryString;
             }
             setInterval(function() { refreshImage(); }, %d);
           </script>
        )=====", imagerefresh * 1000);
    */

    // close page
      client.println("</P>");                                // end of section
      client.println("</form>\n");                           // end form section (used by buttons etc.)
      webfooter(client);                                     // html page footer
      delay(3);
      client.stop();
}


// ----------------------------------------------------------------
//            -Action any user input on root web page
// ----------------------------------------------------------------

void rootUserInput(WiFiClient &client) {

  // // if value1 was entered
  //   if (server.hasArg("value1")) {
  //   String Tvalue = server.arg("value1");   // read value
  //   if (Tvalue != NULL) {
  //     int val = Tvalue.toInt();
  //     log_system_message("Value entered on root page = " + Tvalue );
  //   }
  // }

  // // if radio button "RADIO1" was selected
  //   if (server.hasArg("RADIO1")) {
  //     String RADIOvalue = server.arg("RADIO1");   // read value of the "RADIO" argument
  //     //if radio button 1 selected
  //     if (RADIOvalue == "1") {
  //       log_system_message("radio button 1 selected");
  //       _TEMPVARIABLE_ = 1;
  //     }
  //     //if radio button 2 selected
  //     if (RADIOvalue == "2") {
  //       log_system_message("radio button 2 selected");
  //       _TEMPVARIABLE_ = 2;
  //     }
  //     //if radio button 3 selected
  //     if (RADIOvalue == "3") {
  //       log_system_message("radio button 3 selected");
  //       _TEMPVARIABLE_ = 3;
  //     }
  //   }

  //   // if button "demobutton" was pressed
  //     if (server.hasArg("demobutton")) {
  //         log_system_message("demo button was pressed");
  //     }
}


// ----------------------------------------------------------------
//     -data web page requested     i.e. http://x.x.x.x/data
// ----------------------------------------------------------------
// the root web page requests this periodically via Javascript in order to display updating information.
// information in the form of comma seperated text is supplied which are then inserted in to blank lines on the web page
// Note: to change the number of lines displayed update variable 'rNoLines' at the top of handleroot()

void handleData(){

   String reply; reply.reserve(500); reply = "";                   // reserve space to cut down on fragmentation
   char buff[100]; 

   // line1 - current time
     reply += "<br>Current time: " + currentTime();
     reply += "<br>,";        // end of line

   // line2 - DRO readings
     String spa = "%0" + String(DROnoOfDigits + 1) + ".2f";             // create sprint argument for number format (in the format "%07.2f")
     String spa2 = "Absolute=" + spa + " : 1=" + spa + " : 2=" + spa + " : 3=" + spa + "<br>";  // for the full line

     if (DATA_PIN_X != -1) {
      sprintf(buff, spa2.c_str(), xReading, xReading - xAdj[0], xReading - xAdj[1], xReading - xAdj[2]);
      reply += "X: " + String(buff);
     }
     if (DATA_PIN_Y != -1) {
      sprintf(buff, spa2.c_str(), yReading, yReading - xAdj[0], yReading - xAdj[1], yReading - xAdj[2]);
      reply += "Y: " + String(buff);
     }
     if (DATA_PIN_Z != -1) {
      sprintf(buff, spa2.c_str(), zReading, zReading - xAdj[0], zReading - xAdj[1], zReading - xAdj[2]);
      reply += "Z: " + String(buff);      
     }
     reply += "Current selected coordinate system: " + String(currentCoord + 1);
     reply += "<br>,";        // end of line

   // line3 -  
     reply += ",";        // end of line

   // line4 -  
     


   server.send(200, "text/plane", reply); //Send millis value only to client ajax request
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

  String message = "ok";
  server.send(200, "text/plain", message);   // send reply as plain text
}


// ----------------------------------------------------------------
//                    -settings stored in eeprom
// ----------------------------------------------------------------
// @param   eDirection   1=write, 0=read
// Note on esp32 this has now been replaced by preferences.h although I still prefer this method - see: by https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/

#if ENABLE_EEPROM
void settingsEeprom(bool eDirection) {

    const int dataRequired = 30;   // total size of eeprom space required (bytes)

    uint32_t demoInt = 42;         // 4 byte variable to be stored in eeprom as an example of how (can be removed)

    int currentEPos = 0;           // current position in eeprom

    EEPROM.begin(dataRequired);

    if (eDirection == 0) {

      // read settings from Eeprom
        EEPROM.get(currentEPos, demoInt);             // read data
        if (demoInt < 1 || demoInt > 99) demoInt = 1; // validate
        currentEPos += sizeof(demoInt);               // increment to next free position in eeprom

    } else {

      // write settings to Eeprom
        EEPROM.put(currentEPos, demoInt);             // write demoInt to Eeprom
        currentEPos += sizeof(demoInt);               // increment to next free position in eeprom

      EEPROM.commit();                              // write the data out (required on esp devices as they simulate eeprom)
    }

}
#endif


// ----------------------------------------------------------------
//                -Touch screen has been pressed
// ----------------------------------------------------------------

void actionScreenTouch(TS_Point p) {

  const int lineHight = 16;
  const bool showPress = 0;     // show data on screen

  // calculate position on screen
    int x = map(p.x, TOUCH_LEFT, TOUCH_RIGHT, 0, SCREEN_WIDTH);  
    int y = map(p.y, TOUCH_TOP, TOUCH_BOTTOM, 0, SCREEN_HEIGHT);  
    if (serialDebug) Serial.println("Touch data: " + String(p.x) + "," + String(p.y) + "," + String(x) + "," + String(y));    

  // check if a button has been pressed
    for (uint8_t b = 0; b < buttonCount; b++) {
      if (screenButtons[b].enabled == 1 && ( screenButtons[b].page == displayingPage || screenButtons[b].page == 0) && screenButtons[b].btn->contains(x, y)) {
        // button pressed
          tft.setFreeFont(FM9);         // standard Free Mono font - available sizes: 9, 12, 18 or 24
          tft.setTextSize(2);        
          screenButtons[b].btn->drawSmoothButton(true);
          if (serialDebug) Serial.println("button " + String(screenButtons[b].label) + " pressed!");
          screenButtons[b].btn->press(true);
          screenButtons[b].btn->pressAction();          
          //screenButtons[b].action();   // call the buttons procedure directly
      } else {
        // button is not pressed
          screenButtons[b].btn->press(false);  // flag button released
          if ( (screenButtons[b].page == displayingPage|| screenButtons[b].page == 0) && screenButtons[b].enabled == 1 ) { 
            tft.setFreeFont(FM9);         // standard Free Mono font - available sizes: 9, 12, 18 or 24
            tft.setTextSize(2);                    
            screenButtons[b].btn->drawSmoothButton(false);      // draw button
          }
      }
    }      

  // draw data on screen
    if (showPress) {
      //tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextSize(1);
      String temp; y -= lineHight;
      //temp = "Pressure = " + String(p.z);       // show pressure data
      //tft.drawCentreString(temp, x, y, 1);
      //y += lineHight;
      temp = "X = " + String(p.x);
      tft.drawCentreString(temp, x, y, 1);
      y += lineHight;
      temp = "Y = " + String(p.y);
      tft.drawCentreString(temp, x, y, 1);
    }

  // send data to serial port
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
//                     -draw the screen
// ----------------------------------------------------------------
// screen = page number to show

void drawScreen(int screen) {

  if (serialDebug) Serial.println("Displaying page " + String(screen));
  displayingPage = screen;      // set current page

  tft.fillScreen(TFT_BLACK);    // clear screen
  
  // NOTE: If this font is changed it also needs to be changed in 'actionScreenTouch()'
    tft.setFreeFont(FM9);         // standard Free Mono font - available sizes: 9, 12, 18 or 24
    tft.setTextSize(2);

  // draw the buttons
    for (uint8_t b = 0; b < buttonCount; b++) {
      if (screenButtons[b].enabled == 1) {   
         if( screenButtons[b].page == screen || screenButtons[b].page == 0) {       // 0 = show on all pages
            screenButtons[b].btn->drawSmoothButton(false, screenButtons[b].outlineThickness, TFT_BLACK);      
         }
      }
    } 

  displayRefreshX = 1;    // flag the caliper reading displays to refresh 
  displayRefreshY = 1;
  displayRefreshZ = 1;

}


// ----------------------------------------------------------------
//              -wait for gpio pin state to change  
// ----------------------------------------------------------------
// returns 0 if times out

bool waitPinState(int pin, bool state) {
  int timeout = 3000000;         // timeout
  while(digitalRead(pin) == state && timeout > 0) {
    timeout--;
  }
  if (timeout < 1) return 0;
  else return 1;
}


//      ***************************  Calipers  *****************************


// ----------------------------------------------------------------
//                       -read caliper data
// ----------------------------------------------------------------
// Note - Using transistors inverts the signal levels          returns 8888.88 if failed to get data
// see: http://wei48221.blogspot.com/2016/01/using-digital-caliper-for-digital-read_21.html

float readCaliper(int clockPin, int dataPin) {

  // data levels on data and clock pins 
    const bool pin0 = invertCaliperDataSignals;
    const bool pin1 = !invertCaliperDataSignals;

  int sign = 1;         // if the reading is negative
  int inches = 0;       // flag if data is in inches (0 = mm) -  NOTE: this doesn't seem to work (always 1)?
  long value = 0;       // reading from caliper  
  for(int i=0;i<24;i++) {                                 // read the data bits 
    if (!waitPinState(clockPin, pin0)) return 8888.88;       // wait for gpio pin state to change
    if (!waitPinState(clockPin, pin1)) return 8888.88;
    if(digitalRead(dataPin) == pin1) {
      // 1 received
        if(i<20) value|=(1<<i);
        if(i==20) sign=-1;
        if(i==23) inches=1;    
    } 
  }
  return (value*sign)/100.0;
}

// ----------------------------------------------------------------
//                   -display caliper readings
// ----------------------------------------------------------------

void displayReadings() {

    // create sprint argument  (in the format "%07.2f")
      String spa = "%0" + String(DROnoOfDigits + 1) + ".2f";    // https://alvinalexander.com/programming/printf-format-cheat-sheet/
   
    // font size
      if (displayingPage == 1) tft.setFreeFont(&sevenSeg35pt7b);         // seven segment style font from sevenSeg.h  
      else tft.setFreeFont(&sevenSeg16pt7b);

    // Cheap Yellow Display
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextSize(1);
      //tft.setTextPadding(DROwidth);           // this clears the previous text (it should anyway)
      tft.setTextPadding( tft.textWidth("8") * DROnoOfDigits );           // this clears the previous text

      char buff[30];

    // x
      if (DATA_PIN_X != -1) {
        sprintf(buff, spa.c_str(), xReading - xAdj[currentCoord]);
        if (strlen(buff) == DROnoOfDigits+1) tft.drawString(buff, 0, 0);    
        else if (serialDebug) Serial.println("Invalid reading from X: " + String(buff));
      }

    // y
      if (DATA_PIN_Y != -1) {
        sprintf(buff, spa.c_str(), yReading - yAdj[currentCoord]);
        if (strlen(buff) == DROnoOfDigits+1) tft.drawString(buff, 0, tft.fontHeight() );    
        else if (serialDebug) Serial.println("Invalid reading from Y: " + String(buff));
      }

    // z
      if (DATA_PIN_Z != -1) {
        sprintf(buff, spa.c_str(), zReading - zAdj[currentCoord]);
        if (strlen(buff) == DROnoOfDigits+1) tft.drawString(buff, 0, tft.fontHeight() * 2 );    
        else if (serialDebug) Serial.println("Invalid reading from Z: " + String(buff));
      }   
   
    tft.setFreeFont(FM12);        // switch back to standard Free Mono font - available sizes: 9, 12, 18 or 24   
    tft.setTextPadding(0);        // clear the padding setting
      
    // Show current coordinate system (if showing page 1)
      if (displayingPage == 1) {
        sprintf(buff, "C%d", currentCoord + 1);
        tft.setFreeFont(FM12);         // standard Free Mono font - available sizes: 9, 12, 18 or 24
        tft.setTextSize(1);            // 1 or 2  
        tft.setTextColor(TFT_MAROON, TFT_BLACK);
        tft.drawString(buff, ( (DRObuttonWidth + buttonSpacing) * 3), DROheight + 5);   
      }

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
//           -testing page     i.e. http://x.x.x.x/test
// ----------------------------------------------------------------
// Use this area for experimenting/testing code

void handleTest(){

  WiFiClient client = server.client();          // open link with client

  // log page request including clients IP address
    IPAddress cip = client.remoteIP();
    String clientIP = decodeIP(cip.toString());   // check for known IP addresses
    log_system_message("Test page requested from: " + clientIP);

  webheader(client);                 // add the standard html header
  client.println("<br><H2>TEST PAGE</H2><br>");

  


  // ---------------------------- test section here ------------------------------


// // test x zero
//   xAdj[currentCoord] = xReading;


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
