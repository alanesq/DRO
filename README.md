# DRO

Note: This is a work in progress project, not yet fully working<br><br>
A super cheap Digital Readout (DRO) for lathes, milling machines etc. using a esp32 based "cheap yellow display" (AKA ESP32-2432S028R) and cheap eBay digital calipers.<br>
You can pick up the display modules on Aliexpress for around £12 and calipers around £6 each.<br><br>

Misc info:<br>
Cheap Yellow Display:
- https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/
  
Communicating with the digital caliper: 
- http://electronoobs.com/eng_arduino_tut93.php <br>
-  http://wei48221.blogspot.com/2016/01/using-digital-caliper-for-digital-read_21.html  <br>
-  https://github.com/MakingStuffChannel/DigitalCalipers/blob/master/DigitalCalipers.ino <br>

<br>
Note: I use a couple of transistors to convert the 1.5v signals from the caliper to the 3.3v of the ESP32, this results in the signal levels being inverted so if you use some other method you will need to change these in my code.  The circuit diagram I used can be seen in the PCB folder.  To supply the lower voltage to power the caliper I just use a simple voltage divider as they require very little power.<br>

<br><br><img src="/pics/DROproject.jpg" /><br>
