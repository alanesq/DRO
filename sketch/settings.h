/* -----------------------------------------------------------------------------------------


          DRO Settings 

          Part of the  "SuperLowBudget-DRO" sketch - https://github.com/alanesq/DRO

          
// -----------------------------------------------------------------------------------------
*/


  const char* stitle = "SuperLowBudget-DRO";             // title of the sketch

  const char* sversion = "09Feb24";                      // version of the sketch

  // OTA
    #define ENABLE_OTA 1                                 // Enable Over The Air updates (OTA)
    const String OTAPassword = "password";               // Password to enable OTA (supplied as - http://<ip address>?pwd=xxxx )

  bool wifiEnabled = 0;                                  // if wifi is to be enabled at boot (can be enabled from display menu if turned off here)
  int wifiType = 1;                                      // The type of wifi connection to use  - 1=Use WifiManager, 2=specify wifi credentials, 3=Access point

  const bool serialDebug = 1;                            // provide debug info on serial port  (disable if using Tx or Rx gpio pins for caliper)
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

  const int noOfCoordinates = 4;                         // number of coordinate systems available
  int currentCoord = 0;                                  // active coordinate    

  const int maxGcodeStringLines = 1024;                  // maximum number of lines of gcode allowed
  const int estimateAverageLineLength = 35;              // average line length


  // CYD - Display 
    #define SCREEN_ROTATION 1
    #define SCREEN_WIDTH 320
    #define SCREEN_HEIGHT 240
    #define SCREEN_BACKLIGHT 21


  // Digital calipers

      // structure for the calipers (do not modify)
      struct caliperStruct {
        String title;                           // title of caliper (just a single upper case letter is best, e.g. X, Y and Z)
        bool enabled;                           // if caliper is enabled
        int clockPIN;                           // gpio pins
        int dataPIN;
        bool direction;                         // set to 1 is direction is reversed
        unsigned long lastReadTime;             // last time a reading was received (millis)
        float reading;                          // current reading
        float adj[noOfCoordinates];             // adjustment of caliper reading to displayed reading for each coordinate system
      };

    // define calipers in use
      const int caliperCount = 3;               // number of calipers in use 
      
      caliperStruct calipers[] {  
        // Items: name (single character, upper case), if enabled, clock gpio pin, data gpio pin, direction (1=reversed), 0, 9999, {0}
        
        { "X", 1,  0,  4, 0, 0, 9999, {0} },        // Note: pin 0 is the onboard button (it can stop the device rebooting after programming)
        
        { "Y", 1, 17, 16, 0, 0, 9999, {0} },
        
        { "Z", 1, 22,  5, 0, 0, 9999, {0} }         // Note: pin 5 is used by the SD card (chip select)
        
      };


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
    //#define TOUCH_CS PIN_D2    

      
  // ----------------------------------------------------------------------------------------------------
  // end

