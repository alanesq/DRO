/* -----------------------------------------------------------------------------------------


          DRO Settings 

          Part of the  "SuperLowBudget-DRO" sketch - https://github.com/alanesq/DRO

          Ensure you have modified the library for working with the CDY - see: https://github.com/alanesq/DRO/tree/main/LIBRARIES
          If using the two usb port version of the 'Cheap Yellow Display' change 'User_Setup_Select.h' in the TFT_eSPI library


// -----------------------------------------------------------------------------------------
*/


                const char* stitle = "Super Low Budget DRO";    

      
                const char* sversion = "18Feb24";              



// ----------------------------------------------------------------------

      
  bool wifiEnabled = 0;                                 // If wifi is enabled by default (can be enabled in DRO menu)
  int wifiType = 1;                                     // Type of wifi to use (1=WifiManager, 2=manual wifi credentials, 3=access point)      

  const String OTAPassword = "password";                // Password to enable OTA (supplied as - http://<ip address>?pwd=xxxx )

  const bool serialDebug = 0;                            // provide debug info on serial port  (disable if using Tx or Rx gpio pins for caliper)
  const int serialSpeed = 115200;                        // Serial data speed to use  

  const bool invertCaliperDataSignals = 1;               // If using transistors on the data and clock pins the signals will be inverted so set this to 1

  // web page
    const int gcodeTextHeight = 15;                      // Size of text box on main web page for entering gcode
    const int gcodeTextWidth = 50;

  const int p1HoldButtonDelay = 5000;                    // how long to wait when HOLD button is pressed on page 1 (ms)

  #define WDT_TIMEOUT 60                                 // timeout of watchdog timer (seconds) - i.e. auto restart if sketch stops responding

  const char* HomeLink = "/";                            // Where home button on web pages links to (usually "/")
  const uint16_t ServerPort = 80;                        // ip port to serve web pages on (usually 80)

  int dataRefresh = 2;                                   // Refresh rate of the updating data on root web page (seconds)

  const byte LogNumber = 75;                             // number of entries in the system log

  unsigned long wifiRetryTime = 30;                      // how often to try to reconnect to wifi if connection is lost (seconds)   

  const byte noOfCoordinates = 4;                        // number of coordinate systems available
  int currentCoord = 0;                                  // active coordinate    (0 to noOfCoordinates - 1)
  
  const int maxGcodeStringLines = 512;                   // maximum number of lines of gcode allowed
  const int estimateAverageLineLength = 30;              // average line length

  // CYD - Display 
    #define SCREEN_ROTATION 1
    #define SCREEN_WIDTH 320
    #define SCREEN_HEIGHT 240
    #define SCREEN_BACKLIGHT 21


  // Digital calipers

    float highestAllowedReading = 9999.00;     // readings outside this range are flagged as an error
    float lowestAllowedReading  = -999.00;

    // structure for the calipers
      struct caliperStruct {
        const String title;                     // title of caliper (just a single upper case letter is best, e.g. X, Y and Z)
        const bool enabled;                     // if caliper is enabled
        const int clockPIN;                     // gpio pins
        const int dataPIN;
        bool direction;                         // set to 1 is direction is reversed
        unsigned long lastReadTime;             // last time a reading was received (millis)
        float reading;                          // current reading
        int error;                              // error status (0=ok)
        float adj[noOfCoordinates];             // adjustment of caliper reading to displayed reading for each coordinate system
      };

    // define calipers in use
      const int caliperCount = 3;               // number of calipers below
      
      caliperStruct calipers[] {  
        // Items: Axis name (single character, upper case), if enabled, clock gpio pin, data gpio pin, direction (1=reversed), 0, 0.0, 0, {0.0}
        // NOTE: Some of the menus and buttons expect there to be 3 axes but you can set the enabled to '0' if they are not required

        { "X", 1,  0,  4, 0, 0, 0.0, 0, {0.0} },        // Note: pin 0 is the onboard button (it can stop the device rebooting after programming)
        
        { "Y", 1, 17, 16, 0, 0, 0.0, 0, {0.0} },
        
        { "Z", 1, 22,  5, 0, 0, 0.0, 0, {0.0} }         // Note: pin 5 is used by the SD card (chip select)
        
      };

  // CYD - Touchscreen 

    #define TOUCH_LEFT 172         // touch positions relating to the display (may be better to have some kind of calibration routine?)
    #define TOUCH_RIGHT 3680
    #define TOUCH_TOP 239
    #define TOUCH_BOTTOM 3820
    #define XPT2046_IRQ 36
    #define XPT2046_MOSI 32
    #define XPT2046_MISO 39
    #define XPT2046_CLK 25
    #define XPT2046_CS 33       

      
  // ----------------------------------------------------------------------------------------------------
  // end

