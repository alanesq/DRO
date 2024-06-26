# DRO - A super cheap Digital Readout (DRO) for lathes, milling machines etc. 

<br>My homemade ESP32 based DRO which conists of:
 - [Cheap Yellow Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) &ensp; (A.K.A. ESP32-2432S028R) which costs around £12 from Aliexpress
 - Digital calipers which cost under £2.50 each on Aliexpress 
 - Plus a few transistors, general wiring etc... 

As a result, building a three-axis DRO for your milling machine or lathe can set you back less than £25 (around $30). <br><br>

<p align="center" width="100%">
  <img height="200px" src="/pics/pics.jpg" /><br><br>
  <img height="200px" src="/pics/screenGrabs.jpg" />
  <img height="200px" class="center" src="/pics/webpage.jpg" /><br><br>
</p>

I wanted a Digital Readout (DRO) for both my lathe and milling machine. However, the cheapest options available was still going to cost me several hundred pounds, which I just couldn’t justify spending. I also suspected that the cheapest DRO on the market would probably fail after a short time anyway going on my past experience of cheap electronics.  Therefore, I decided to see if I can create my own as inexpensively as possible.  I think it safe to say that I have achieved this goal :-)<br>
Not only did this save me spending a lot of money when I do not even know how much I would use one but it means I can modify it to my exact requirements and it will always be easily fixable.<br><br>

