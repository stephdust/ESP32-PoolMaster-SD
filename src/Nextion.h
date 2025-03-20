/*
  NEXTION TFT related code, based on EasyNextion library by Seithan / Athanasios Seitanis (https://github.com/Seithan/EasyNextionLibrary)
  The trigger(s) functions at the end are called by the Nextion library on event (buttons, page change).

  Completely reworked and simplified. These functions only update Nextion variables. The display is then updated localy by the
  Nextion itself.

  Remove every usages of String in order to avoid duplication, fragmentation and random crashes.
  Note usage of snprintf_P which uses a fmt string that resides in program memory.
*/
#ifndef POOLMASTER_NEXTION_H
#define POOLMASTER_NEXTION_H

#include <Arduino.h>
#include "Config.h"
#include "PoolMaster.h"
#include "EasyNextionLibrary.h"
#include "EasyNextionMenus.h"
#include "translation.h"           // Include all translated strings into flash
#include "Data.h"

#define GLOBAL  "globals" // Name of Nextion page to store global variables
#define MAX_SHOWN_NETWORKS  15  // Maximum number of scanned networks to display
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define GRAPH_PH_BASELINE  620
#define GRAPH_ORP_BASELINE  600

//static volatile int CurrentPage = 0;
//static volatile bool TFT_ON = true;           // display status (not in sleep mode at startup)

static char temp[32];
static char temp_command[32];
static unsigned long LastAction = 0; // Last action time done on TFT. Go to sleep after TFT_SLEEP
static int LeftMenuPosition = 0;
static uint8_t Current_Language = 0;
const char compile_date[] = __DATE__ " " __TIME__;

static unsigned long LastWifiScan = 0; // Last Wifi Networks Scan

// Sample Data For Graphing
extern CircularBuffer<int,NUMBER_OF_HISTORY_SAMPLES> pH_Samples;
extern CircularBuffer<int,NUMBER_OF_HISTORY_SAMPLES> Orp_Samples;
extern CircularBuffer<int,NUMBER_OF_HISTORY_SAMPLES> WTemp_Samples;

// Variables used to refresh buttons only on status change

static bool TFT_Automode = false;
static bool TFT_Filt = false;
static bool TFT_Robot = false;
static bool TFT_R0 = false;
static bool TFT_R1 = false;
static bool TFT_Winter = false;
static bool TFT_Electro = false; // ajout
static bool TFT_Electro_Mode = false; // ajout
static bool TFT_pHAutoMode = false; // ajout
static bool TFT_OrpAutoMode = false; // ajout
static uint32_t MASK_AUTOMODE = 512;
static uint32_t MASK_FILT     = 256;
static uint32_t MASK_ROBOT    = 128;
static uint32_t MASK_R0       = 64;
static uint32_t MASK_R1       = 32;
static uint32_t MASK_WINTER   = 16;
static uint32_t MASK_ELECTRO  = 8;
static uint32_t MASK_ELEC_MOD = 4;
static uint32_t MASK_PH_AUTO  = 2;
static uint32_t MASK_ORP_AUTO = 1;

BaseType_t xWasDelayed;     // Return value for task delay

static bool PoolMaster_BoardReady = false;      // Is Board Up
static bool PoolMaster_WifiReady = false;      // Is Wifi Up
static bool PoolMaster_MQTTReady = false;      // Is MQTT Connected
static bool PoolMaster_NTPReady = false;      // Is NTP Connected
static bool PoolMaster_FullyLoaded = false;      // At startup gives time for everything to start before exiting Nextion's splash screen

static bool MainMenuLoaded = false;
static bool SubMenuLoaded = false;

// Used for tasks
void stack_mon(UBaseType_t&);
//Nextion TFT object. Choose which ever Serial port
//you wish to connect to (not "Serial" which is used for debug), here Serial2 UART
static EasyNex myNex(Serial2);
static EasyNextionMenus  MainMenu(&myNex,15,ENM_MAIN);
static EasyNextionMenus  SubMenu1(&myNex,7,ENM_SUB);
static EasyNextionMenus  SubMenu2(&myNex,7,ENM_SUB);
static EasyNextionMenus  SubMenu3(&myNex,7,ENM_SUB);
static EasyNextionMenus  SubMenu4(&myNex,7,ENM_SUB);
static EasyNextionMenus  SubMenu5(&myNex,7,ENM_SUB);
static EasyNextionMenus  SubMenu6(&myNex,7,ENM_SUB);
static EasyNextionMenus  SubMenu7(&myNex,7,ENM_SUB);
static EasyNextionMenus  SubMenu8(&myNex,7,ENM_SUB);
static EasyNextionMenus  SubMenu9(&myNex,7,ENM_SUB);
static EasyNextionMenus  SubMenu10(&myNex,7,ENM_SUB);
static EasyNextionMenus  SubMenu11(&myNex,7,ENM_SUB);
static EasyNextionMenus  SubMenu12(&myNex,7,ENM_SUB);

// Functions prototypes
void InitMenu(void);
void ResetTFT(void);
double map(double, double, double, int, int);
void SetFullyLoaded(void);
void SetBoardReady(void);
void SetWifiReady(bool);
void SetMQTTReady(bool);
void SetNTPReady(bool);
void UpdateTFT(void*);
void ScanWiFiNetworks(void);
void printScannedNetworks(uint16_t);
void WriteSwitches(void);
extern void DisconnectFromWiFi(bool);
extern void reconnectToWiFi(void);
extern void mqttInit(void);
extern void mqttDisconnect(void);
void printLanguages(void);

#endif