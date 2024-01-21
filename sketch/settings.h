/* -----------------------------------------------------------------------------------------


          DRO Settings - 21Jan24

          Part of the  "SuperLowBudget-DRO" sketch - https://github.com/alanesq/DRO

          
// -----------------------------------------------------------------------------------------
*/


  const char* stitle = "SuperLowBudget-DRO";             // title of this sketch

  const char* sversion = "21Jan24";                      // version of this sketch

  // OTA
    #define ENABLE_OTA 1                                 // Enable Over The Air updates (OTA)
    const String OTAPassword = "password";               // Password to enable OTA (supplied as - http://<ip address>?pwd=xxxx )

  bool wifiEnabled = 0;                                  // if wifi is to be enabled at boot (can be enabled from display menu if turned off here)
  
  const bool serialDebug = 0;                            // provide debug info on serial port  (disable if using Tx or Rx gpio pins for caliper)
  const int serialSpeed = 115200;                        // Serial data speed to use  
  
  const bool showPress = 0;                              // show touch data on screen (for calibration / testing)

  const bool invertCaliperDataSignals = 1;               // If using transistors on the data and clock pins the signals will be inverted so set this to 1

  // web page
    const int gcodeTextHeight = 15;                      // Size of text box on main web page for entering gcode
    const int gcodeTextWidth = 40;

  const int p1HoldButtonDelay = 5000;                    // how long to wait when HOLD button is pressed on page 1 (ms)

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
      #define CLOCK_PIN_X 0     // 0   Note: gpio is the onboard button (it can stop the device rebooting after programming)
      #define DATA_PIN_X  4     // 4
      
      #define CLOCK_PIN_Y 17    // 17
      #define DATA_PIN_Y  16    // 16
      
      #define CLOCK_PIN_Z 22    // 22
      #define DATA_PIN_Z  5     // 5   Note: this pin is used by the SD card (chip select)


  // CYD - Touchscreen 

    #define TOUCH_LEFT 172         // touch positions relating to the display (may be better to have some kind of calibration routine?)
    #define TOUCH_RIGHT 3647
    #define TOUCH_TOP 239
    #define TOUCH_BOTTOM 3820
    #define XPT2046_IRQ 36
    #define XPT2046_MOSI 32
    #define XPT2046_MISO 39
    #define XPT2046_CLK 25
    #define XPT2046_CS 33       
    #define TOUCH_CS PIN_D2    

      
  // ----------------------------------------------------------------------------------------------------
  // end
