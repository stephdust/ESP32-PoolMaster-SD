
# ESP32 PoolMaster

# Brief description
This project is a fork of ESP32 poolmaster by Gixy ([https://github.com/Gixy31/ESP32-PoolMaster](https://github.com/Gixy31/ESP32-PoolMaster)).
PoolMaster is a complete pool management system. It performs automatic water maintenance by measuring various water metrics and controling pool equipment (Pump, Heater, Lightning, pH and Orp pumps, etc.).

Project comes with two PCB designs
- main control card
- pH/Orp aquisition cards.

Moreover, it can controlled using:
- A large color Nextion touchscreen 
- MQTT to read pool status and control the system remotely via home automation systems (Home Assistant, Jeedom, etc.). It can be accessed from anywhere from any device (phone, tablet, etc.)

Finally code and PCB are made with high flexibility to precisely tailor the features to the user.

![Typical installation (without electrical part)](https://github.com/christophebelmont/ESP32-PoolMaster/blob/main/docs/InsideBox.png)

# Brief description
**In addition of the original project, this fork includes:**
- Support for Salt Water Chlorine Generator which can work in parallel (or in replacement) of a Chlorine pump
- Support for ElegantOTA for remote upgrade
- Support for momentary relay mode to simulate button press. This allows controlling push button operated devices (exemple automatically switch on and off an Intex Salt Water Cholrine Generator)
  
The project isn't a fork of the original one due to the different structure of source files with PlatformIO ((.cpp, .h). A dedicated board has been designed to host all components. There are 8 LEDs at the bottom to display status, warnings and alarms.  
  
In version ESP-3.0, the display function has been very simplified (twice less code), using Nextion variables only to deport the logic into the Nextion and updating the display only when it is ON.  
  
A new version of the board allows the connection of the pH\_Orp board from Loïc (https://github.com/Loic74650/pH\_Orp\_Board/tree/main) on an additional I2C connector. The sofware is modified accordingly. The configuration is defined in the config.h file. CAD\_files 2 Gerber 3 files are provided.  
  
The version V6, (aka ESP-2.0) implement direct usage of FreeRTOS functions for managing tasks and queues. There are 10 tasks sharing the app_CPU : - The Arduino loopTask, with only the setup() function. When the setup is finished, the task deletes itself to recover memory; - PoolMaster, running every 500ms, which mainly supervises the overall timing of the system; - AnalogPoll, running every 125ms, to acquire analog measurements of pH, ORP and Pressure with an ADS115 sensor on an I2C bus; - GetTemp, running every 1000ms, to acquire water and air temperatures with DS18B20 sensors on two 1Wire busses; - ORPRegulation, running every 1000ms, to manage Chlorine pump; - pHRegulation, running every 1000ms, to manage Acid/Soda pump; - ProcessCommand, running every 500ms, to process commands received on /Home/Pool6/API MQTT Topic; - SettingsPublish, running when notified only (e.g with external command), to publish settings on the MQTT topic; - MeasuresPublish, running every 30s and when notified, to publish actual measures and status; - StatusLights, running every 3000ms, to display a row of 8 status LEDs on the mother board, through a PCF8574A on the I2C bus.  
  

![](/docs/Profiling.jpg)

  
  

![](/docs/PoolMaster_board.JPG "Board")

  
  

![](/docs/Page 0.JPG)

![](/docs/Page 1.JPG)

![](/docs/Page 3.JPG)

  
  

## PoolMaster 5.0.0

## Arduino Mega2560 (or Controllino-Maxi) Ph/Orp (Chlorine) regulation system for home pools

  

![](/docs/PoolMaster_2.jpg "Overview")

  
  
  

![](/docs/Grafana.png "Dashboard")
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTE1NDQyOTUyMDhdfQ==
-->