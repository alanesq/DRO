# DRO

A super cheap Digital Readout (DRO) for lathes, milling machines etc. using a esp32 based "cheap yellow display" (AKA ESP32-2432S028R) and cheap eBay digital calipers.

You can pick up the display modules on Aliexpress for around £12 and calipers around £6 each.

Note: This is a work in progress, whilst it is now just about in a usable state I have yet to install it myself or add the features I require.

<img src="/pics/DROproject.jpg" />

--------------------------------------

Misc info:

If you get the error "class TFT_eSPI' has no member named 'getTouch'"<br>
  edit User_Setup.h in the TFT_eSPI library folder and add the line:   #define TOUCH_CS PIN_D2      // Chip select pin (T_CS) of touch screen 

Cheap Yellow Display:
- https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/
  
Communicating with the digital caliper: 
- http://electronoobs.com/eng_arduino_tut93.php 
- http://wei48221.blogspot.com/2016/01/using-digital-caliper-for-digital-read_21.html  <br>
- https://github.com/MakingStuffChannel/DigitalCalipers/blob/master/DigitalCalipers.ino <br>

<br>
An interesting thing I was not expecting is you get 2 decimal places even from the very cheap calipers which only display 1, as can be seen in the picture above.<br>

If just using one caliper then gpio 16 and 17 can be easily used but to find enough GPIO pins to read 3 calipers I had to get a bit creative by removing the onboard 3 colour LED and soldering wires directly on to the ESP32 module.  This is not as difficult as you might think, if you use a small soldering iron tip and thin wire it's pretty easy even for my uncoordinated soldering skills.
The wiring is not as complex as it might look, it is just a simple transistor on each data pin to convert from 1.5v up to 3.3v for the esp32 to be able to read, I use my CNC router to create the circuit boards but this could be easily built on some strip board etc. <br>
Note: the transistors to convert the 1.5v signals from the caliper results in the signal levels being inverted so if you use some other method you will need to change 'invertCaliperDataSignals' to 0.  The circuit diagram I used can be seen in the PCB folder along with the files I used to create the circuit boards.  To supply the lower voltage to power the caliper I just use a simple voltage divider 
as they require very little power.  I originally tried using a level shifter board but I don't think it liked the 1.5v?  I am just using two 100 ohm resistors as a voltage divider to power the calipers and they seem ok with this.<br><br>

I am using the cheapest calipers I can find and they seem to work surprisingly well although you only get 160mm of travel.  You can buy calipers 200mm but these tend to be around £15, I have one on order and will post info. here when I receive it.  
I have also ordered some custom circuit boards which should allow for much longer travel and will post here how this goes.
<br>https://www.youtube.com/watch?v=JYnit_PSSMY                  
https://www.pcbway.com/project/shareproject/Digital_Caliper_Hack_Mod_new_2021.html       

This sketch is wifi enabled just because I already had the code to do this so why not but this can be disabled by changing a flag in the settings if you do not require it.

If you have a 3D printer this is a very nice little case for the CYD: https://www.printables.com/model/685845-enclosure-for-cheap-yellow-display-esp32-2432s028r
<br>I filed a slot on one side to let a small ribbon cable come through carrying the GPIO signals.

--------------------------------------

Features:
- Simulate pressing the screen with: http://x.x.x.x/touch?x=100&y=50
- Restart esp32 with: http://x.x.x.x/reboot
- Check it is live with: http://x.x.x.x/ping
- Log of activity: http://x.x.x.x/log
- Testing code: see 'handleTest' at bottom of page, access with http://x.x.x.x/test
- Update via OTA: http://x.x.x.x/ota

--------------------------------------

<img src="/PCB/circuit.png" />

