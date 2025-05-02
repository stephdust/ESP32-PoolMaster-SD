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
#include "HistoryStats.h"
#include "uptime.h"

#define GLOBAL  "globals" // Name of Nextion page to store global variables
#define MAX_SHOWN_NETWORKS  15  // Maximum number of scanned networks to display
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define GRAPH_PH_BASELINE  620 // +200 graph size (pH from 6.20 to 8.20)
#define GRAPH_ORP_BASELINE  600 // +200 graph size (Orp from 600 to 800)
#define GRAPH_TEMP_BASELINE  120 // +200 graph size (Temp from 12.0 to 32.0)

static char temp[32];
static char temp_command[32];
static unsigned long LastAction = 0; // Last action time done on TFT. Go to sleep after TFT_SLEEP
static int LeftMenuPosition = 0;
static uint8_t Current_Language = 0;
const char compile_date[] = __DATE__ " " __TIME__;

static unsigned long LastWifiScan = 0; // Last Wifi Networks Scan
static unsigned long LastUpdatedHome = 0; // Last Time Home Page Updated


// Sample Data For Graphing
extern CircularBuffer<int,NUMBER_OF_HISTORY_SAMPLES> pH_Samples;
extern CircularBuffer<int,NUMBER_OF_HISTORY_SAMPLES> Orp_Samples;
extern CircularBuffer<int,NUMBER_OF_HISTORY_SAMPLES> WTemp_Samples;

// NEXTION BUTTON RESULTS
#define BUTTON_OFF  0
#define BUTTON_ON   1
#define BUTTON_AUTO 2

BaseType_t xWasDelayed;     // Return value for task delay

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
char map(int, int, int, int, int);
void syncESP2RTC(uint32_t , uint32_t , uint32_t , uint32_t , uint32_t , uint32_t );
void syncRTC2ESP(void);
void UpdateTFT(void*);
void ScanWiFiNetworks(void);
void printScannedNetworks(uint16_t);
void WriteSwitches(void);
extern void DisconnectFromWiFi(bool);
extern void reconnectToWiFi(void);
extern void mqttInit(void);
extern void mqttDisconnect(void);
void printLanguages(void);
void SetValue(const char* , int = -1, int = -1, int = -1);
void ToggleValue(const char* , int );
void graphTable(CircularBuffer<int,NUMBER_OF_HISTORY_SAMPLES>&, int = 2 , int = 0);
#endif