ESP8266
=======

An Arduino library for the ESP8266EX chipset (WI07c module)

Milestones
-----------
1. Implement all of the WI07c module's features such as TCP/UDP server/client.
2. Break code into classes (based on protocol and role)
3. Inherit from Arduino base classes, such as Stream
4. Optimize

Pinout
-----------
![ESP8266 WI07c module pinout](http://i.imgur.com/AjL0nPu.png)

Notes
-----------
The WI07c module is connected at the Arduino's hardware serial. SoftwareSerial can't handle speeds more than 34800kbps. Unfortunately as of today, the module's baudrate isn't configurable and is set to 115200kbps. You can either use an FTDI breakout for the serial monitor (using SoftwareSerial), or an Arduino Mega.

References
-----------
* [Wi07c - ElectroDragon](http://www.electrodragon.com/w/Wi07c)
* [ESP8266 - NURDspace](https://nurdspace.nl/ESP8266)

**This is a WIP.**
While it is not complete, use full and stable lib from [itead](https://github.com/itead/ITEADLIB_Arduino_ESP8266)
