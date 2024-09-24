---


---

<h1 id="esp32-poolmaster">ESP32 PoolMaster</h1>
<h2 id="brief-description">Brief description</h2>
<p>This project is a fork of ESP32 Poolmaster by Gixy (<a href="https://github.com/Gixy31/ESP32-PoolMaster">https://github.com/Gixy31/ESP32-PoolMaster</a>).<br>
Gixy’s project is a redesign for ESP32 of an initial project by Loic74650 <a href="https://github.com/Loic74650/PoolMaster">(https://github.com/Loic74650/PoolMaster</a>) which was intended for Atmega2560 MCUs.</p>
<p>PoolMaster is a complete pool management system. It performs automatic water maintenance by measuring various water metrics and controling pool equipment (Pump, Heater, Lightning, pH and Orp pumps, etc.).</p>
<p>Project includes ESP code and two PCB designs for:</p>
<ul>
<li>The main control card</li>
<li>The pH/Orp aquisition card.</li>
</ul>
<p>The main unit interfaces with a 3.5’’ Nextion touchscreen for local information and control.<br>
It includes Wifi connectivity to communicate via MQTT and SMTP. Project includes configuration files for various home automation systems such as Home Assistant, Jeedom, NodeRed, Grafana, etc.</p>
<p>Finally code and PCB are made with high flexibility to precisely tailor the features to the user.</p>
<p><img src="https://github.com/christophebelmont/ESP32-PoolMaster/blob/main/docs/System%20Wiring.png" alt="System Wiring"></p>
<h2 id="project-details">Project Details</h2>
<h3 id="features">Features</h3>
<p>System continuously monitors various metrics and periodically reports them over MQTT and touchscreen:</p>
<ul>
<li>Water and Air Temperature</li>
<li>Water pressure</li>
<li>pH</li>
<li>Orp</li>
<li>Chlorine tank level</li>
<li>pH tank level</li>
</ul>
<p>pH and Orp are adjusted thanks to two peristaltic pumps continuously controlled by two PID regulation loops.<br>
PoolMaster also supports Chlorine injection via Salt Water Chlorine Generator.<br>
Water temperature can be controlled by the system via dry contact relays.</p>
<p>Defined time-slots and water temperature are used to start/stop the filtration pump for a daily given amount of time (a relay starts/stops the filtration pump). A winter starts the filtration if temperature reaches -2°C until it rises back above +2°C.</p>
<p>Tank-levels are estimated based on the running-time and flow-rate of each pump. Additionnally a low level contact can be plugged into the system.</p>
<h3 id="hardware">Hardware</h3>
<p>Project includes two PCBs, one for the main ESP32 MCU and one for the pH/Orp board.<br>
<img src="https://github.com/christophebelmont/ESP32-PoolMaster/blob/main/docs/Hardware.png" alt="Project Hardware"></p>
<h3 id="software">Software</h3>
<ul>
<li>Support for ElegantOTA for remote upgrade</li>
<li>Support for momentary relay mode to simulate button press. This allows controlling push button operated devices (exemple automatically switch on and off an Intex Salt Water Cholrine Generator)</li>
</ul>
<p>The project isn’t a fork of the original one due to the different structure of source files with PlatformIO ((.cpp, .h). A dedicated board has been designed to host all components. There are 8 LEDs at the bottom to display status, warnings and alarms.</p>
<p>In version ESP-3.0, the display function has been very simplified (twice less code), using Nextion variables only to deport the logic into the Nextion and updating the display only when it is ON.</p>
<p>A new version of the board allows the connection of the pH_Orp board from Loïc (<a href="https://github.com/Loic74650/pH_Orp_Board/tree/main">https://github.com/Loic74650/pH_Orp_Board/tree/main</a>) on an additional I2C connector. The sofware is modified accordingly. The configuration is defined in the config.h file. CAD_files 2 Gerber 3 files are provided.</p>
<p>The version V6, (aka ESP-2.0) implement direct usage of FreeRTOS functions for managing tasks and queues. There are 10 tasks sharing the app_CPU : - The Arduino loopTask, with only the setup() function. When the setup is finished, the task deletes itself to recover memory; - PoolMaster, running every 500ms, which mainly supervises the overall timing of the system; - AnalogPoll, running every 125ms, to acquire analog measurements of pH, ORP and Pressure with an ADS115 sensor on an I2C bus; - GetTemp, running every 1000ms, to acquire water and air temperatures with DS18B20 sensors on two 1Wire busses; - ORPRegulation, running every 1000ms, to manage Chlorine pump; - pHRegulation, running every 1000ms, to manage Acid/Soda pump; - ProcessCommand, running every 500ms, to process commands received on /Home/Pool6/API MQTT Topic; - SettingsPublish, running when notified only (e.g with external command), to publish settings on the MQTT topic; - MeasuresPublish, running every 30s and when notified, to publish actual measures and status; - StatusLights, running every 3000ms, to display a row of 8 status LEDs on the mother board, through a PCF8574A on the I2C bus.</p>
<p><img src="/docs/Profiling.jpg" alt=""></p>
<p><img src="/docs/PoolMaster_board.JPG" alt="" title="Board"></p>
<p>![](/docs/Page 0.JPG)</p>
<p>![](/docs/Page 1.JPG)</p>
<p>![](/docs/Page 3.JPG)</p>
<h2 id="poolmaster-5.0.0">PoolMaster 5.0.0</h2>
<h2 id="arduino-mega2560-or-controllino-maxi-phorp-chlorine-regulation-system-for-home-pools">Arduino Mega2560 (or Controllino-Maxi) Ph/Orp (Chlorine) regulation system for home pools</h2>
<p><img src="/docs/PoolMaster_2.jpg" alt="" title="Overview"></p>
<p><img src="/docs/Grafana.png" alt="" title="Dashboard"></p>

