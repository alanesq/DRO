# Libraries

## The libraries required for the DRO sketch are:
 - WiFiManager
 - XPT2046_Touchscreen
 - TFT_eSPI
 - TFT_eWidget

They can all be installed from the library manager in the Arduino IDE 

## Once installed, the TFT_eSPI library requires some minor modifications 
So that the library will work with the Cheap Yellow Display, once you have installed it copy the files here in to /TFT_eSPI folder where your libraries are located <br><br>

Note: if using the Cheap Yellow Display with two usb ports it has aslightly different display so edit "User_Setup_Select.h" 
- comment out :   #include <User_Setup-std.h>    
- uncomment:      #include <User_Setup-dual.h>     
close the Arduino IDE and re-open it to ensure the library will be re-compiled

## Additional info

I have read that if you have the ESP32-2432S032 variant, change line 131 of the User_setup.h
- from: #define TFT_BL   21            // LED back-light control pin 
- to: #define TFT_BL   27            // LED back-light control pin

