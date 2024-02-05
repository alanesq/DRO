/**************************************************************************************************
 *
 *      Procedures (which are likely to be the same for all projects) - 21Nov23
 *
 *      part of the BasicWebserver sketch - https://github.com/alanesq/BasicWebserver
 *
 **************************************************************************************************/

#include <Arduino.h>  

// // Forward declarations 
  class Button;
  class repeatTimer;
  class Led;
  String decodeIP(String);
  void log_system_message(String);
  void webheader(WiFiClient&, const char* adnlStyle = "", int refresh = 0);
  void webfooter(WiFiClient&);
  void handleLogpage();
  void handleNotFound();
  void handleReboot();
  int WIFIcheck();
  bool debounceReadGPIO(int, bool);
      

// ----------------------------------------------------------------
//                              -Startup
// ----------------------------------------------------------------

// some useful html/css 
  const char colRed[] = "<span style='color:red;'>";        // red text
  const char colGreen[] = "<span style='color:green;'>";    // green text
  const char colBlue[] = "<span style='color:blue;'>";      // blue text
  const char colEnd[] = "</span>";                          // end coloured text
  const char htmlSpace[] = "&ensp;";                        // leave a space  (see 'HTML entity')

// misc variables
  String lastClient = "n/a";                  // IP address of most recent client connected

// log  
  const int maxLogEntryLength = 100;          // max character length of a log entry
  int system_message_pointer = 0;             // pointer for current system message position
  String system_message[LogNumber + 1];       // system log message store  (suspect serial port issues caused if not +1 ???)


// ----------------------------------------------------------------
//                    -decode IP addresses
// ----------------------------------------------------------------
// Check for known IP addresses

String decodeIP(String IPadrs) {

    if (IPadrs == "192.168.1.100") IPadrs = "pc";
    else if (IPadrs == "192.168.1.101") IPadrs = "laptop";


    // log last IP client connected
      if (IPadrs != lastClient) {
        lastClient = IPadrs;
        log_system_message("New IP client connected: " + IPadrs);
      }

    return IPadrs;
}



// ----------------------------------------------------------------
//                          -repeating Timer
// ----------------------------------------------------------------
// repeat an operation periodically
// usage example:   static repeatTimer timer1;             // set up a timer
//                  if (timer1.check(2000)) {do stuff};    // repeat every 2 seconds

class repeatTimer {

  private:
    uint32_t  _lastTime;                                              // store of last time the event triggered
    bool _enabled;                                                    // if timer is enabled

  public:
    repeatTimer() {
      reset();
      enable();
    }

    bool check(uint32_t timerInterval, bool timerReset=1) {           // check if supplied time has passed
      if ( (unsigned long)(millis() - _lastTime) >= timerInterval ) {
        if (timerReset) reset();
        if (_enabled) return 1;
      }
      return 0;
    }

    void disable() {                                                  // disable the timer
      _enabled = 0;
    }

    void enable() {                                                   // enable the timer
      _enabled = 1;
    }

    void reset() {                                                    // reset the timer
      _lastTime = millis();
    }
};


// ----------------------------------------------------------------
//                      -log a system message
// ----------------------------------------------------------------

void log_system_message(String smes) {

  // increment position pointer
    system_message_pointer++;
    if (system_message_pointer >= LogNumber) system_message_pointer = 0;

  // add the new message to log
    String tStore = currentTime() + " - " + smes;
    tStore.substring(0, maxLogEntryLength - 1);                 // limit length of entry
    system_message[system_message_pointer] = tStore;

  // also send the message to serial
    if (serialDebug) Serial.println("Log:" + system_message[system_message_pointer]);

  // also send message to oled if debug is enabled
    #if ENABLE_OLED_MENU
      if (serialDebug) displayMessage("Log:", system_message[system_message_pointer]);
    #endif

}


// ----------------------------------------------------------------
//                         -header (html)
// ----------------------------------------------------------------
// HTML at the top of each web page
// @param   client    the http client
// @param   adnlStyle any additional style to be included - default = ""
// @param   refresh   enable page auto refreshing (1 or 0) - default = 0

