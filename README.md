# DRO - A super cheap Digital Readout (DRO) for lathes, milling machines etc. 

My homemade DRO consists of an ESP32-based "cheap yellow display" which costs around £12, some digital calipers that cost under £5 each plus a few transistors, wiring etc... As a result, building a three-axis DRO can set you back less than £30 (around $40).

<table><tr>
  <td><img src="/pics/DROproject.jpg" /></td>
  <td><img src="/pics/DROinstalled.jpg" /></td>
</tr>tr><tr></tr>
  <td><img src="/pics/DROparts.jpg" /></td>
  <td><img src="/pics/caliperInstalled.jpg" /></td>
</tr></table> 

I wanted a Digital Readout (DRO) for both my lathe and milling machine. However, the cheapest options available was still going to cost me several hundred pounds, which I just couldn’t justify spending.  I also suspected that the cheapest DRO on the market would probably fail after a short time anyway.  Therefore, I decided to see if I can create my own as inexpensively as possible. I think it safe to say that I have achieved this goal! <br>

WARNING: The well known problem with these cheap calipers is the battery goes flat if you do not use them for a while, it turns out that they are producing the data we use here all the time, even when turned off.  I bought one of the longer calipers recently which looks like a "new/impoved" design which has got round this issue by turning off the data all together which is very bad news if you want to use it for this project.  You can still use the reader from a cheaper caliper on this longer bar though.<br>

