/* -----------------------------------------------------------------------------------------


          DRO Settings - 19Jan24

          Part of the  "SuperLowBudget-DRO" sketch - https://github.com/alanesq/DRO

          
// -----------------------------------------------------------------------------------------
*/

  const char* stitle = "SuperLowBudget-DRO";             // title of this sketch

  const char* sversion = "19Jan24";                      // version of this sketch

  // OTA
    #define ENABLE_OTA 1                                 // Enable Over The Air updates (OTA)
    const String OTAPassword = "password";               // Password to enable OTA (supplied as - http://<ip address>?pwd=xxxx )

  bool wifiEnabled = 0;                                  // if wifi is to be enabled at boot (can be enabled from display menu if turned off here)
  
  const bool serialDebug = 0;                            // provide debug info on serial port  (disable if using Tx or Rx gpio pins for caliper)
  const int serialSpeed = 115200;                        // Serial data speed to use  
  
  const bool showPress = 0;                              // show touch data on screen 

  const bool invertCaliperDataSignals = 1;               // If using transistors on the data and clock pins the signals will be inverted so set this to 1

  const int checkDROreadings = 50;                       // how often to refresh DRO readings (ms)  
  
  // web page
    const int gcodeTextHeight = 15;                      // Size of text box on main web page for entering gcode
    const int gcodeTextWidth = 40;

  #define WDT_TIMEOUT 60                                 // timeout of watchdog timer (seconds) - i.e. auto restart if sketch stops responding

  const char* HomeLink = "/";                            // Where home button on web pages links to (usually "/")
  const uint16_t ServerPort = 80;                        // ip port to serve web pages on (usually 80)

  int dataRefresh = 2;                                   // Refresh rate of the updating data on root web page (seconds)

  const byte LogNumber = 50;                             // number of entries in the system log

  unsigned long wifiRetryTime = 30;                      // how often to try to reconnect to wifi if connection is lost (seconds)   


  // CYD - Display 
    #define SCREEN_ROTATION 1
    #define SCREEN_WIDTH 320
    #define SCREEN_HEIGHT 240
    #define SCREEN_BACKLIGHT 21


  // Digital calipers
  
    // if caliper is enabled - set to 0 if not in use
      bool xEnabled = 1;
      bool yEnabled = 1;
      bool zEnabled = 1;

    // direction of travel - i.e. if direction of caliper is reversed 
      const bool reverseXdirection = 0;       
      const bool reverseYdirection = 0;
      const bool reverseZdirection = 0;    

    // Caliper gpio pins (set to -1 if not used)
      #define CLOCK_PIN_X 0     // 0   Note: gpio is the onboard button so not really a good idea to use it but it seems to work (it can stop the device rebooting its self)
      #define DATA_PIN_X  4     // 4
      
      #define CLOCK_PIN_Y 17    // 17
      #define DATA_PIN_Y  16    // 16
      
      #define CLOCK_PIN_Z 22    // 22
      #define DATA_PIN_Z  5     // 5   Note: this pin is used by the SD card (chip select)


  // CYD - Touchscreen 

    #define TOUCH_LEFT 200         // touch positions relating to the display (may be better to have some kind of calibration routine?)
    #define TOUCH_RIGHT 3700
    #define TOUCH_TOP 200
    #define TOUCH_BOTTOM 3850
    #define XPT2046_IRQ 36
    #define XPT2046_MOSI 32
    #define XPT2046_MISO 39
    #define XPT2046_CLK 25
    #define XPT2046_CS 33       
    #define TOUCH_CS PIN_D2    
  
  
    
  // --------------------------------------- Menu/screen settings ---------------------------------------

    
  // font to use for all buttons  
    #define MENU_FONT FM9                                // standard Free Mono font - available sizes: 9, 12, 18 or 24
    #define MENU_SIZE 2                                  // font size (1 or 2)

    // Page selection buttons
    // Note: to add more pages increase 'numberOfPages', modify the two sections marked with '#noPages' and add procedure to 'buttons.h'
      const int numberOfPages = 4;
      const int pageButtonWidth = 32;                    // width of the buttons
      const int pageButtonSpacing = 6;                   // space between the buttons
      const int pageButtonHeight = (SCREEN_HEIGHT / numberOfPages) - pageButtonSpacing;  // height of button
  
    // DRO readings size/position settings 
        const int DROdisplaySpacing = 10;                // space between the DRO readings
        const int DROnoOfDigits1 = 4;                    // number of digits to display in caliper readings (before decimal)
        const int DROnoOfDigits2 = 2;                    // number of digits to display in caliper readings (after decimal)
        const int DROwidth = 200;                        // width of DRO reading display   i.e. x position of zero buttons  
        const int DROheight = 200;                       // Height of the DRO reading display for placement of lower buttons  
        const int DRObuttonheight = 30;                  // buttons around DRO readings              
        const int DRObuttonWidth = 35;                     
        const int DROdbuttonPlacement = 70;              // vertical spacing of the DRO buttons  

    // Hold button
        const int p1HoldButtonDelay = 5000;              // how long to wait when HOLD button is pressed (ms)
        
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

      
  // ----------------------------------------------------------------------------------------------------
  // end