void webheader(WiFiClient &client, const char* adnlStyle, int refresh) {

  // start html page
    client.write("HTTP/1.1 200 OK\r\n");
    client.write("Content-Type: text/html\r\n");
    client.write("Connection: close\r\n");
    client.write("\r\n");
    client.write("<!DOCTYPE HTML>\n");
    client.write("<html lang='en'>\n");
    client.write("<head>\n");
    client.write(" <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n");
  // page refresh
    if (refresh > 0) client.printf(" <meta http-equiv='refresh' content='%d'>\n", refresh);

 // this is the below section compacted via https://www.textfixer.com/html/compress-html-compression.php
  client.printf(R"=====(<title>%s</title><style>body {color: black;background-color: #FFFF00;text-align: center;}ul {list-style-type: none; margin: 0; padding: 0; overflow: hidden; background-color: rgb(128, 64, 0);}li {float: left;}li a {display: inline-block; color: white; text-align: center; padding: 30px 20px; text-decoration: none;}li a:hover { background-color: rgb(100, 0, 0);}%s</style></head><body><ul><li><a href='%s'>Home</a></li><li><a href='/log'>Log</a></li><h1> <font color='#FF0000'>%s</font></h1></ul>)=====", stitle, adnlStyle, HomeLink, stitle);

/*
    // HTML / CSS
      client.printf(R"=====(
          <title>%s</title>
          <style>
            body {
              color: black;
              background-color: #FFFF00;
              text-align: center;
            }
            ul {list-style-type: none; margin: 0; padding: 0; overflow: hidden; background-color: rgb(128, 64, 0);}
            li {float: left;}
            li a {display: inline-block; color: white; text-align: center; padding: 30px 20px; text-decoration: none;}
            li a:hover { background-color: rgb(100, 0, 0);}
            %s
          </style>
        </head>
        <body>
          <ul>
            <li><a href='%s'>Home</a></li>
            <li><a href='/log'>Log</a></li>
            <h1> <font color='#FF0000'>%s</font></h1>
          </ul>
      )=====", stitle, adnlStyle, HomeLink, stitle);
*/
}


// ----------------------------------------------------------------
//                             -footer (html)
// ----------------------------------------------------------------
// HTML at the end of each web page
// @param   client    http client

void webfooter(WiFiClient &client) {

   // get mac address
     byte mac[6];
     WiFi.macAddress(mac);

   client.print("<br>\n");

   // Status display at bottom of screen 

     client.write("<div style='text-align: center;background-color:rgb(128, 64, 0)'>\n");
     client.printf("<small> %s", colRed);
     client.printf("%s %s", stitle, sversion);

     client.printf(" | Memory: %dK", ESP.getFreeHeap() /1000);

     // wifi
      client.printf(" | Wifi: %ddBm", WiFi.RSSI());
      client.println("(" + String(wifiok) + ")");

     // NTP server link status
      int tstat = timeStatus();   // ntp status
      if (tstat == timeSet) client.print(" | NTP OK");
      else if (tstat == timeNeedsSync) client.print(" | NTP Sync failed");
      else if (tstat == timeNotSet) client.print(" | NTP Failed");

//    // SPIFFS
//      client.printf(" | Spiffs: %dK", ( SPIFFS.totalBytes() - SPIFFS.usedBytes() / 1024 ) );             // if using spiffs

//      // MAC address
//       client.print(" | MAC: ");
//        client.println(WiFi.macAddress());
        //client.printf("%2x%2x%2x%2x%2x%2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);       // mac address

//     // free PSRAM
//       if (psramFound()) {
//         client.printf(" | PSram: %dK", heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024 );
//       }

     client.printf("%s </small>\n", colEnd);
     client.write("</div>\n");

  /* end of HTML */
   client.write("</body>\n");
   client.write("</html>\n");

}


// ----------------------------------------------------------------
//   -log web page requested    i.e. http://x.x.x.x/log
// ----------------------------------------------------------------