The cheap calipers only have a range of 150mm although longer 200mm or even 300mm are available on eBay for under £20, this is probably enough for my requirements although I have now ordered some circuit boards designed by "Limi DIY" to try, these promise to extend this range to 650mm.  These are pretty expensive by the standards of this project but still not too bad although I have yet to find out if they work ok for me. <br>
[YouTube video-extending the range](https://www.youtube.com/watch?v=JYnit_PSSMY) - [PCBway order link](https://www.pcbway.com/project/shareproject/Digital_Caliper_Hack_Mod_new_2021.html)

You can of course buy better quality calipers if you do not trust these cheap ones to be accurate or you can buy similar items from China which are actually designed to be used as a DRO but it soon starts to get expensive.  These cheap calipers really are amazing for the price, they are almost being given away.  Big Clive has made a nice video showing what is inside them and how to take them apart:
[Big Clive](https://www.youtube.com/watch?v=fKSSY1gzCEs) <br>

One interesting thing I was not expecting is you get two decimal places from these very cheap calipers even from the only display one on their LCD, as can be seen in the picture above.

Note: This is a work in progress, whilst I now have it installed on my milling machine (it works really well :-) it is early days and I want to add more features to it.

## Misc info:
[Cheap Yellow Display Information](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
  
Communicating with the digital caliper
- [electronoobs](http://electronoobs.com/eng_arduino_tut93.php)
- [wei48221](http://wei48221.blogspot.com/2016/01/using-digital-caliper-for-digital-read_21.html)
- [makingstuffchannel](https://github.com/MakingStuffChannel/DigitalCalipers/blob/master/DigitalCalipers.ino)

This sketch is wifi enabled just because I already had the code to do this so why not, this is turned off by default but can be turned on via the DROs menu.  You can enable wifi at startup by changing a flag in the settings section.  It is mainly useful for OTA updates but offers the option to add features at a later date. <br>
At present the display has zero buttons for each axis along with a divide by zero button plus buttons to switch to different display pages.  There is plenty of scope to add lots of other features later, I have an idea that it may be nice to have the option to enter a list of points via the web page then have the DRO guide me to each point? <br>
On page 3 of the display there is a number entry keypad which at present is not used, it stores the entered number in to a float and a String.

If you have a 3D printer this is a case I have modified to be large enough to contain all the electronics: [custom case](https://github.com/alanesq/DRO/tree/main/case) <br>
Also a replacement case for the digital calipers: [caliper case](https://github.com/alanesq/DRO/tree/main/caliperCase)

Current Wifi Features:
- Simulate pressing the screen with: http://x.x.x.x/touch?x=100&y=50
- Restart esp32 with: http://x.x.x.x/reboot
- Check it is live with: http://x.x.x.x/ping
- Log of activity: http://x.x.x.x/log
- Testing code: see 'handleTest' at bottom of page, access with http://x.x.x.x/test
- Update via OTA: http://x.x.x.x/ota

--------------------------------------

If you only requiring X and Y you could use connector CN1 (GPIO 22 and 27) and connected to P1 (GPIO 1 and 3), 'serialDebug' would need to be set to 0 and you would need to disconnect the P1 caliper to program the CYD. <br> 
To find enough GPIO pins to read 3 calipers I had to get a bit more creative; I removed the onboard 3 colour LED and then soldered some ribbon wire directly on to one side of the ESP32 module it's self.  This is not as difficult as you might imagine, if you use a small soldering iron tip and magnification it was pretty easy even with my uncoordinated soldering skills.  Tin the pins first ideally using some extra flux, then attach the wires and use some hot melt glue to hold the ribbon wire in place.  I soldered 8 wires to the ESP module plus two to a nearby connector for the 3.3v power.
 - [GPIO pins to use](https://github.com/alanesq/DRO/blob/main/pics/CYD-gpioPins.jpg)
 - [Image](https://github.com/alanesq/DRO/blob/main/PCB/ribbonCablePins.jpeg) <br>

The wiring is not as complex as it might look, it is just a simple transistor on each data pin to convert from 1.5v up to 3.3v for the esp32 to be able to read, I use my CNC router to create the circuit boards but this could be easily built on some strip board etc.  I tried using a level shifter board at first but I don't think it worked very well with the 1.5v levels. <br>
Note: the transistors to convert the 1.5v signals from the caliper results in the signal levels being inverted so if you use some other method you will need to change 'invertCaliperDataSignals' to 0.  The circuit diagram I used can be seen in the PCB folder along with the files I used to create the circuit boards (Fritzing and CNC router).  To supply the lower voltage to power the caliper I just use a simple voltage divider 

<img src="/PCB/circuit.png" />

Note: I am using GPIO0 which isn't ideal as it is also attached to the onboard button, but if you do not install the pullup resistor and use a lower value resistor on the input this seems to work fine (although it can stop the hardware reset working when programming the CYD).

There are connections for GPIO1 and 3 which are the serial pins but could be used as general GPIO if required (it can be programmed by OTA so this would not be a big problem).

--------------------------------------

## Adnl information:

In an attempt to try and make the calipers more robust I have put some insulation tape over the circuit board (where the slider runs along it) as I think the bare circuit board is very vulnerable to damp entering and corroding the board.  [Picture](https://github.com/alanesq/DRO/blob/main/pics/caliperMod.jpg) <br>
It may also be an idea to coat the rest of the circuit board in some kind of conformal coating (clear nail varnish?) and maybe some kind of dust seal around the slider where it enters the reader (I believe some come with one fitted).
<br>One advantage of having the remote display is it will make it easier to fit the calipers out of harms way as you do not need to see the display on them.

I have found that many USB power supplies upset the calipers resulting in the readings being very random or steadily increasing for no apparent reason, I am guessing the switching frequency of the switch mode power supply is the cause of this issue.  In the end I built a linear power supply rather than use a switch mode one which is probably a good idea anyway although another option which may work if you have this issue is use a linear regulator on a 5v switch mode power supply to provide 3.3v and power it from this?

At present it is set up to have 5 display pages, buttons can be created and assigned to the pages in the "define the button widgets" section of the sketch.  The caliper readings are displayed on all pages but there are two sizes available (this size is set in "displayReadings").  Button actions are defined in the "buttons.h" file.

Custom actions for the pages can be added to the "pageSpecificOperations()" procedure e.g. see "if page 3 show number entered on keypad".

One possible issue I anticipate is that the calipers often have a random reading when first powered on, this could be a problem if it is close to the limit of the caliper if you then move it past this limit (the limit looks to be 999.99mm on mine).  It can of course be solved simply by pressing the zero button on the caliper but this is something I may look in to later.  I believe the caliper can be zeroed by taking one of the data pins to ground but this would require extra circuitry.

## Contact
If you use this sketch please let me know how you get on  - alanesq@disroot.org

