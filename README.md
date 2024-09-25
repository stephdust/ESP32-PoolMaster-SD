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

<table>
<thead>
<tr>
<th align="left">Task</th>
<th align="left">Description</th>
<th>Run every</th>
</tr>
</thead>
<tbody>
<tr>
<td align="left"><code>Arduino Loop</code></td>
<td align="left">with only the setup() function. When the setup is finished, the task deletes itself to recover memory</td>
<td><code>Startup</code></td>
</tr>
<tr>
<td align="left"><code>PoolMaster</code></td>
<td align="left">mainly supervises the overall timing of the system</td>
<td><code>500ms</code></td>
</tr>
<tr>
<td align="left"><code>AnalogPoll</code></td>
<td align="left">acquire analog measurements of pH, ORP and Pressure with an ADS115 sensor on an I2C bus</td>
<td><code>125ms</code></td>
</tr>
<tr>
<td align="left"><code>GetTemp</code></td>
<td align="left">acquire water and air temperatures with DS18B20 sensors on two 1Wire busses</td>
<td><code>1000ms</code></td>
</tr>
<tr>
<td align="left"><code>ORPRegulation</code></td>
<td align="left">manage Chlorine pump</td>
<td><code>1000ms</code></td>
</tr>
<tr>
<td align="left"><code>pHRegulation</code></td>
<td align="left">manage Acid/Soda pump</td>
<td><code>1000ms</code></td>
</tr>
<tr>
<td align="left"><code>ProcessCommand</code></td>
<td align="left">process commands received on API MQTT Topic</td>
<td><code>500ms</code></td>
</tr>
<tr>
<td align="left"><code>SettingsPublish</code></td>
<td align="left">publish settings on the MQTT topic (Set1 - Set5)</td>
<td><code>When notifed</code></td>
</tr>
<tr>
<td align="left"><code>MeasuresPublish</code></td>
<td align="left">publish measurement on the MQTT topic (Meas1 - Meas2)</td>
<td><code>30s</code></td>
</tr>
<tr>
<td align="left"><code>StatusLights</code></td>
<td align="left">display a row of 8 status LEDs on the mother board, through a PCF8574A on the I2C bus</td>
<td><code>3000ms</code></td>
</tr>
</tbody>
</table><p><img src="https://github.com/Gixy31/ESP32-PoolMaster/blob/main/docs/Profiling.jpg" alt="enter image description here"></p>
<h3 id="touchscreen-control">TouchScreen control</h3>
<p><img src="https://github.com/christophebelmont/ESP32-PoolMaster/blob/main/docs/Nextion_Screens.png" alt="Nextion TouchScreen HMI"></p>
<h3 id="home-automation-integration">Home Automation Integration</h3>
<p><img src="https://github.com/christophebelmont/ESP32-PoolMaster/blob/main/docs/Grafana%20and%20App.png" alt="Home Automation Integration"></p>
<h3 id="tips">Tips</h3>
<p>Before attempting to regulate your pool water with this automated system, it is essential that you start with:</p>
<ol>
<li>testing your water quality (using liquid kits and/or test strips for instance) and balancing it properly (pH, Chlorine, Alkalinity, Hardness). Proper water balancing will greatly ease the pH stability and regulation</li>
<li>calibrating the pH probe using calibrated buffer solutions (pay attention to the water temperature which plays a big role in pH readings)</li>
<li>adjusting pH to 7.4</li>
<li>List item</li>
<li>once above steps 1 to 3 are ok, you can start regulating ORP</li>
</ol>
<p>Notes:</p>
<ul>
<li>the ORP sensor should theoretically not be calibrated nore temperature compensated (by nature its 0mV pivot point cannot shift)</li>
<li>the ORP reading is strongly affected by the pH value and the water temperature. Make sure pH is adjusted at 7.4</li>
<li>prefer platinium ORP probes for setpoints &gt;500mV (ie. Pools and spas)</li>
<li>the response time of ORP sensors can be fast in reference buffer solutions (10 secs) and yet very slow in pool water (minutes or more) as it depends on the water composition</li>
</ul>
<h3 id="details-on-the-pid-regulation">Details on the PID regulation</h3>
<p>In this project the Arduino PID library is used to start/stop the chemicals pumps in a cyclic manner, similar to a “PWM” signal.<br>
The period of the cycle is defined by the WINDOW SIZE parameter which is fixed (in Milliseconds). What the PID library does is vary the duty cycle of the cycle.<br>
If the computed error in the PID loop is null or negative, the duty cycle is set to zero (the ouput of the function PID.Compute()) and the pump is never actuated.<br>
If the error is positive, the duty cycle is set to a value between 0 and a max value (equal to the WINDOW SIZE, ie. the pump is running full time).</p>
<p>So in practice the ouput of the PID.Compute() function is a duration in milliseconds during which the pump will be activated for every cycle.<br>
If for instance, the WINDOW SIZE is set to 3600000ms (ie. one hour) and the output of the PID is 600000ms (ie. 10mins), the pump will be activated for 10mins at the begining of every hour cycle.</p>
<p>On the default Kp,Ki,Kd parameter values of the PID:<br>
By default in this project, Ki and Kd are null for stability reasons and so the PID loop is only a P loop, ie. a proportional loop.<br>
Adding some Ki and Kd to the PID loop may theoretically increase regulation performance but is also more complex to adjust and could result in instabilities. Since a P-only loop worked well enough and that safety considerations should be taken seriously in this project, I left it as is.</p>
<p>For my 50m3 pool the Kp default values are 2000000 for the pH loop and 4500 for the Orp loop. They were chosen experimentally in the following way:</p>
<p>I experimentally checked how much chemical was required to change the measured parameter (pH or Orp) by a certain amount. For instance I determined that 83ml of acid changed the pH by 0.1 for my 50m3 pool. The flow rate of the acid pump being 1.5L/hour, we can then determine for how many minutes the pump should be activated if the pH error is 0.1, which are (0.083*60/1.5) = 3.3minutes or roughly 200000ms.<br>
And so for an error of 1 in the pH PID loop, the pump needs to be activated 10 times longer, ie. during 2000000ms, which should be taken as the Kp value. The same reasoning goes for the Kp value of the Orp PID loop.</p>
<p>On the WINDOW SIZE:<br>
Various parameters influence the speed at which an injected chemical in the pool water will result in a variation in the measured pH or Orp.<br>
Experimentally I measured that in my case it can take up to 30minutes and therefore the injection cycle period should be at least 30mins or longer in order not to inject more chemical over the following cycles thinking that it required more when in fact the chemical reactions simply needed more time to take effect, which would eventually result in overshooting. So in my case I setlled for a safe one hour WINDOW SIZE (ie. 3600000ms)</p>
<h3 id="mqtt-api">MQTT API</h3>
<h4 id="get-information">Get Information</h4>
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
<pre class=" language-http"><code class="prism  language-http">IO
</code></pre>

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
<td align="left">current state of Filtration Pump (1=on, 0=off)</td>
</tr>
<tr>
<td>6</td>
<td align="left">PhPump</td>
<td align="left">current state of Ph Pump (1=on, 0=off)</td>
</tr>
<tr>
<td>5</td>
<td align="left">ChlPump</td>
<td align="left">current state of Chl Pump (1=on, 0=off)</td>
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
</table><pre class=" language-http"><code class="prism  language-http">IO2
</code></pre>

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
<td align="left">current state of Robot Pump (1=on, 0=off)</td>
</tr>
<tr>
<td>3</td>
<td align="left">RELAYR0</td>
<td align="left">current state of spare Relay0 (1=on, 0=off)</td>
</tr>
<tr>
<td>2</td>
<td align="left">RELAYR1</td>
<td align="left">current state of spare Relay1 (1=on, 0=off)</td>
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
</table><pre class=" language-http"><code class="prism  language-http">IO3
</code></pre>

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
<td align="left"></td>
<td align="left">Unused</td>
</tr>
<tr>
<td>6</td>
<td align="left"></td>
<td align="left">Unused</td>
</tr>
<tr>
<td>5</td>
<td align="left"></td>
<td align="left">Unused</td>
</tr>
<tr>
<td>4</td>
<td align="left"></td>
<td align="left">Unused</td>
</tr>
<tr>
<td>3</td>
<td align="left">SWGMode</td>
<td align="left">current mode of Salt Water Chlorine Generator (0=no SWG, 1=SWG active)</td>
</tr>
<tr>
<td>2</td>
<td align="left">SWGState</td>
<td align="left">current state of Salt Water Chlorine Generator (0=off, 1=on)</td>
</tr>
<tr>
<td>1</td>
<td align="left">pHPIDEnabled</td>
<td align="left">current mode of pH PID (0=stop, 1=PID active)</td>
</tr>
<tr>
<td>0</td>
<td align="left">OrpPIDEnabled</td>
<td align="left">current mode of Orp PID (0=stop, 1=PID active)</td>
</tr>
</tbody>
</table><p>In addition the system published on-demand settings :</p>
<pre class=" language-http"><code class="prism  language-http">POOLTOPIC/Set1
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
</table><pre class=" language-http"><code class="prism  language-http">POOLTOPIC/Set3
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
<td align="left"><code>pHC0</code></td>
<td align="left">pH sensor calibration coefficient C0</td>
</tr>
<tr>
<td align="left"><code>pHC1</code></td>
<td align="left">pH sensor calibration coefficient C1</td>
</tr>
<tr>
<td align="left"><code>OrpC0</code></td>
<td align="left">Orp sensor calibration coefficient C0</td>
</tr>
<tr>
<td align="left"><code>OrpC1</code></td>
<td align="left">Orp sensor calibration coefficient C1</td>
</tr>
<tr>
<td align="left"><code>PSIC0</code></td>
<td align="left">Pressure sensor calibration coefficient C0</td>
</tr>
<tr>
<td align="left"><code>PSIC1</code></td>
<td align="left">Pressure sensor calibration coefficient C1</td>
</tr>
</tbody>
</table><pre class=" language-http"><code class="prism  language-http">POOLTOPIC/Set4
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
<td align="left"><code>pHKp</code></td>
<td align="left">pH PID coeffcicient Kp</td>
</tr>
<tr>
<td align="left"><code>pHKi</code></td>
<td align="left">pH PID coeffcicient Ki</td>
</tr>
<tr>
<td align="left"><code>pHKd</code></td>
<td align="left">pH PID coeffcicient Kd</td>
</tr>
<tr>
<td align="left"><code>OrpKp</code></td>
<td align="left">Orp PID coeffcicient Kp</td>
</tr>
<tr>
<td align="left"><code>OrpKi</code></td>
<td align="left">Orp PID coeffcicient Ki</td>
</tr>
<tr>
<td align="left"><code>OrpKd</code></td>
<td align="left">Orp PID coeffcicient Kd</td>
</tr>
<tr>
<td align="left"><code>Dpid</code></td>
<td align="left">Delay from FSta for the water regulation/PIDs to start (mins)</td>
</tr>
<tr>
<td align="left"><code>PubP</code></td>
<td align="left">Settings publish period in sec</td>
</tr>
</tbody>
</table><pre class=" language-http"><code class="prism  language-http">POOLTOPIC/Set5
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
<td align="left"><code>pHTV</code></td>
<td align="left">Acid tank nominal volume (Liters)</td>
</tr>
<tr>
<td align="left"><code>ChlTV</code></td>
<td align="left">Chl tank nominal volume (Liters)</td>
</tr>
<tr>
<td align="left"><code>pHFR</code></td>
<td align="left">Acid pump flow rate (L/hour)</td>
</tr>
<tr>
<td align="left"><code>OrpFR</code></td>
<td align="left">Chl pump flow rate (L/hour)</td>
</tr>
<tr>
<td align="left"><code>SWGSec</code></td>
<td align="left">SWG Chlorine Generator Secure Temperature (°C)</td>
</tr>
<tr>
<td align="left"><code>SWGDel</code></td>
<td align="left">SWG Chlorine Generator Delay before start after pump (mn)</td>
</tr>
</tbody>
</table><h4 id="push-information">Push Information</h4>
<p>Below are the Payloads/commands to publish on the “PoolTopicAPI” topic in Json format in order to launch actions :</p>

