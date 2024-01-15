# DRO - A super cheap Digital Readout (DRO) for lathes, milling machines etc. 

<br>My homemade ESP32 based DRO which conists of:
 - [Cheap Yellow Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) &ensp; (A.K.A. ESP32-2432S028R) which costs around £12 from Aliexpress
 - Digital calipers which cost under £5 each on eBay or Aliexpress
 - Plus a few transistors, general wiring etc... 

As a result, building a three-axis DRO for your milling machine or lathe can set you back less than £30 (around $40). <br><br>

<p align="center" width="100%">
  <img src="/pics/pics.jpg" /><br><br>
  <img src="/pics/screenGrabs.jpg" />
  <img width="450" class="center" src="/pics/webpage.jpg" /><br><br>
</p>

I wanted a Digital Readout (DRO) for both my lathe and milling machine. However, the cheapest options available was still going to cost me several hundred pounds, which I just couldn’t justify spending. I also suspected that the cheapest DRO on the market would probably fail after a short time anyway ging on past experience of cheap electronics.  Therefore, I decided to see if I can create my own as inexpensively as possible. <br>
I think it safe to say that I have achieved this goal ! <br><br>

## Patterns

Rather than a complicated hole pattern facilities on this DRO (for drilling a circle of holes etc.) I decided it would be easier and more versatile to enter the coordinates of the positions required via the web page, these coordinates can then be steped through from the DRO screen (page 4).  <br>
The coordinates are ented as a simple list in the format:<br>
```
x0 y0
x0 y10
x10 y0
x10 y10
```
It also has the option to paste simple gcode, this gives the ability to create your own bolt hole patterns etc. using whatever software you preffer (A good choice would be Inkscape and [Krabzcam](https://github.com/mkrabset/krabzcam) ) or even easier you can use online tools for creating them &ensp; e.g. [online gcode tools](https://www.intuwiz.com/drilling.html) <br><br>

## Digital calipers

The cheap calipers only have a range of 150mm although longer 200mm or even 300mm are available on eBay for under £20, this is probably enough for my requirements although I have had some longer PCBs created which were designed by "Limi DIY" and they appear to work and extend the range to around 400mm.  There is the possibility to link these together although the join would need to be very accurate.  I had 5 lengths made which came in at £10 each so they are not cheap by the standards of this project but not as expensive as you may think. <br> 
&ensp; &ensp; [extending the range](https://hackaday.com/2021/08/05/custom-caliper-tracks-for-when-youre-going-the-distance/) &ensp; - &ensp;
[PCBway order link](https://www.pcbway.com/project/shareproject/Digital_Caliper_Hack_Mod_new_2021.html)

You can of course buy better quality calipers if you do not trust these cheap ones to be accurate or you can buy similar items from China which are actually designed to be used as a DRO, but it soon starts to get expensive; These cheap calipers really are amazing for the price, they are so cheap that it feels like they are almost being given away and appear to be very accurate.  Also, I find the cheaper ones with a plastic slider are better than metal ones as you do not get issues with the metal contacting the metal of your machine which can cause some issues.  Big Clive has made a nice video showing what is inside them and how to take them apart: &ensp; [Big Clive](https://www.youtube.com/watch?v=fKSSY1gzCEs) <br>

One good thing that I was not expecting is you still get two decimal places from these very cheap calipers even though they only display one on their LCD, as can be seen in the pictures above. <br><br>

## Warnings!

The well known problem with these cheap calipers is the battery goes flat if you do not use them for a while, it turns out that they are producing the data we use here all the time, even when turned off.  I bought one of the longer calipers recently which looks like it is a "new/impoved" design which has got round this issue by turning off the data all together which is very bad news if you want to use it for this project.  So if you are buying calipers I suggest you try to buy the older style ones if you can as the newer ones do not seem to support data, the good news is these will be the cheapest ones.  You can still use the reader from a cheaper caliper on this longer bar if you do have one though.<br>
Also, I understand that some of these calipers use a different data standard although all the ones I have tried are the same<br>

Make sure to copy the CYD custom UserSetup.h file into the TFT_eSPI library. &ensp; See: [setup instructions](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md) <br>
 
Make sure your Cheap Yellow Display is not the version with two USB ports as this has a different display (I am looking in to supporting both)<br><br>
 
## Misc info:

Brian Lough's information on the [Cheap Yellow Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
  
Communicating with the digital calipers
- [Martin's Creations](https://sites.google.com/site/marthalprojects/home/arduino/arduino-reads-digital-caliper)
- [Robot Room](https://www.robotroom.com/Caliper-Digital-Data-Port.html) - includes some info on different data protocols 
- [electronoobs](http://electronoobs.com/eng_arduino_tut93.php)
- [wei48221](http://wei48221.blogspot.com/2016/01/using-digital-caliper-for-digital-read_21.html)

This sketch is wifi enabled, this is turned off by default but can be turned on via the DROs menu (You can enable wifi at startup by changing a flag in the settings section).  It is useful for OTA updates and for entering drill hole patterns etc.  BTW - I have been very surprised at the range I get from it's wifi.<br>
At present the display has zero buttons for each axis along with a divide by zero button plus buttons to switch to different display pages.  There is plenty of scope to add lots of other features later, I have an idea that it may be nice to have the option to enter a list of points via the web page then have the DRO guide me to each point? <br>
On page 3 of the display there is a number entry keypad which at present is only used to enter a number to jump to on page 4 (by pressing KBD) but it has the potential for other uses, it stores the entered number in to a float and a String.<br>
Note: The first time you start wifi it will create a wifi hotspot which you can connect to in order to enter your local wifi login details.

If you have a 3D printer this is a case I have modified to be large enough to contain all the electronics: &ensp; [custom case](https://github.com/alanesq/DRO/tree/main/case) <br>
Also a replacement case for the digital calipers: &ensp; [caliper case](https://github.com/alanesq/DRO/tree/main/caliperCase)

Current Wifi Features:
- Simulate pressing the screen with: http://x.x.x.x/touch?x=100&y=50
- Restart esp32 with: http://x.x.x.x/reboot
- Check it is live with: http://x.x.x.x/ping
- Log of activity: http://x.x.x.x/log
- Testing code: see 'handleTest' at bottom of page, access with http://x.x.x.x/test
- Update via OTA: http://x.x.x.x/ota

If just connecting one caliper you could use (GPIO 22 and 27) via the CYD connectors but to find enough GPIO pins to read 3 calipers I had to get a bit more creative; I removed the onboard 3 colour LED and then soldered some ribbon wire directly on to one side of the ESP32 module it's self.  This is not as difficult as you might imagine, if you use a small soldering iron tip and magnification it was pretty easy even with my uncoordinated soldering skills.  Tin the pins first ideally using some extra flux, then attach the wires and use some hot melt glue to hold the ribbon wire in place.  I soldered 8 wires to the ESP module plus two to a nearby connector for the 3.3v power.
 - [GPIO pins to use](https://github.com/alanesq/DRO/blob/main/pics/CYD-gpioPins.jpg)
 - [Image](https://github.com/alanesq/DRO/blob/main/PCB/ribbonCablePins.jpeg) <br>

The wiring is not as complex as it might look, it is just a simple transistor on each data pin to convert from 1.5v up to 3.3v for the esp32 to be able to read, I use my CNC router to create the circuit boards but this could be easily built on some strip board etc.  I tried using a level shifter board at first but I don't think it worked very well with the 1.5v levels. <br>
Note: the transistors to convert the 1.5v signals from the caliper results in the signal levels being inverted so if you use some other method you will need to change 'invertCaliperDataSignals' to 0.  The circuit diagram I used can be seen in the PCB folder along with the files I used to create the circuit boards (Fritzing and CNC router).  To supply the lower voltage to power the caliper I just use a simple voltage divider 

<img width="50%" src="/PCB/trans.jpg"/><br>
<img src="/PCB/circuit.jpg"/>

Note: I am using GPIO0 which isn't ideal as it is also attached to the onboard button, but if you do not install the pull-up resistor and use a lower value resistor on the input this seems to work fine (although it can stop the hardware reset working when programming the CYD).

There are connections for GPIO1 and 3 which are the serial pins but could be used as general GPIO if required (it can be programmed by OTA so this would not be a big problem).

In an attempt to try and make the calipers more robust I have put some insulation tape over the circuit board (where the slider runs along it) as I think the bare circuit board is very vulnerable to damp entering and corroding the board. &ensp; [Picture](https://github.com/alanesq/DRO/blob/main/pics/caliperMod.jpg) <br>
It may also be an idea to coat the rest of the circuit board in some kind of conformal coating (clear nail varnish?) and maybe some kind of dust seal around the slider where it enters the reader (I believe some come with one fitted).
<br>One advantage of having the remote display is it will make it easier to fit the calipers out of harms way as you do not need to see the display on them.

At present it is set up to have 4 display pages, buttons can be created and assigned to the pages in the "define the button widgets" section of the sketch.  The caliper readings are displayed on all pages but there are two sizes available (this size is set in "displayReadings").  Button actions are defined in the "buttons.h" file.  It has a demo keypad for entering numbers but it is not yet used for anything.  I plan to add more features later on.  Let me know if there are features you would like me to add? <br><br>

On page 2 there is the option to store the current readings and recall them.  This is stored in eeprom so can be recalled even after the device has been turned off.

## Troubleshooting

I have found that the power supply is critical for the calipers in that most switch mode USB power supplies seem to cause them to give random readings.  Many USB power supples will cause the calipers to have randomly changing displays or for start counting up for no apparent reason etc., at best the caliper readings will not be very stable.  I built a linear power supply but even this was not perfect and othjer appliances on the mains would cause the readings to become unstable. <br>
I think by far the best way to power this would be from a battery as this will give a very stable power and be completely isolated from any mains spikes etc..  I have used an old PC power supply for mine which is the next best thing.  A USB power bank seems to work ok also.  Another option may be to power the CYD from a usb power supply but power the calipers from a AA battery? <br>
The displays can vary by 100th of a mm when stable but any more than this can be either noise on the power supply or I find the calipers with a metal slider can cause this.  I understand this is from issues with it making contact with the metal of the machine it is attached to so it needs to be insulated at the mounting points, I get round this by using the plastic ones.

If you find the CYD reboots as soon as a button is pressed this is probably becuase the User_Setup.h file has not been copied over for the "TFT_eSPI" library [see here](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md)
Note: I have had issues where I change something in the library settings and even when I change it back it no longer works.  This is becuase the Arduino IDE doesn't re-compile everything, every time so to get it to re-compile I changed the dev board to something else then back again (just closing and re-opening the IDE may have worked?)

One possible issue I anticipate is that the calipers often have a random reading when first powered on, this could be a problem if it is close to the limit of the caliper if you then move it past this limit (the limit looks to be 9999.99mm on mine?).  It can of course be solved simply by pressing the zero button on the caliper but this is something I may look in to later.  I believe the caliper can be zeroed by taking one of the data pins to ground but this would require extra circuitry.

I have issues sometimes with interference from other electrical devices, for some odd reason it usually seems to cause the caliper to jump up or down by exactly 5mm.  I have tried all sorts of things to try and improve this but so far nothing seems to make much difference.  I believe it is R.F. intererence on the mains earth is the issue so I am still experimenting.  
As a temp. fix I have added a hold button to the main screen (H), this stores the current readings, waits a few seconds then restores them.  This can be pressed when interference is expected so that any change in readings are ignored.

--------------------------------------

## Contact

If you use this sketch please let me know how you get on and if you have any suggestions for improvments etc.  - alanesq@disroot.org

