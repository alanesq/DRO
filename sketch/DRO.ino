/*******************************************************************************************************************
 *
 *                        DRO for milling machine using a CheapYellowDisplay and cheap digital calipers
 *
 *                 Tested in Arduino IDE 2.2.1 with board managers ESP32 2.0.14 & ESP8266 3.1.2
 *
 *                   Included files: email.h, standard.h, ota.h, oled.h, neopixel.h & wifi.h
 *
 *
 *
 *      Note:  To add ESP8266/32 ability to the Arduino IDE enter the below two lines in to FILE/PREFERENCES/BOARDS MANAGER
 *                                            http://arduino.esp8266.com/stable/package_esp8266com_index.json
 *                                            https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *             You can then add them in BOARDS MANAGER (search for esp8266 or ESP32)
 *
 *      First time the ESP starts it will create an access point "ESPPortal" which you need to connect to in order to enter your wifi details.
 *             default password = "12345678"   (change this in wifi.h)
 *             see: https://randomnerdtutorials.com/wifimanager-with-esp8266-autoconnect-custom-parameter-and-manage-your-ssid-and-password
 *
 *      Much of the sketch is the code from other people's work which I have combined together, I believe I have links
 *      to all the sources but let me know if I have missed anyone.
 *
 *                                                                                      Created by: https://alanesq.github.io/
 *
 *        Distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
 *        implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 ********************************************************************************************************************

  2 Channel DRO using cheap digital calipers and a ESP32-2432S028R  (Cheap Yellow Display)

                                                                                    17Dec23
  
  Libraries used:     https://github.com/PaulStoffregen/XPT2046_Touchscreen
                      https://github.com/Bodmer/TFT_eSPI
                      https://github.com/Bodmer/TFT_eWidget
                      SPI
                      WiFiManager - see:  https://github.com/tzapu/WiFiManager


  Ref:  https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/


  GPIO Pins - https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/PINS.md

    Ribbon cable (soldered down one side of the esp32 module, LED removed)
    Note: If using Z then the serial port can not be used whilst caliper is connected and make sure 'serialDebug' is set to 0 (you can still update sketch via OTA)
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


      35 - On connector P3 - Input only - 
      22 - On connector CN1 -
      27 - On connector CN1 -
      04 - onboard LED -
      16 - onboard LED -
      17 - onboard LED -
      05, 18, 19, 23 - SD card -
      34 - Light sensor - 
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
          DAT   
          CLK  
          VCC 1.5V  


    Colours available  
          TFT_BLACK,TFT_NAVY,TFT_DARKGREEN,TFT_DARKCYAN,TFT_MAROON,TFT_PURPLE,TFT_OLIVE,TFT_LIGHTGREY,TFT_DARKGREY,TFT_BLACK,TFT_GREEN,TFT_CYAN,
          TFT_RED,TFT_MAGENTATFT_YELLOW,TFT_WHITE,TFT_ORANGE,TFT_GREENYELLOW,TFT_PINK,TFT_BROWN,TFT_GOLD,TFT_SILVER,TFT_SKYBLUE,TFT_VIOLET
*/

#if (!defined ESP32)
  #error This code is for ESP32 only
#endif

// watchdog timer
  #if (defined ESP32)  
    #include <esp_task_wdt.h>             // watchdog timer   - see: https://iotassistant.io/esp32/enable-hardware-watchdog-timer-esp32-arduino-ide/
  #endif
    
    
// ---------------------------------------------------------------


// Required by PlatformIO

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


