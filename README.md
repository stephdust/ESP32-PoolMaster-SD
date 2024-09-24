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
<p><img src="https://github.com/christophebelmont/ESP32-PoolMaster/blob/main/docs/Ecosystem.png" alt="Ecosystem"></p>
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
<p>A dedicated board has been designed to host all components. There are 8 LEDs at the bottom to display status, warnings and alarms.</p>
<p>Project includes two PCBs, one for the main ESP32 MCU card and one for the pH/Orp board.<br>
<img src="https://github.com/christophebelmont/ESP32-PoolMaster/blob/main/docs/Hardware.png" alt="Project Hardware"></p>
<h3 id="software">Software</h3>
<p>The project isn’t a fork of the original one due to the different structure of source files with PlatformIO ((.cpp, .h).<br>
It includes:</p>
<ul>
<li>Management of ESP32 and Wifi</li>
<li>ESP32-Arduino framework with PlatformIO IDE</li>
<li>Async MQTT Client</li>
<li>JSON upgrade to version 6</li>
<li>Async Analog measurement via external I2C ADS115</li>
</ul>
<p>The version V6, (aka ESP-2.0) implement direct usage of FreeRTOS functions for managing tasks and queues. There are 10 tasks sharing the app_CPU :</p>
<ul>
<li>The Arduino loopTask, with only the setup() function. When the setup is finished, the task deletes itself to recover memory;</li>
<li>PoolMaster, running every 500ms, which mainly supervises the overall timing of the system;</li>
<li>AnalogPoll, running every 125ms, to acquire analog measurements of pH, ORP and Pressure with an ADS115 sensor on an I2C bus;</li>
<li>GetTemp, running every 1000ms, to acquire water and air temperatures with DS18B20 sensors on two 1Wire busses;</li>
<li>ORPRegulation, running every 1000ms, to manage Chlorine pump;</li>
<li>pHRegulation, running every 1000ms, to manage Acid/Soda pump;</li>
<li>ProcessCommand, running every 500ms, to process commands received on /Home/Pool6/API MQTT Topic;</li>
<li>SettingsPublish, running when notified only (e.g with external command), to publish settings on the MQTT topic;</li>
<li>MeasuresPublish, running every 30s and when notified, to publish actual measures and status;</li>
<li>StatusLights, running every 3000ms, to display a row of 8 status LEDs on the mother board, through a PCF8574A on the I2C bus.<br>
<img src="https://github.com/Gixy31/ESP32-PoolMaster/blob/main/docs/Profiling.jpg" alt="enter image description here"></li>
</ul>
<h3 id="mqtt-api">MQTT API</h3>
<p>Every 30 seconds (configurable), the system will publish the following information:</p>
<pre class=" language-http"><code class="prism  language-http">POOLTOPIC/Meas1
</code></pre>

<table>
<thead>
<tr>
<th align="left">Parameter</th>
<th align="left">Description</th>
<th>Unit</th>
</tr>
</thead>
<tbody>
<tr>
<td align="left"><code>TE</code></td>
<td align="left">Measured air temperature</td>
<td>(/!\ x100)</td>
</tr>
<tr>
<td align="left"><code>Tmp</code></td>
<td align="left">Measured water temperature</td>
<td>(/!\ x100)</td>
</tr>
<tr>
<td align="left"><code>pH</code></td>
<td align="left">pH measurement</td>
<td>(/!\ x100)</td>
</tr>
<tr>
<td align="left"><code>PSI</code></td>
<td align="left">Pump pressure PSI</td>
<td>(/!\ x100)</td>
</tr>
<tr>
<td align="left"><code>Orp</code></td>
<td align="left">Orp measurement</td>
<td></td>
</tr>
<tr>
<td align="left"><code>PhUpT</code></td>
<td align="left">pH peristaltic pump uptime</td>
<td>(/!\ x100)</td>
</tr>
<tr>
<td align="left"><code>ChlUpT</code></td>
<td align="left">Chlorine peristaltic pump uptime</td>
<td>(/!\ x100)</td>
</tr>
</tbody>
</table><pre class=" language-http"><code class="prism  language-http">POOLTOPIC/Meas2
</code></pre>

<table>
<thead>
<tr>
<th align="left">Parameter</th>
<th align="left">Description</th>
<th>Unit</th>
</tr>
</thead>
<tbody>
<tr>
<td align="left"><code>AcidF</code></td>
<td align="left">pH tank fill percentage</td>
<td></td>
</tr>
<tr>
<td align="left"><code>ChlF</code></td>
<td align="left">Chlorine tank fill percentage</td>
<td></td>
</tr>
<tr>
<td align="left"><code>IO</code></td>
<td align="left">Bitmap1 (see below)</td>
<td></td>
</tr>
<tr>
<td align="left"><code>IO2</code></td>
<td align="left">Bitmap2 (see below)</td>
<td></td>
</tr>
<tr>
<td align="left"><code>IO3</code></td>
<td align="left">Bitmap3 (see below)</td>
<td></td>
</tr>
</tbody>
</table><p>Bitmap are on/off states of various elements in the system:</p>

<table>
<thead>
<tr>
<th>Bit</th>
<th align="left">Parameter</th>
<th align="left">Description</th>
</tr>
</thead>
<tbody>
<tr>
<td>7</td>
<td align="left">FiltPump</td>
<td align="left">current state of Filtration Pump (0=on, 1=off)</td>
</tr>
<tr>
<td>6</td>
<td align="left">PhPump</td>
<td align="left">current state of Ph Pump (0=on, 1=off)</td>
</tr>
<tr>
<td>5</td>
<td align="left">ChlPump</td>
<td align="left">current state of Chl Pump (0=on, 1=off)</td>
</tr>
<tr>
<td>4</td>
<td align="left">PhlLevel</td>
<td align="left">current state of Acid tank level (0=empty, 1=ok)</td>
</tr>
<tr>
<td>3</td>
<td align="left">ChlLevel</td>
<td align="left">current state of Chl tank level (0=empty, 1=ok)</td>
</tr>
<tr>
<td>2</td>
<td align="left">PSIError</td>
<td align="left">over-pressure error</td>
</tr>
<tr>
<td>1</td>
<td align="left">pHErr</td>
<td align="left">pH pump overtime error flag</td>
</tr>
<tr>
<td>0</td>
<td align="left">ChlErr</td>
<td align="left">Chl pump overtime error flag</td>
</tr>
</tbody>
</table>
<table>
<thead>
<tr>
<th>Bit</th>
<th align="left">Parameter</th>
<th align="left">Description</th>
</tr>
</thead>
<tbody>
<tr>
<td>7</td>
<td align="left">pHPID</td>
<td align="left">current state of pH PID regulation loop (1=on, 0=off)</td>
</tr>
<tr>
<td>6</td>
<td align="left">OrpPID</td>
<td align="left">current state of Orp PID regulation loop (1=on, 0=off)</td>
</tr>
<tr>
<td>5</td>
<td align="left">Mode</td>
<td align="left">state of pH and Orp regulation mode (0=manual, 1=auto)</td>
</tr>
<tr>
<td>4</td>
<td align="left">RobotPump</td>
<td align="left">current state of Robot Pump (0=on, 1=off)</td>
</tr>
<tr>
<td>3</td>
<td align="left">RELAYR0</td>
<td align="left">current state of spare Relay0 (0=on, 1=off)</td>
</tr>
<tr>
<td>2</td>
<td align="left">RELAYR1</td>
<td align="left">current state of spare Relay1 (0=on, 1=off)</td>
</tr>
<tr>
<td>1</td>
<td align="left">Winter</td>
<td align="left">current state of winter mode (0=summer, 1=winter)</td>
</tr>
<tr>
<td>0</td>
<td align="left">NA</td>
<td align="left">Unused</td>
</tr>
</tbody>
</table><pre class=" language-http"><code class="prism  language-http">POOLTOPIC/Set1
</code></pre>

<table>
<thead>
<tr>
<th align="left">Parameter</th>
<th align="left">Description</th>
</tr>
</thead>
<tbody>
<tr>
<td align="left"><code>FW</code></td>
<td align="left">Firmware Version</td>
</tr>
<tr>
<td align="left"><code>FSta</code></td>
<td align="left">Computed filtration start hour, in the morning (hours)</td>
</tr>
<tr>
<td align="left"><code>FStaM</code></td>
<td align="left">Earliest Filtration start hour, in the morning (hours)</td>
</tr>
<tr>
<td align="left"><code>FDu</code></td>
<td align="left">Computed filtration duration based on water temperature (hours)</td>
</tr>
<tr>
<td align="left"><code>FStoM</code></td>
<td align="left">Latest hour for the filtration to run. Whatever happens, filtration won’t run later than this hour</td>
</tr>
<tr>
<td align="left"><code>FSto</code></td>
<td align="left">Computed filtration stop hour, equal to FSta + FDu (hour)</td>
</tr>
<tr>
<td align="left"><code>pHUTL</code></td>
<td align="left">Max allowed daily run time for the pH pump (/!\ mins)</td>
</tr>
<tr>
<td align="left"><code>ChlUTL</code></td>
<td align="left">Max allowed daily run time for the Chl pump (/!\ mins)</td>
</tr>
</tbody>
</table><pre class=" language-http"><code class="prism  language-http">POOLTOPIC/Set2
</code></pre>

<table>
<thead>
<tr>
<th align="left">Parameter</th>
<th align="left">Description</th>
</tr>
</thead>
<tbody>
<tr>
<td align="left"><code>pHWS</code></td>
<td align="left">pH PID window size (/!\ mins)</td>
</tr>
<tr>
<td align="left"><code>ChlWS</code></td>
<td align="left">Orp PID window size (/!\ mins)</td>
</tr>
<tr>
<td align="left"><code>pHSP</code></td>
<td align="left">pH setpoint (/!\ x100)</td>
</tr>
<tr>
<td align="left"><code>OrpSP</code></td>
<td align="left">Orp setpoint</td>
</tr>
<tr>
<td align="left"><code>WSP</code></td>
<td align="left">Water temperature setpoint (/!\ x100)</td>
</tr>
<tr>
<td align="left"><code>WLT</code></td>
<td align="left">Water temperature low threshold to activate anti-freeze mode (/!\ x100)</td>
</tr>
<tr>
<td align="left"><code>PSIHT</code></td>
<td align="left">Water pressure high threshold to trigger error (/!\ x100)</td>
</tr>
<tr>
<td align="left"><code>PSIMT</code></td>
<td align="left">Water pressure medium threshold (unused yet) (/!\ x100)</td>
</tr>
</tbody>
</table><p>{“Tmp”:818,“pH”:321,“PSI”:56,“Orp”:583,“FilUpT”:8995,“PhUpT”:0,“ChlUpT”:0}<br>
{“AcidF”:100,“ChlF”:100,“IO”:11,“IO2”:0}<br>
Tmp: measured Water temperature value in °C x100 (8.18°C in the above example payload)<br>
pH: measured pH value x100 (3.21 in the above example payload)<br>
Orp: measured Orp (aka Redox) value in mV (583mV in the above example payload)<br>
PSI: measured Water pressure value in bar x100 (0.56bar in the above example payload)<br>
FiltUpT: current running time of Filtration pump in seconds (reset every 24h. 8995secs in the above example payload)<br>
PhUpT: current running time of Ph pump in seconds (reset every 24h. 0secs in the above example payload)<br>
ChlUpT: current running time of Chl pump in seconds (reset every 24h. 0secs in the above example payload)<br>
AcidF: percentage fill estimate of acid tank (“pHTank” command must have been called when a new acid tank was set in place in order to have accurate value)<br>
ChlF: percentage fill estimate of Chlorine tank (“ChlTank” command must have been called when a new Chlorine tank was set in place in order to have accurate value)<br>
IO: a variable of type BYTE where each individual bit is the state of a digital input on the Arduino. These are :</p>
<ul>
<li>FiltPump: current state of Filtration Pump (0=on, 1=off)</li>
<li>PhPump: current state of Ph Pump (0=on, 1=off)</li>
<li>ChlPump: current state of Chl Pump (0=on, 1=off)</li>
<li>PhlLevel: current state of Acid tank level (0=empty, 1=ok)</li>
<li>ChlLevel: current state of Chl tank level (0=empty, 1=ok)</li>
<li>PSIError: over-pressure error</li>
<li>pHErr: pH pump overtime error flag</li>
<li>ChlErr: Chl pump overtime error flag</li>
</ul>
<p>IO2: a variable of type BYTE where each individual bit is the state of a digital input on the Arduino. These are :</p>
<ul>
<li>
<p>pHPID: current state of pH PID regulation loop (1=on, 0=off)</p>
</li>
<li>
<p>OrpPID: current state of Orp PID regulation loop (1=on, 0=off)</p>
</li>
<li>
<p>Mode: state of pH and Orp regulation mode (0=manual, 1=auto)</p>
</li>
<li>
<p>Heat: state of water heat command (0=off, 1=on)</p>
</li>
<li>
<p>R1: state of Relay1 (0=off, 1=on)</p>
</li>
<li>
<p>R2: state of Relay2 (0=off, 1=on)</p>
</li>
<li>
<p>R6: state of Relay6 (0=off, 1=on)</p>
</li>
<li>
<p>R7: state of Relay7 (0=off, 1=on)</p>
</li>
<li>
<p>Support for ElegantOTA for remote upgrade</p>
</li>
<li>
<p>Support for momentary relay mode to simulate button press. This allows controlling push button operated devices (exemple automatically switch on and off an Intex Salt Water Cholrine Generator)</p>
</li>
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

