# esp8266-SNTPClock

Clock-Library for esp8266 with SNTP support

This project needs the Arduino environment for the ESP8266 chip: 
 https://github.com/esp8266/Arduino.git

It provides an instance of a clock, which is derived from the Ticker library see: 
 http://arduino.esp8266.com/stable/doc/reference.html#ticker

The clock has an API with following functions:

- `begin()`	set params and retrieve time with: timeServerName, timezone, daylight
- `getTimeSeconds()`	get time in seconds
- `getTimeStruct()`	get a time structur (see Time.h)
- `getTimeStr()`	get a string with current time ```13:50:59```
- `getDateTimeStr()`	get a string with date and time ```2015-10-28 13:50:59```
- `getSecond()`	get the current time seconds
- `attachCb()`	attach a callback function with a timestamp, when to callback
- `detachCb()`	detach the callback function

This libraray uses time functions from the esp8266 SDK. Therefore, no separate NTP network calls are required.

The library is based on the work of https://github.com/esp8266/Arduino.git 

