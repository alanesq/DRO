# DRO
A super cheap Digital Readout (DRO) for lathes, milling machines etc. 
----------------------------------

<br>I wanted a Digital Readout (DRO) for both my lathe and milling machine. However, the cheapest options available from China were still going to be several hundred pounds each, which I just couldn’t justify spending. Therefore, I decided to see if I can create my own as inexpensively as possible.
I think it safe to say that I achieved this goal! 
<br>My homemade DRO consists of an ESP32-based "cheap yellow display," which costs around £12, and a pair of digital calipers that cost £6 each. The remaining expenses include wiring, a few resistors, transistors etc.. As a result, building a two-axis DRO will set you back less than £30 (around $40).

The cheap calipers only have a range of 150mm although longer 200mm are available but around twice the price, this is probably enough for my requirements although I have now ordered some circuit boards to try which promise to extend this range to 650mm (although this does blow my budget somewhat ;-).

Note: This is a work in progress, whilst it is now in a usable state I have yet to install it myself or add the features I require.

<img src="/pics/DROproject.jpg" />

--------------------------------------

Misc info:

Cheap Yellow Display:
- https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/
  
Communicating with the digital caliper: 
- http://electronoobs.com/eng_arduino_tut93.php 
- http://wei48221.blogspot.com/2016/01/using-digital-caliper-for-digital-read_21.html  <br>
- https://github.com/MakingStuffChannel/DigitalCalipers/blob/master/DigitalCalipers.ino <br>

<br>
An interesting thing I was not expecting is you get 2 decimal places even from the very cheap calipers which only display 1, as can be seen in the picture above.<br>

<br>As stated above, I have also ordered some custom circuit boards which should allow for much longer travel and will post here how this goes.
<br>https://www.youtube.com/watch?v=JYnit_PSSMY                  
https://www.pcbway.com/project/shareproject/Digital_Caliper_Hack_Mod_new_2021.html       

This sketch is wifi enabled just because I already had the code to do this so why not but this can be disabled by changing a flag in the settings if you do not require it.  It is mainly useful for OTA updates but it would be easy to add features to this later.

If you have a 3D printer this is a case I have modified to be large enough to contain all the electronics: https://github.com/alanesq/DRO/tree/main/case
It prints without support (face down) and takes around 2hrs on my printer.

--------------------------------------

Wifi Features:
- Simulate pressing the screen with: http://x.x.x.x/touch?x=100&y=50
- Restart esp32 with: http://x.x.x.x/reboot
- Check it is live with: http://x.x.x.x/ping
- Log of activity: http://x.x.x.x/log
- Testing code: see 'handleTest' at bottom of page, access with http://x.x.x.x/test
- Update via OTA: http://x.x.x.x/ota

--------------------------------------

If just using one caliper then gpio 16 and 17 can be easily used but to find enough GPIO pins to read 3 calipers I had to get a bit creative by removing the onboard 3 colour LED and soldering wires directly on to the ESP32 module.  This is not as difficult as you might think, if you use a small soldering iron tip and thin wire it's pretty easy even for my uncoordinated soldering skills.
<br>I soldered a 10 wire ribbon cable to one side of the ESP32 module then run this to my custom PCB.
The wiring is not as complex as it might look, it is just a simple transistor on each data pin to convert from 1.5v up to 3.3v for the esp32 to be able to read, I use my CNC router to create the circuit boards but this could be easily built on some strip board etc. <br>
Note: the transistors to convert the 1.5v signals from the caliper results in the signal levels being inverted so if you use some other method you will need to change 'invertCaliperDataSignals' to 0.  The circuit diagram I used can be seen in the PCB folder along with the files I used to create the circuit boards (Fritzing and CNC router).  To supply the lower voltage to power the caliper I just use a simple voltage divider 
as they require very little power.  I originally tried using a level shifter board but I don't think it liked the 1.5v?  I am just using two 100 ohm resistors as a voltage divider to power the calipers and they seem ok with this.<br><br>

<img src="/PCB/circuit.png" />
Note: I am using GPIO0 which isn't ideal as it is also attached to the onboard button, but if you do not install the pullup resistor and use a lower value resistor on the input this seems to work fine (although it can stop the hardware reset working when programming the CYD).
<br>There are connections for GPIO1 and 3 which are the serial pins but could be used as general GPIO if required (it can be programmed by OTA so this would not be a big problem).

--------------------------------------

Misc bits of information:

In an attempt to try and make the calipers more robust I have put some insulation tape over the circuit board (where the slider runs along it) as I think the bare circuit board is very volnurable to damp entering and corroding the board.
See pic here:    [https://github.com/alanesq/DRO/pics/caliperMod.jpg](https://github.com/alanesq/DRO/blob/main/pics/caliperMod.jpg)
It may also be an idea to coat the rest of the circuit board in some kind of conformal coating (clear nail varnish?) and maybe some kind of dust seal around the slider where it enters the reader?
<br>One advantage of having the remote display is it will make it easier to fit the calipers out of harms way as you do not need to see the display on them.

I have found that some of my plug in USB power supplies upset the calipers resulting in the readings being random or steadily increasing for no reason.  I suspect a decent sized capacitor on the power feed to the CYD will be a very good idea.

At present it is set up to have 5 display pages, buttons can be created and assigned to the pages in the "define the button widgets" section of the sketch.  The caliper readings are displayed on all pages but there are two sizes available (this size is set in "displayReadings").  Button actions are defined in the "buttons.h" file.
<br>Custom actions for the pages can be added to the "rawScreen" procedure e.g. see "if page 3 show number entered on keypad".

If you are planning to try my sketch let me know first (alanesq@disroot.org) as I will probably have a more recent version I can upload for you.