The libraries required for this sketch are detailed [HERE](https://github.com/alanesq/DRO/tree/main/LIBRARIES)<br>
Note that the "TFT_eSPI" library needs to be modified to work with the CYD and if your CYD has two USB ports there is a further change to this required.

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
[PCBway order link](https://www.pcbway.com/project/shareproject/Digital_Caliper_Hack_Mod_new_2021.html) <br>

Especially on my lathe I found that I actually did not need to have a long caliper as the work I require to measure tends to be in a small area so I have just attached a short caliper body to the machine but the slider is attached via a magnet, so if I extend beyond the calipers reach it simply detatches and then re-attaches when it returns.  This works really well and made installing the DRO very simple.  I also have the third caliper on my lathe fully attached with magnets and have two locations I can choose to attach it.  [see pic here](https://github.com/alanesq/DRO/blob/main/pics/lathe1.jpg)<br>

These cheap calipers really are amazing for the price, in fact they are so cheap that it feels like they are almost being given away, also, I find the cheaper ones with a plastic slider are better than metal ones in this application as you do not get issues with the metal contacting the metal of your machine which can cause issues.  Big Clive has made a nice video showing what is inside them and how to take them apart etc.: &ensp; [Big Clive](https://www.youtube.com/watch?v=fKSSY1gzCEs) <br>
I have read that because of interference issues this type caliper is not at all suitable to use as a DRO and I was starting to agree as I had a lot of trouble at first with this but I eventually found the solution (see bottom of page for more info.).<br>
One good thing that I was not expecting is you still get two decimal places from these very cheap calipers even though they only display one on their LCD, as can be seen in the pictures above. <br>

The most recent calipers i have purchased were:   [eBay £5.40 each](https://www.ebay.co.uk/sch/i.html?_dkr=1&iconV2Request=true&_blrs=recall_filtering&_ssn=bestsale-22&store_name=discountdeals111&_oac=1&_nkw=6%22%20vernier%20caliper%20internal%20%26%20external%20measure%20tool%20gauge%20rule%20slide%20gauge%20150m) , [Aliexpress £10 for 4](https://www.aliexpress.com/item/1005005699974701.html) <br><br>   


## Warnings!

The well known problem with these cheap calipers is the battery goes flat if you do not use them for a while, it turns out that they are producing the data we use here all the time, even when turned off.  I bought one of the longer calipers recently which looks like it is a "new/impoved" design which has got round this issue by turning off the data all together which is very bad news if you want to use it for this project.  So if you are buying calipers I suggest you try to buy the older style ones if you can as the newer ones do not seem to support data, the good news is these will be the cheapest ones.  You can still use the reader from a cheaper caliper on this longer bar if you do have one though.  I understand that some of these calipers use a different data standard although all the ones I have tried are the same.  The calipers would not cope well to getting wet so you will need to cover them well if this is likely to happen.


## Misc info:

I have recently given one of my DROs to the most excellent Youtuber Steamhead - You can see it [HERE](https://youtu.be/Z2Sz5ipYt7Y?si=eyIY_YAoHz-dP2uL&t=618)<br>

Brian Lough's information on the [Cheap Yellow Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
  
Communicating with the digital calipers
- [Martin's Creations](https://sites.google.com/site/marthalprojects/home/arduino/arduino-reads-digital-caliper)
- [Robot Room](https://www.robotroom.com/Caliper-Digital-Data-Port.html) - includes some info on different data protocols 
- [electronoobs](http://electronoobs.com/eng_arduino_tut93.php)
- [wei48221](http://wei48221.blogspot.com/2016/01/using-digital-caliper-for-digital-read_21.html)

This sketch is wifi enabled, this is turned off by default but can be turned on via the DROs menu (You can enable wifi at startup by changing a flag in settings.h).  It is useful for OTA updates and for entering drill hole patterns etc.  BTW - I have been very surprised at the range I get from it's wifi.<br>
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
 - [GPIO pins available to use](https://github.com/alanesq/DRO/blob/main/pics/CYD-gpioPins.jpg)
 - [GPIO pins I used](https://github.com/alanesq/DRO/blob/main/PCB/ribbonCablePins.jpeg) <br>

The wiring is not as complex as it might look, it is just a simple transistor on each data pin to convert from 1.5v up to 3.3v for the esp32 to be able to read, I use my CNC router to create the circuit boards but this could be easily built on some strip board etc.  I tried using a level shifter board at first but I don't think it worked very well with the 1.5v levels. <br>
Note: the transistors to convert the 1.5v signals from the caliper results in the signal levels being inverted so if you use some other method you will need to change 'invertCaliperDataSignals' to 0.  The circuit diagram I used can be seen in the PCB folder along with the files I used to create the circuit boards (Fritzing and CNC router).  To supply the lower voltage to power the caliper I just use a simple voltage divider although after reading in several places what a bad idea this was I tried using a string of diodes instead but this caused the calipers to become very unstable so I converted back.<br>
The best way I found to wire the calipers is to use shielded cable.  I connect the shield and negative power together at the interface end of the wire then at the caliper end I do not connect the sheild to anything and connect the negative wire to the metal of the lathe/mill (i.e. mains earth).  When connected this way I find the displays very stable, only fluctuating by 100th mm at most.<br>

<img width="50%" src="/PCB/trans.jpg"/><br>
<img src="/PCB/circuit.jpg"/>

Note: I am using GPIO0 which isn't ideal as it is also attached to the onboard button, but if you do not install the pull-up resistor and use a lower value resistor on the input this seems to work fine (although it can stop the hardware reset working when programming the CYD).

There are connections for GPIO1 and 3 which are the serial pins but could possibly be used as general GPIO if required? (it can be programmed by OTA so this would not be a big problem).

In an attempt to try and make the calipers more robust I have put some insulation tape over the circuit board (where the slider runs along it) as I think the bare circuit board is very vulnerable to damp entering and corroding the board. &ensp; [Picture](https://github.com/alanesq/DRO/blob/main/pics/caliperMod.jpg) <br>
It may also be an idea to coat the rest of the circuit board in some kind of conformal coating (clear nail varnish?) and maybe some kind of dust seal around the slider where it enters the reader (I believe some come with one fitted).
<br>One advantage of having the remote display is it will make it easier to fit the calipers out of harms way as you do not need to see the display on them.

At present it is set up to have 4 display pages, buttons can be created and assigned to the pages in the "define the button widgets" section of the sketch.  The caliper readings are displayed on all pages but there are two sizes available (this size is set in "displayReadings").  Button actions are defined in the "buttons.h" file.  It has a demo keypad for entering numbers but it is not yet used for anything.  I plan to add more features later on.  Let me know if there are features you would like me to add? <br><br>

On page 2 there is the option to store the current readings and recall them.  This is stored in eeprom so can be recalled even after the device has been turned off.  <br>
On page 1 there is a hold button "H" which I created when I was having trouble with interference causing the displays to jump about, this reads the current values, waits a while then restores them, I have left it as it may prove of some use one day?

On my lathe I have a stop which I can attach to make sure I don't take the carriage too far, as a temp. test I attached a caliper to this but it actually worked out so well I have kept it this way.  The end of the calipers slider attaches with a strong magnet so if I take the carriage too far it just detaches and re-attaches when it returns. [picture](/pics/lathe1.jpg) <br>

## Troubleshooting

Not enough memory when compiling:  If using the latest esp32 board drivers it seems to require a lot more memory than the older ones, if you set the partition scheme to "Minimal spiffs" this should resolve the issue.

If you find the caliper readings are unstable, jump about (often in 5mm steps) I have found that the power supply is important for the calipers in that most switch mode USB power supplies seem to cause them to give random readings which jump about and are completely unstable.  I also had a lot of trouble with switching the lathe on/off would cause the caliper readings to jump.  A linear power supply is probably a good idea but I used a PC power supply for mine in the end although I have since discovered that connecting the negative power feed to the calipers to mains earth  i.e. the metal of the mill/lathe (near to the caliper if possible) is by far the most important thing, this makes a massive difference.  Also a big capacitor on the power feed to the calipers helps a lot (I used a 3000uF).  I had considered using a linear regulator to power the CYD from the 5v USB to cut out the switched mode power supply noise but the CYD has two 3.3v regulators onboard so i don't think you can easily power it from 3.3v

I sometimes find that if the DRO has not been used for a while the calipers don't seem to power up in the correct state which can result in one or more of them not responding, if you turn the DRO off for a few seconds and back on this usually resolves the issue.  I guess they are not designed to have the battery removed, this could be resolved by giving the DRO control of the power to the calipers so it can power cycle them at startup, in fact they draw so little power that a gpio pin could probably power them directly (via voltage divider).  Another option which may be worth considering is just power the calipers from a AA battery and leave them powered all the time, they are designed to run from a watch battery so it should last a reasonable time and this would also avoid any issues with power supplies.

If you enable showing of caliper read errors this can help decide if there is a problem with one of the calipers.  I find that if running it from a standard USB power supply you can get a lot of read errors although it stilll functions ok, on mine which are running from an old PC power supply I get pretty much no read errors at all.

If the screen colours seem very odd or if you find the CYD reboots as soon as a button is pressed this is probably becuase the User_Setup.h file has not been copied over for the "TFT_eSPI" library [see here](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md)
Note: I have had issues where I change something in the library settings and even when I change it back it no longer works.  This is becuase the Arduino IDE doesn't re-compile everything, every time so to get it to re-compile I close the Arduino IDE and re-open it after the change.

One possible issue I anticipate is that the calipers often have a random reading when first powered on, this could be a problem if it is close to the limit of the caliper if you then move it past this limit (the limit looks to be 9999.99mm on mine?).  It can of course be solved simply by pressing the zero button on the caliper but this is something I may look in to later.  I believe the caliper can be zeroed by taking one of the data pins to ground but this would require extra circuitry.

I have had a lot of issues with interference from the motor switching on/off on my lathe and to lesser extent my mill. This interference usually seems to cause the caliper to jump up or down in 5mm steps, this it turns out is a common issue when using this type caliper. 
I tried all sorts of things to try and improve this but after a lot of experimentation and head scratching I finally discovered that in my case at least the solution was to use shielded cable from the esp32 interface to the calipers with the shield connected to the GND (Esp32 end).  At the caliper end I connect the shield to the metal of the lathe/mill (i.e. mains earth).  I do not know if this is a "good idea" electronics wise but the result is almost spectacular with a rock solid reading even when turning the motor on and off repeatedly :-)

--------------------------------------

## Contact

If you use this sketch please let me know how you get on and if you have any suggestions for improvments etc.  - alanesq@disroot.org

<br><img width="90%" src="/pics/lathe1.jpg" /><br>