// ---------------------------------------------------------------
//                          -SETTINGS
// ---------------------------------------------------------------
// ESP32 gpio pins: https://linuxhint.com/esp32-pinout-reference/
    

  const char* stitle = "SuperLowBudget-DRO";             // title of this sketch

  const char* sversion = "24Dec23";                      // version of this sketch

  const bool wifiEnabled = 1;                            // if wifi to be used

  // Display
    #define SCREEN_WIDTH 320
    #define SCREEN_HEIGHT 240


  // Digital Caliper GPIO pins              X=16/17, Y=4/5, Z=22/1(Serial)        -1 = not in use
    #define CLOCK_PIN_X 4
    #define DATA_PIN_X  16
    #define CLOCK_PIN_Y -1
    #define DATA_PIN_Y  -1
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

  const bool serialDebug = 1;                            // provide debug info on serial port
  const int serialSpeed = 115200;                        // Serial data speed to use

  // DRO readings display settings
      const int DROdisplaySpacing = 10;                   // DRO readout vertical space between displays
      const int DROnoOfDigits = 6;                       // number of digits to display
      const int DROwidth = 200;                          // width of DRO reading display   i.e. x position of buttons
      const int DROheight = 200; 
      const int DRObuttonheight = 30;                      
      const int DRObuttonWidth = 35;                     // buttons around DRO readings
      const int DROdbuttonPlacement = 70;                // vertical spacing of the DRO buttons (page1)
      const int checkDROreadings = 400;                  // how often to refresh DRO readings (ms)
  
  #define WDT_TIMEOUT 150                                // timeout of watchdog timer for esp32 (seconds) - Note: emails can take over a min to send

  #define ENABLE_EEPROM 0                                // if some settings are to be stored in eeprom
  // Note on esp32 this should really be replaced with preferences.h - see: by https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/

  #define ENABLE_OLED_MENU 0                             // Enable OLED / rotary encoder based menu

  #define ENABLE_EMAIL 0                                 // Enable E-mail support

  #define ENABLE_NEOPIXEL 0                              // Enable Neopixel support

  #define ENABLE_OTA 1                                   // Enable Over The Air updates (OTA)
  const String OTAPassword = "password";                 // Password to enable OTA service (supplied as - http://<ip address>?pwd=xxxx )

  const char* HomeLink = "/";                            // Where home button on web pages links to (usually "/")
  const uint16_t ServerPort = 80;                        // ip port to serve web pages on (usually 80)

  int dataRefresh = 2;                                   // Refresh rate of the updating data on web page (seconds)

  const byte LogNumber = 30;                             // number of entries in the system log

  const byte onboardLED = 25;                             // indicator LED pin - 2 or 16 on esp8266 nodemcu, 3 on esp8266-01, 2 on ESP32, 22 on esp32 lolin lite, 25 on the HiLetGo

  const byte onboardButton = 0;                          // onboard button gpio (FLASH)

  const bool ledBlinkEnabled = 1;                        // enable blinking status LED
  const uint16_t ledBlinkRate = 1500;                    // Speed to blink the status LED (milliseconds) - also perform some system tasks

  unsigned long wifiRetryTime = 30;                      // how often to try to reconnect to wifi when it is down (seconds)
  unsigned long wifiBackupRetryTime = 300;               // how often to try to reconnect to main wifi when using backup (seconds) - when using wifi2.h

  
// ---------------------------------------------------------------

// display
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

  bool displayRefreshX = 0;    // flag the caliper reading displays to refresh 
  bool displayRefreshY = 0; 
  bool displayRefreshZ = 0; 

  // store for current dro readings 
    float xReading, yReading, zReading;

  // adjustment for calipers (for the 3 coordinate systems)
    float xAdj[3] = {0};
    float yAdj[3] = {0};
    float zAdj[3] = {0};
    int currentCoord = 0;     // current one in use
  
  // screen buttons

    int displyingPage = 1;        // which screen page is being displayed
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
    
    // create button widgets
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
      ButtonWidget but5 = ButtonWidget(&tft); 
      ButtonWidget but6 = ButtonWidget(&tft); 

    // forward declarations for pressed button procedures
      void zeroXpressed();     // zero
      void zeroYpressed();
      void zeroZpressed();
      void zAllpressed();

      void halfXpressed();     // half
      void halfYpressed();
      void halfZpressed();      

      void coord1pressed();    // change coordinate system 
      void coord2pressed();
      void coord3pressed();

      void button4Pressed();   // misc
      void button5Pressed();
      void button6Pressed();

    const int buttonSpacing = 6;        // spacings used when placing the buttons on display
    
    // -define the buttons (see buttonStruct above)
      buttonStruct screenButtons[] {

        // page, enabled, title, x, y, width, height, border colour, burder thickness, fill colour, pressed colour, button, procedure

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

        // Page select buttons
        {1, 1, "P2", DROwidth + ( (DRObuttonWidth + buttonSpacing) * 2), 0, DRObuttonWidth, DRObuttonheight * 3 - buttonSpacing, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &but4, &button4Pressed},
        {2, 1, "Page1", 20, 140, 100, DRObuttonheight, TFT_WHITE, 1, TFT_ORANGE, TFT_BLACK, &but5, &button5Pressed},

        // misc buttons
        {2, 1, "Reboot", 200, 140, 100, DRObuttonheight, TFT_WHITE, 1, TFT_RED, TFT_BLACK, &but6, &button6Pressed}

      };
      uint8_t buttonCount = sizeof(screenButtons) / sizeof(screenButtons[0]);    // number of buttons created


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
  #include <EEPROM.h>                   // for storing settings in eeprom