<table>
<thead>
<tr>
<th align="left">Parameter</th>
<th align="left">Description</th>
</tr>
</thead>
<tbody>
<tr>
<td align="left">{“Mode”:1} or {“Mode”:0}</td>
<td align="left">set “Mode” to manual (0) or Auto (1). In Auto, filtration starts/stops at set times of the day and pH and Orp are regulated (if pH/Orp PID Mode activated see below)</td>
</tr>
<tr>
<td align="left">{“FiltPump”:1} or {“FiltPump”:0}</td>
<td align="left">manually start/stop the filtration pump</td>
</tr>
<tr>
<td align="left">{“PhPIDEnabled”:1} or {“PhPIDEnabled”:0}</td>
<td align="left">set " PID Mode" for pH regulation to manual (0) or Auto (1). In Auto mode the pH PID regulation loop (see below) turns on and off automatically.</td>
</tr>
<tr>
<td align="left">{“PhPID”:1} or {“PhPID”:0}</td>
<td align="left">start/stop the Ph PID regulation loop which directly controls the pH pump.</td>
</tr>
<tr>
<td align="left">{“PhPump”:1} or {“PhPump”:0}</td>
<td align="left">manually start/stop the Acid pump to lower the Ph</td>
</tr>
<tr>
<td align="left">{“OrpPIDEnabled”:1} or {“OrpPIDEnabled”:0}</td>
<td align="left">set " PID Mode" for Orp regulation to manual (0) or Auto (1). In Auto mode the Orp PID regulation loop (see below) turns on and off automatically.</td>
</tr>
<tr>
<td align="left">{“OrpPID”:1} or {“OrpPID”:0}</td>
<td align="left">start/stop the Orp PID regulation loop which directly controls the Chlorine pump.</td>
</tr>
<tr>
<td align="left">{“ChlPump”:1} or {“ChlPump”:0}</td>
<td align="left">manually start/stop the Chl pump to add more Chlorine</td>
</tr>
<tr>
<td align="left">{“ElectrolyseMode”:1} or {“ElectrolyseMode”:0}</td>
<td align="left">set auto mode for Chlorine production via Salt Water Chlorine Generation. In Auto mode the Orp regulates itself (when filtration pump on) with Salt Water Chlorine Generator.</td>
</tr>
<tr>
<td align="left">{“Electrolyse”:1} or {“Electrolyse”:0}</td>
<td align="left">start/stop the Salt Water Chlorine Generator device (if exists)</td>
</tr>
<tr>
<td align="left">{"ElectroSecure:16}</td>
<td align="left">set the minimum temperature for Salt Water Chlorine Generation system to operate</td>
</tr>
<tr>
<td align="left">{"ElectroDelay:2}</td>
<td align="left">set the delay between filtration pump start and Salt Water Chlorine Generation system start.</td>
</tr>
<tr>
<td align="left">{“PhCalib”:[4.02,3.8,9.0,9.11]}</td>
<td align="left">multi-point linear regression calibration (minimum 1 point-couple, 6 max.) in the form [ProbeReading_0, BufferRating_0, xx, xx, ProbeReading_n, BufferRating_n]</td>
</tr>
<tr>
<td align="left">{“OrpCalib”:[450,465,750,784]}</td>
<td align="left">multi-point linear regression calibration (minimum 1 point-couple, 6 max.) in the form [ProbeReading_0, BufferRating_0, xx, xx, ProbeReading_n, BufferRating_n]</td>
</tr>
<tr>
<td align="left">{“PhSetPoint”:7.4}</td>
<td align="left">set the Ph setpoint, 7.4 in this example</td>
</tr>
<tr>
<td align="left">{“OrpSetPoint”:750.0}</td>
<td align="left">set the Orp setpoint, 750mV in this example</td>
</tr>
<tr>
<td align="left">{“WSetPoint”:27.0}</td>
<td align="left">set the water temperature setpoint, 27.0deg in this example</td>
</tr>
<tr>
<td align="left">{“Winter”:1} or {“Winter”:0}</td>
<td align="left">set “Winter” mode to On or Off. Mainly deactivate all the regulation. Only pump continues to operate. Pump will start operating when temperature reaches -2°C and stop if temperature rises back to +2°C.</td>
</tr>
<tr>
<td align="left">{“WTempLow”:10.0}</td>
<td align="left">set the water low-temperature threshold below which there is no need to regulate Orp and Ph (ie. in winter)</td>
</tr>
<tr>
<td align="left">{“OrpPIDParams”:[2857,0,0]}</td>
<td align="left">respectively set Kp,Ki,Kd parameters of the Orp PID loop. In this example they are set to 2857, 0 and 0</td>
</tr>
<tr>
<td align="left">{“PhPIDParams”:[1330000,0,0.0]}</td>
<td align="left">respectively set Kp,Ki,Kd parameters of the Ph PID loop. In this example they are set to 1330000, 0 and 0</td>
</tr>
<tr>
<td align="left">{“OrpPIDWSize”:3600000}</td>
<td align="left">set the window size of the Orp PID loop in msec, 60mins in this example</td>
</tr>
<tr>
<td align="left">{“PhPIDWSize”:1200000}</td>
<td align="left">set the window size of the Ph PID loop in msec, 20mins in this example</td>
</tr>
<tr>
<td align="left">{“Date”:[1,1,1,18,13,32,0]}</td>
<td align="left">set date/time of RTC module in the following format: (Day of the month, Day of the week, Month, Year, Hour, Minute, Seconds), in this example: Monday 1st January 2018 - 13h32mn00secs</td>
</tr>
<tr>
<td align="left">{“FiltT0”:9}</td>
<td align="left">set the earliest hour (9:00 in this example) to run filtration pump. Filtration pump will not run beofre that hour</td>
</tr>
<tr>
<td align="left">{“FiltT1”:20}</td>
<td align="left">set the latest hour (20:00 in this example) to run filtration pump. Filtration pump will not run after that hour</td>
</tr>
<tr>
<td align="left">{“PubPeriod”:30}</td>
<td align="left">set the periodicity (in seconds) at which the system measurement (pumps states, tank levels states, measured values, etc) will be published to the MQTT broker</td>
</tr>
<tr>
<td align="left">{“PumpsMaxUp”:1800}</td>
<td align="left">set the Max Uptime (in secs) for the Ph and Chl pumps over a 24h period. If over, PID regulation is stopped and a warning flag is raised</td>
</tr>
<tr>
<td align="left">{“Clear”:1}</td>
<td align="left">reset the pH and Orp pumps overtime error flags in order to let the regulation loops continue. “Mode” also needs to be switched back to Auto (1) after an error flag was raised</td>
</tr>
<tr>
<td align="left">{“DelayPID”:30}</td>
<td align="left">Delay (in mins) after FiltT0 before the PID regulation loops will start. This is to let the Orp and pH readings stabilize first. 30mins in this example. Should not be &gt; 59mins</td>
</tr>
<tr>
<td align="left">{“TempExt”:4.2}</td>
<td align="left">Provide the system with the external temperature. Should be updated regularly and will be used to start filtration when temperature is less than 2°C. 4.2deg in this example</td>
</tr>
<tr>
<td align="left">{“PSIHigh”:1.0}</td>
<td align="left">set the water high-pressure threshold (1.0bar in this example). When water pressure is over that threshold, an error flag is set</td>
</tr>
<tr>
<td align="left">{“pHTank”:[20,100]}</td>
<td align="left">call this function when the Acid tank is replaced or refilled. First parameter is the tank volume in Liters, second parameter is its percentage fill (100% when full)</td>
</tr>
<tr>
<td align="left">{“ChlTank”:[20,100]}</td>
<td align="left">call this function when the Chlorine tank is replaced or refilled. First parameter is the tank volume in Liters, second parameter is its percentage fill (100% when full)</td>
</tr>
<tr>
<td align="left">{“Relay”:[1,1]}</td>
<td align="left">call this generic command to actuate spare relays. Parameter 1 is the relay number (R1 in this example), parameter 2 is the relay state (ON in this example). This command is useful to use spare relays for additional features (lighting, etc). Available relay numbers are 1 and 2</td>
</tr>
<tr>
<td align="left">{“Reboot”:1}</td>
<td align="left">call this command to reboot the controller (after 8 seconds from calling this command)</td>
</tr>
<tr>
<td align="left">{“Settings”:1}</td>
<td align="left">force settings (Set1-Set5) publication to MQTT to refresh values</td>
</tr>
<tr>
<td align="left">{“pHPumpFR”:1.5}</td>
<td align="left">call this command to set pH pump flow rate un L/s. In this example 1.5L/s</td>
</tr>
<tr>
<td align="left">{“ChlPumpFR”:3}</td>
<td align="left">call this command to set Chl pump flow rate un L/s. In this example 3L/s</td>
</tr>
<tr>
<td align="left">{“RstpHCal”:1}</td>
<td align="left">call this command to reset the calibration coefficients of the pH probe</td>
</tr>
<tr>
<td align="left">{“RstOrpCal”:1}</td>
<td align="left">call this command to reset the calibration coefficients of the Orp probe</td>
</tr>
<tr>
<td align="left">{“RstPSICal”:1}</td>
<td align="left">call this command to reset the calibration coefficients of the pressure sensor</td>
</tr>
</tbody>
</table>
