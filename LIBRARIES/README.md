
modify the "TFT_eSPI" to work with the Cheap Yellow Display - 18th Jan 2024
-----------------------------------------------------------


copy these files in to the "TFT_eSPI" folder where your libraries are stored


Note: if using the Cheap Yellow Display with two usb ports edit "User_Setup_Select.h" 

       comment out :   #include <User_Setup-std.h>            // Standard (single USB port)
  
      and uncomment:   #include <User_Setup-dual.h>           // Dual usb port
  
      close the Arduino IDE and re-open it to ensure the library will be re-compiled
