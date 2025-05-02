#pragma once
#define ARDUINOJSON_USE_DOUBLE 1  // Required to force ArduinoJSON to treat float as double

#include "Arduino_DebugUtils.h"   // Debug.print
#include <time.h>                 // Struct and function declarations for dealing with time
#include "TimeLib.h"              // Low level time and date functions
#include <RunningMedian.h>        // Determine the running median by means of a circular buffer
#include <PID_v1.h>               // PID regulation loop
#include "OneWire.h"              // Onewire communication
#include <Wire.h>                 // Two wires / I2C library
#include <stdlib.h>               // Definitions  for common types, variables, and functions
#include <vector>                 // List vectors
#include <ArduinoJson.h>          // JSON library
#include <Pump.h>                 // Simple library to handle home-pool filtration and peristaltic pumps
//#include <Relay.h>                 // Simple library to handle home-pool filtration and peristaltic pumps
#include <DallasTemperature.h>    // Maxim (Dallas DS18B20) Temperature temperature sensor library
#include <esp_task_wdt.h>         // ESP task management library
#include <Preferences.h>          // Non Volatile Storage management (ESP)
#include <WiFi.h>                 // ESP32 Wifi support
#include <WiFiClient.h>           // Base class that provides Client
#include <WiFiUdp.h>              // UDP support
#include <ESPmDNS.h>              // mDNS
#include <ArduinoOTA.h>           // Over the Air WiFi update 
#include "AsyncMqttClient.h"      // Async. MQTT client
#include "ADS1115.h"              // ADS1115 sensors library
#include "Helpers.h"
#ifdef ELEGANT_OTA
#include <ESPAsyncWebServer.h>            // Used for ElegantOTA
#include <ElegantOTA.h>
#endif

#define NO_INTERLOCK 255
struct StorePumpConfig
{
  uint8_t pin_number, pin_direction, pin_interlock;
  bool pin_active_level, relay_operation_mode;
  double pump_flow_rate, tank_vol, tank_fill;
  uint8_t tank_level_pin;
  unsigned long pump_min_uptime, pump_max_uptime; 
} ;

// General shared data structure
struct StoreStruct
{
  uint8_t ConfigVersion;   // This is for testing if first time using eeprom or not
  bool Ph_RegulationOnOff, Orp_RegulationOnOff, AutoMode, WinterMode;
  uint8_t FiltrationDuration, FiltrationStart, FiltrationStop, FiltrationStartMin, FiltrationStopMax, DelayPIDs;
  //unsigned long PhPumpUpTimeLimit, ChlPumpUpTimeLimit;
  unsigned long PublishPeriod;
  unsigned long PhPIDWindowSize, OrpPIDWindowSize, PhPIDwindowStartTime, OrpPIDwindowStartTime;
  double Ph_SetPoint, Orp_SetPoint, PSI_HighThreshold, PSI_MedThreshold, WaterTempLowThreshold, WaterTemp_SetPoint, AirTemp, pHCalibCoeffs0, pHCalibCoeffs1, OrpCalibCoeffs0, OrpCalibCoeffs1, PSICalibCoeffs0, PSICalibCoeffs1;
  double Ph_Kp, Ph_Ki, Ph_Kd, Orp_Kp, Orp_Ki, Orp_Kd, PhPIDOutput, OrpPIDOutput, WaterTemp, PhValue, OrpValue, PSIValue;
  //double AcidFill, ChlFill, pHTankVol, ChlTankVol, pHPumpFR, ChlPumpFR;
  uint8_t SecureElectro, DelayElectro; //ajout
  bool ElectrolyseMode,pHAutoMode,OrpAutoMode, FillAutoMode;
  uint8_t Lang_Locale;
  IPAddress MQTT_IP;
  uint MQTT_PORT;
  char MQTT_LOGIN[63], MQTT_PASS[63], MQTT_ID[30], MQTT_TOPIC[50];
  char SMTP_SERVER[50];
  uint SMTP_PORT;
  char SMTP_LOGIN[63], SMTP_PASS[63], SMTP_SENDER[150], SMTP_RECIPIENT[50];
  //uint  FillingPumpMinTime,FillingPumpMaxTime;
  bool BuzzerOn;
  StorePumpConfig PumpsConfig[8]; // Table representing the configuration for Pumps
} ;

// Global status of the board
extern bool PoolMaster_BoardReady;      // Is Board Up
extern bool PoolMaster_WifiReady;      // Is Wifi Up
extern bool PoolMaster_MQTTReady;      // Is MQTT Connected
extern bool PoolMaster_NTPReady;      // Is NTP Connected
extern bool PoolMaster_FullyLoaded;      // At startup gives time for everything to start before exiting Nextion's splash screen

extern StoreStruct storage;

// For NTP Synch
extern void syncESP2RTC(uint32_t , uint32_t , uint32_t , uint32_t , uint32_t , uint32_t );
extern void syncRTC2ESP(void);

//Queue object to store incoming JSON commands (up to 10)
#define QUEUE_ITEMS_NBR 10
#define QUEUE_ITEM_SIZE 200
extern QueueHandle_t queueIn;

//The four pumps of the system (instanciate the Pump class)
//In this case, all pumps start/Stop are managed by relays
extern Pump FiltrationPump;
extern Pump PhPump;
extern Pump ChlPump;
extern Pump RobotPump;
extern Pump FillingPump;
extern Pump SWGPump;    // Pump class which control the Salt Water Chlorine Generator (switch it on and off)

extern std::vector<PIN*> Pool_Equipment;

// The Relay to activate and deactivate Orp production
extern Relay RELAYR0;
extern Relay RELAYR1;

#ifdef ELEGANT_OTA
extern AsyncWebServer server;
#endif

//PIDs instances
//Specify the links and initial tuning parameters
extern PID PhPID;
extern PID OrpPID;

extern bool PSIError;

extern tm timeinfo;

// Firmware revision
extern String Firmw;

extern AsyncMqttClient mqttClient;                     // MQTT async. client

// Various flags
extern volatile bool startTasks;                       // flag to start loop tasks       
extern bool MQTTConnection;                            // MQTT connected flag
//extern bool EmergencyStopFiltPump;                     // Filtering pump stopped manually; needs to be cleared to restart
extern bool AntiFreezeFiltering;                       // Filtration anti freeze mode
extern bool cleaning_done;      					   // Robot clean-up done   

