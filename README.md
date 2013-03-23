m4HouseStation
==============

This is a project logging the electricity and gas consumption. In addition Temperature and Humidity is logged.

Features
--------
- Detecting Impulses through..
  - ..Reed Switch -> for Gas Meter
  - ..Photoresistor -> for Ele Meter
- Logging Temperature
- Logging Humidity
- Saving every Impulse in CSV files on the SD card (different files for different days) 
    marked with the current Timestamp (1 second accuracy)
- Saving meter values once every day
- Backing Up Meter Values every 15 minutes (in EEPROM)
- On power down, the time is not lost (RTC chip with 3.3V battery)
- Providing a Webinterface
  - Current Stats
    - Meter Values
    - Temperature and Humidity
    - Current Ele and Gas Consumption
  - Daily Graphs of Gas,Ele,Temp,Hum (also unfinished days)
  - Overview of Gas and Ele
  - Meter Values can be adjusted through the Interface
  - Days can be chosen via a comfortable Datepicker

Hardware
--------
- Arduino Uno
- Arduino Ethernet Shield
- SD card
- RTC chip
- Temperature and Humidity Sensor
- Logic for detecting Pulses

Libraries
---------
tbA
