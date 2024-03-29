#PCB to interface the CYD to the digital calipers

<img src="circuit.jpg" /> <br>
<img width="50%" src="trans.jpg" />

The calipers operate at 1.5v so this is required to convert this to the 3.3v of the CYD.  It is a very simple circuit which uses a single transistor per data pin, I used my CNC router to cut a PCB but you could easily just build this on some strip board etc. I have seen others use the level shifters from eBay but I could not get mine to operate with the 1.5volts. <br>
I think if I were building another one I would seriously consider using a AA battery to provide the 1.5v for the calipers rather than using the 5v power rail, this would get round a lot of potential issues with noise on the power line upsetting the calipers and issues of them not starting up correctly when the power is first applied.<br>

The calipers are linked to the CYD via a ribon cable soldered directly to the ESP32 module (the 3 colour led needs to be removed).<br>
<img src="ribbonCablePins.jpeg" /> <br>


See [the main page](https://github.com/alanesq/DRO) for more info. <br>

--------------------
https://github.com/alanesq/DRO - alanesq@disroot.org