void handleLogpage() {

  WiFiClient client = server.client();                     // open link with client

  // log page request including clients IP address
    IPAddress cip = client.remoteIP();
    String clientIP = decodeIP(cip.toString());   // check for known IP addresses
    //log_system_message("Log page requested from: " + clientIP);


  // build the html for /log page

    webheader(client);                         // send html page header

    client.println("<P>");                     // start of section

    client.println("<br>SYSTEM LOG<br><br>");

    // list all system messages
    int lpos = system_message_pointer;         // most recent entry
    for (int i=0; i < LogNumber; i++){         // count through number of entries
      client.print(system_message[lpos]);
      client.println("<br>");
      if (lpos == 0) lpos = LogNumber;
      lpos--;
    }

    // close html page
      webfooter(client);                       // send html page footer
      delay(3);
      client.stop();

}


// ----------------------------------------------------------------
//                      -invalid web page requested
// ----------------------------------------------------------------

void handleNotFound() {

  WiFiClient client = server.client();          // open link with client

  // log page request including clients IP address
    IPAddress cip = client.remoteIP();
    String clientIP = decodeIP(cip.toString());   // check for known IP addresses
    log_system_message("Invalid URL requested from " + clientIP);

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 200, "text/plain", message );
  message = "";      // clear variable

}


// ----------------------------------------------------------------
//   -reboot web page requested        i.e. http://x.x.x.x/reboot
// ----------------------------------------------------------------
// note: this can fail if the esp has just been reflashed and not restarted

void handleReboot() {

      String message = "Rebooting....";
      server.send(200, "text/plain", message);   // send reply as plain text

      // rebooting
        delay(500);          // give time to send the above html
        ESP.restart();
        delay(5000);         // restart fails without this delay

}


// --------------------------------------------------------------------------------------
//                                 -wifi connection check
// --------------------------------------------------------------------------------------
// Periodic check of wifi status

int WIFIcheck() {

  // if wifi is still not connected
    if (WiFi.status() != WL_CONNECTED) {       
      if ( wifiok > 0 ) {    // wifi was flagged as ok
            log_system_message("WifiCheck: Wifi connection lost");
            wifiok = 0;
            wifiDownTime = millis();   // record time it went down
            #if WIFIUSED == 2   
              startWifi();    
            #else
              WiFi.reconnect();   
            #endif
      }
    } else {   
        if ( wifiok == 0 ) {  // wifi was flagged as down
              if (String(SSID) != WiFi.SSID()) {
                log_system_message("WifiCheck: Backup Wwfi connection is back");
                wifiok = 2;    
              } else {
                log_system_message("WifiCheck: Main wifi connection is back");
                wifiok = 1; 
              }
              wifiDownTime = 0;
        }
    }   
    

    // peridic attempt to re-connect to wifi
    static repeatTimer wifiBackupTimer;                           // set up a repeat timer (see standard.h)
    if (wifiBackupTimer.check(wifiRetryTime * 1000)) {            // repeat at set interval (ms)   

      // try to reconnect to main wifi if connected to backup wifi
        if (wifiok == 2) {    
          //log_system_message("WifiCheck: Attempting to connect back to main wifi");
          // connected to backup wifi so search available networks to see if main wifi is available
            bool mAv = 0;   // flag if main wifi was found
            int n = WiFi.scanNetworks();
            if (n > 0) {
              for (int i = 0; i < n; ++i) 
                  if (WiFi.SSID(i)  == SSID) mAv = 1;    // network found
            }
          if (mAv == 1) {
            log_system_message("WifiCheck: Main wifi found - attempting to connect");
            startWifi();
          }
        }

      // try to re-reconnect to main wifi
        if (wifiok == 0) {
          #if WIFIUSED == 1        // if using wifi.h
            //log_system_message("WifiCheck: attempting to re-connect to wifi");
            WiFi.reconnect();    
          #else                    // if using wifi2.h
            //log_system_message("WifiCheck: attempting to connect to a wifi");
            WiFi.disconnect();
            startWifi();
          #endif
        }  
    }
          
  return wifiok;
}