#endif

#if ENABLE_OTA
  #include "ota.h"                      // Over The Air updates (OTA)
#endif

#if ENABLE_NEOPIXEL
  #include "neopixel.h"                 // Neopixels
#endif

#if ENABLE_OLED_MENU
  #include "oled.h"                     // OLED display - i2c version SSD1306
#endif

#if ENABLE_EMAIL
    #define _SenderName "ESP"           // name of email sender (no spaces)
    #include "email.h"
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
  tft.setRotation(1);           // set display to landscape
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
    ts.setRotation(1);  

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
      //screenButtons[b].btn->setPressAction(screenButtons[b].action);
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
      static repeatTimer caliperTimerX;                      // set up a repeat timer 
      if (caliperTimerX.check(checkDROreadings)) {           // repeat at set interval (ms)  
        if (!waitPinState(CLOCK_PIN_X, HIGH)) goto failedReadX;   // wait for gpio pin to change
        unsigned long tmpTime=micros();
        if (!waitPinState(CLOCK_PIN_X, LOW)) goto failedReadX;
        if((micros()-tmpTime)>500) {
          float tRead = readCaliper(CLOCK_PIN_X, DATA_PIN_X);  // read data from caliper
          if (tRead == 9999) goto failedReadX;                 // 9999 = failed to read caliper data
          if (tRead != xReading || displayRefreshX == 1) {     // if reading has changed or flag to refresh is set update display
            xReading = tRead;                                  // store result in global variable
            displayReadings();                                 // display caliper readings 
            displayRefreshX = 0;
          }
        }
      }
    }
    failedReadX:                                             // jump to here if read fails     

  // Digital Caliper - Y
    if (DATA_PIN_Y != -1) {                                  // if caliper is present
      static repeatTimer caliperTimerY;                      // set up a repeat timer 
      if (caliperTimerY.check(checkDROreadings + 7)) {       // repeat at set interval (ms)  
        if (!waitPinState(CLOCK_PIN_Y, HIGH)) goto failedReadY;   // wait for gpio pin to change
        unsigned long tmpTime=micros();
        if (!waitPinState(CLOCK_PIN_Y, LOW)) goto failedReadY;
        if((micros()-tmpTime)>500) {
          float tRead = readCaliper(CLOCK_PIN_Y, DATA_PIN_Y); // read data from caliper
          if (tRead == 9999) goto failedReadY;                // 9999 = failed to read caliper data
          if (tRead != yReading|| displayRefreshY == 1) {     // if reading has changed or flag to refresh is set update display
            yReading = tRead;                                 // store result in global variable
            displayReadings();                                // display caliper readings 
            displayRefreshY = 0;
          }
        }
      }
    }
    failedReadY:                                             // jump to here if read fails 

  // Digital Caliper - Z
    if (DATA_PIN_Z != -1) {                                  // if caliper is present
      static repeatTimer caliperTimerZ;                      // set up a repeat timer 
      if (caliperTimerZ.check(checkDROreadings + 13)) {      // repeat at set interval (ms)  
        if (!waitPinState(CLOCK_PIN_Z, HIGH)) goto failedReadZ;   // wait for gpio pin to change
        unsigned long tmpTime=micros();
        if (!waitPinState(CLOCK_PIN_Z, LOW)) goto failedReadZ;
        if((micros()-tmpTime)>500) {
          float tRead = readCaliper(CLOCK_PIN_Z, DATA_PIN_Z); // read data from caliper
          if (tRead == 9999) goto failedReadY;                // 9999 = failed to read caliper data
          if (tRead != zReading|| displayRefreshZ == 1) {     // if reading has changed or flag to refresh is set update display
            zReading = tRead;                                 // store result in global variable
            displayReadings();                                // display caliper readings 
            displayRefreshZ = 0;
          }
        }
      }   
    }
    failedReadZ:                                             // jump to here if read fails  

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
      if (screenButtons[b].enabled == 1 && screenButtons[b].page == displyingPage && screenButtons[b].btn->contains(x, y)) {
          tft.setFreeFont(FM9);         // standard Free Mono font - available sizes: 9, 12, 18 or 24
          tft.setTextSize(2);        
          screenButtons[b].btn->drawSmoothButton(true);
          delay(500);
          screenButtons[b].btn->drawSmoothButton(false);
          if (serialDebug) Serial.println("button " + String(screenButtons[b].label) + " pressed!");
          screenButtons[b].btn->press(true);
          //screenButtons[b].btn->pressAction();          
          screenButtons[b].action();   // call the buttons procedure 
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
  displyingPage = screen;       // set current page

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
// Note - Using transistors inverts the signal levels          returns 9999 if failed to get data
// see: http://wei48221.blogspot.com/2016/01/using-digital-caliper-for-digital-read_21.html

float readCaliper(int clockPin, int dataPin) {

  int sign = 1;         // if the reading is negative
  int inches = 0;       // flag if data is in inches (0 = mm) -  NOTE: this doesn't seem to work (always 1)?
  long value = 0;       // reading from caliper  
  for(int i=0;i<24;i++) {                                 // read the data bits 
    if (!waitPinState(clockPin, HIGH)) return 9999;       // wait for gpio pin state to change
    if (!waitPinState(clockPin, LOW)) return 9999;
    if(digitalRead(dataPin)==LOW) {
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
      if (displyingPage == 1) tft.setFreeFont(&sevenSeg35pt7b);         // seven segment style font from sevenSeg.h  
      else tft.setFreeFont(&sevenSeg16pt7b);

    // Cheap Yellow Display
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextSize(1);
      tft.setTextPadding(DROwidth);           // this clears the previous text (it should anyway)

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
      if (displyingPage == 1) {
        sprintf(buff, "C%d", currentCoord + 1);
        tft.setFreeFont(FM12);         // standard Free Mono font - available sizes: 9, 12, 18 or 24
        tft.setTextSize(1);            // 1 or 2  
        tft.setTextColor(TFT_MAROON, TFT_BLACK);
        tft.drawString(buff, ( (DRObuttonWidth + buttonSpacing) * 3), DROheight + 5);   
      }

    tft.setTextPadding(0);        // turn off text padding
}


//      ********************************************************************

//                                   button actions


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
  xAdj[currentCoord] = xReading;    // zero X
  yAdj[currentCoord] = yReading;    // zero Y
  zAdj[currentCoord] = zReading;    // zero Z
}

void halfXpressed() { 
  log_system_message("button: halfX");
  float t = (xReading - xAdj[currentCoord]) / 2.0;  // half of current displayed reading
  xAdj[currentCoord] = xAdj[currentCoord] + t;      
}

void halfYpressed() { 
  log_system_message("button: halfY");
  float t = (yReading - yAdj[currentCoord]) / 2.0;  // half of current displayed reading
  yAdj[currentCoord] = yAdj[currentCoord] + t;   
}

void halfZpressed() { 
  log_system_message("button: halfZ");
  float t = (zReading - zAdj[currentCoord]) / 2.0;  // half of current displayed reading
  yAdj[currentCoord] = zAdj[currentCoord] + t;   
}

void coord1pressed() { 
  log_system_message("button: C1");
  currentCoord = 0;
}

void coord2pressed() { 
  log_system_message("button: C2");
  currentCoord = 1;
}

void coord3pressed() { 
  log_system_message("button: C3");
  currentCoord = 2;
}

void button4Pressed() { 
  log_system_message("button: screen 2");
  drawScreen(2);      // switch to page 2
}

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


// simulate half x button press
  halfXpressed(); 


// switch pages
  if (displyingPage == 1) drawScreen(2);  
  else  drawScreen(1);  



/*
// demo of how to request a web page
  // request web page
    String page = "http://192.168.1.166/ping";   // url to request
    //String page = "http://192.168.1.176:84/rover.asp?action=devE11on";   // url to request
    String response;                             // reply will be stored here
    int httpCode = requestWebPage(&page, &response);
  // display the reply
    client.println("Web page requested: '" + page + "' - http code: " + String(httpCode));
    client.print("<xmp>'");     // enables the html code to be displayed
    client.print(response);
    client.println("'</xmp>");
  // test the reply
    response.toLowerCase();                    // convert all text in the reply to lower case
    int tPos = response.indexOf("closed");     // search for text in the reply - see https://www.arduino.cc/en/Tutorial/BuiltInExamples
    if (tPos != -1) {
      client.println("rusult contains 'closed' at position " + String(tPos) + "<br>");
    }
*/



  // -----------------------------------------------------------------------------


  // end html page
    webfooter(client);            // add the standard web page footer
    delay(1);
    client.stop();
}


// --------------------------- E N D -----------------------------
