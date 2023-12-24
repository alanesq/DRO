# DRO

A super cheap Digital Readout (DRO) for lathes, milling machines etc. using a esp32 based "cheap yellow display" (AKA ESP32-2432S028R) and cheap eBay digital calipers.<br>
You can pick up the display modules on Aliexpress for around £12 and calipers around £6 each.

<br>
Note: This is a work in progress, whilst it is now just about in a usable state I have yet to install it myself or add the features I require.<br><br>

<br><img src="/pics/DROproject.jpg" /><br>

Misc info:<br>
Cheap Yellow Display:
- https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/
  
Communicating with the digital caliper: 
- http://electronoobs.com/eng_arduino_tut93.php <br>
-  http://wei48221.blogspot.com/2016/01/using-digital-caliper-for-digital-read_21.html  <br>
-  https://github.com/MakingStuffChannel/DigitalCalipers/blob/master/DigitalCalipers.ino <br>

<br>
Note: I use a couple of transistors to convert the 1.5v signals from the caliper to the 3.3v of the ESP32, this results in the signal levels being inverted so if you use some other method you will need to change this in my code.  The circuit diagram I used can be seen in the PCB folder.  To supply the lower voltage to power the caliper I just use a simple voltage divider as they require very little power.  I originally tried using a level shifter board but I don't think it liked the 1.5v?  I am just using two 1k resistors as a voltage divider to power the calipers and they seem ok with this.
<br><br>

An interesting thing I was not expecting is you get 2 decimal places even from the very cheap calipers which only display 1, as can be seen in the picture above.<br>
If just using one caliper then gpio 16 and 17 can be easily used but to find enough GPIO pins to read 3 calipers I had to get a bit creative by removing the onboard 3 colour LED and soldering wires directly on to the ESP32 module.  This is not as difficult as you might think, if you use a small soldering iron tip and thin wire it's pretty easy even for my uncoordinated soldering skills.
<br>
I am using the cheapest calipers I can find and they seem to work surprisingly well although you only get 160mm of travel.  You can buy calipers 200mm but these tend to be around £15, I have one on order and will post info. here when I receive it.  There is also the possibility of creating longer runners for these sensors which would be good if someone did this and sold them (hint hint ;-).   See: https://www.youtube.com/watch?v=JYnit_PSSMY

This sketch is wifi enabled just because I already had the code to do this so why not but this can be disabled by changing a flag in the settings if you do not require it.