// ----------------------------------------------------------------
//               -read a gpio pin status and debounce
// ----------------------------------------------------------------
// supply GPIO pin and the value it is expected to be

bool debounceReadGPIO(int pinGPIO, bool expectedResult) {
  bool tStore = digitalRead(pinGPIO);
  if (tStore != expectedResult) {
    delay(50);
    tStore = digitalRead(pinGPIO);
  }
  return tStore;
}


// --------------------------------------------------------------------------------------
//                                       -LED control
// --------------------------------------------------------------------------------------
// @param  gpio pin, if LED ON when pin is high or low
// useage:      Led led1(LED_1_GPIO_PIN, GPIO_STATUS_WHEN_ON, STATUS_TO_SET_NOW);     
//              led1.on();

class Led {

  private:
    byte _gpioPin;                                     // gpio pin of the LED
    bool _onState;                                     // logic state when LED is on

  public:
    Led(byte gpioPin, bool onState=HIGH, bool currentState=LOW) {
      this->_gpioPin = gpioPin;
      this->_onState = onState;
      pinMode(_gpioPin, OUTPUT);
      (currentState==HIGH) ? on() : off();
    }

    void on() {
      digitalWrite(_gpioPin, _onState);
    }

    void off() {
      digitalWrite(_gpioPin, !_onState);
    }

    void flip() {
      digitalWrite(_gpioPin, !digitalRead(_gpioPin));   // flip the led status
    }

    bool status() {                                     // returns 1 if LED is on
      bool sState = digitalRead(_gpioPin);
      if (_onState == HIGH) return sState;
      else return !sState;
    }

    void flash(int fLedDelay=350, int fReps=1) {
      if (fLedDelay < 0 || fLedDelay > 20000) fLedDelay=300;   // capture invalid delay
      bool fTempStat = digitalRead(_gpioPin);
      if (fTempStat == _onState) {                      // if led is already on
        off();
        delay(fLedDelay);
      }
      for (int i=0; i<fReps; i++) {
        on();
        delay(fLedDelay);
        off();
        delay(fLedDelay);
      }
      if (fTempStat == _onState) on();                  // return led status if it was on
    }
};


// --------------------------------------------------------------------------------------
//                                    -button control
// --------------------------------------------------------------------------------------
// @param  gpio pin, normal state (i.e. high or low when not pressed)
// example useage:      Button button1(12, HIGH);
//                      if (button1.beenPressed()) {do stuff};

class Button {

  private:
    uint32_t _debounceDelay = 40;              // default debounce setting (ms)
    uint32_t _timer;                           // timer used for debouncing
    bool     _rawState;                        // raw state of the button
    byte     _gpiopin;                         // gpio pin of the button
    bool     _normalState;                     // gpio state when button not pressed
    bool     _debouncedState;                  // debounced state of button
    bool     _beenReleased = HIGH;             // flag to show the button has been released

  public:
    Button(byte bPin, bool bNormalState = LOW) {
      this->_gpiopin = bPin;
      this->_normalState = bNormalState;
      pinMode(_gpiopin, INPUT);
      _debouncedState = digitalRead(_gpiopin);
      _timer = millis();
    }

    bool update() {                            // update and return debounced button status
      bool newReading = digitalRead(_gpiopin);
      if (newReading != _rawState) {
        _timer = millis();
        _rawState = newReading;
      }
      if ( (unsigned long)(millis() - _timer) > _debounceDelay) _debouncedState = newReading;
      return _debouncedState;
    }

    bool isPressed() {                          // returns TRUE if button is curently pressed
      if (_normalState == LOW) return update();
      else return (!update());
    }

    bool beenPressed() {                        // returns TRUE once per button press
      bool cState = isPressed();
      if (cState == LOW) _beenReleased = HIGH;
      if (_beenReleased == LOW) cState = LOW;
      if (cState == HIGH) _beenReleased = LOW;
      return cState;
    }

    void debounce(int debounceDelay) {          // change debounce delay setting
      _debounceDelay = max(debounceDelay, 0);
    }

};

// --------------------------- E N D -----------------------------
