/*
  NEXTION TFT related code, based on EasyNextion library by Seithan / Athanasios Seitanis (https://github.com/Seithan/EasyNextionLibrary)
  The trigger(s) functions at the end are called by the Nextion library on event (buttons, page change).

  Completely reworked and simplified. These functions only update Nextion variables. The display is then updated localy by the
  Nextion itself.

  Remove every usages of String in order to avoid duplication, fragmentation and random crashes.
  Note usage of snprintf_P which uses a fmt string that resides in program memory.
*/
#include <Arduino.h>
#include "Config.h"
#include "PoolMaster.h"
#include "EasyNextionLibrary.h"

#ifdef NEXTION_V2

#define GLOBAL  "globals" // Name of Nextion page to store global variables

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

static volatile int CurrentPage = 0;
static volatile bool TFT_ON = true;           // display status (not in sleep mode at startup)

static char temp[32];
static unsigned long LastAction = 0; // Last action time done on TFT. Go to sleep after TFT_SLEEP
static char HourBuffer[9];
static char DateBuffer[11];

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
static bool PoolMaster_BoardReady = false;      // Is Board Up
static bool PoolMaster_WifiReady = false;      // Is Wifi Up
static bool PoolMaster_MQTTReady = false;      // Is MQTT Connected
static bool PoolMaster_NTPReady = false;      // Is NTP Connected
static bool PoolMaster_FullyLoaded = false;      // At startup gives time for everything to start before exiting Nextion's splash screen

//Nextion TFT object. Choose which ever Serial port
//you wish to connect to (not "Serial" which is used for debug), here Serial2 UART
static EasyNex myNex(Serial2);

// Functions prototypes
void InitTFT(void);
void ResetTFT(void);
double map(double, double, double, int, int);
void SetFullyLoaded(void);
void SetBoardReady(void);
void SetWifiReady(void);
void SetMQTTReady(void);
void SetNTPReady(void);
void UpdateTFT(void);
void UpdateWiFi(bool);


//Reset TFT at start of controller - Change transmission rate to 115200 bauds on both side (Nextion then ESP)
//could have been not in HMI file, but it is good to know that after reset the Nextion goes back to 9600 bauds
void ResetTFT()
{
  myNex.begin(115200);
  myNex.writeStr("sleep=0");
  myNex.writeStr(F("rest"));
  myNex.writeStr(F("wup=1")); // Exit from sleep on page 9 Loading
  myNex.writeStr(F("usup=1")); // Authorize auto wake up on serial data
  PoolMaster_BoardReady = false;
  PoolMaster_WifiReady = false;
  PoolMaster_MQTTReady = false;
  PoolMaster_NTPReady = false;
  PoolMaster_FullyLoaded = false;
  myNex.writeNum(F(GLOBAL".vaFullload.val"),0);
  myNex.writeNum(F(GLOBAL".vaBOARDReady.val"),0);
  myNex.writeNum(F(GLOBAL".vaWIFIReady.val"),0);
  myNex.writeNum(F(GLOBAL".vaMQTTReady.val"),0);
  myNex.writeNum(F(GLOBAL".vaNTPReady.val"),0);
  myNex.writeStr("page pageSplash");
  delay(500);
}

double map(double x, double in_min, double in_max, int out_min, int out_max) {
      if ((in_max - in_min)==0)
        return 0;
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

void InitTFT()
{
  myNex.writeNum(F(GLOBAL".vaMode.val"), storage.AutoMode);
  myNex.writeNum(F(GLOBAL".vaElectrolyse.val"), storage.ElectrolyseMode);
  myNex.writeNum(F(GLOBAL".vaFilt.val"), 0);
  myNex.writeNum(F(GLOBAL".vaRobot.val"), 0);
  myNex.writeNum(F(GLOBAL".vaR0.val"), 0);
  myNex.writeNum(F(GLOBAL".vaR1.val"), 0);
  myNex.writeNum(F(GLOBAL".vaR2.val"), 0);
  myNex.writeStr(F(GLOBAL".vaMCFW.txt"), FIRMW);
  myNex.writeStr(F(GLOBAL".vaTFTFW.txt"), TFT_FIRMW);
}

void SetFullyLoaded()
{
  PoolMaster_FullyLoaded = true;
}

void SetBoardReady()
{
  PoolMaster_BoardReady = true;
}

void SetWifiReady()
{
  PoolMaster_WifiReady = true;
}
void SetMQTTReady()
{
  PoolMaster_MQTTReady = true;
}
void SetNTPReady()
{
  PoolMaster_NTPReady = true;
}

void UpdateWiFi(bool wifi){
  if(wifi){
    snprintf_P(temp,sizeof(temp),PSTR("%s"),WiFi.SSID().c_str());
    myNex.writeStr(F(GLOBAL".vaSSID.txt"),temp);
    snprintf_P(temp,sizeof(temp),PSTR("%s"),WiFi.localIP().toString());
    myNex.writeStr(F(GLOBAL".vaIP.txt"),temp);
  } else
  {
    myNex.writeStr(F(GLOBAL".vaSSID.txt"),"Not connected");
    myNex.writeStr(F(GLOBAL".vaIP.txt"),"");
  } 
}

//Function to update TFT display
//update the global variables of the TFT + the widgets of the active page
//call this function at least every second to ensure fluid display
void UpdateTFT()
{
  // Has any button been touched? If yes, one of the trigger routines
  // will fire
  myNex.NextionListen();  

  // Updates done only if TFT ON, useless (and not taken into account) if not
  if(TFT_ON)
  {
    // Send data according to current page on Nextion
    // #      Page
    // 0      Nothing started
    // 1      Splash Screen
    // 2      Home
    // 3      Settings & Info
    // 4      Calib & Electrolyse Conf

    // Errors and switches are always sent independantly of the current page
    myNex.writeNum(F(GLOBAL".vapHLevel.val"),PhPump.TankLevel() ? 0:1);
    myNex.writeNum(F(GLOBAL".vaChlLevel.val"),ChlPump.TankLevel() ? 0:1);
    myNex.writeNum(F(GLOBAL".vaPSIErr.val"),PSIError ? 1:0);
    myNex.writeNum(F(GLOBAL".vaChlUTErr.val"),ChlPump.UpTimeError ? 1:0);
    myNex.writeNum(F(GLOBAL".vapHUTErr.val"),PhPump.UpTimeError ? 1:0); 
    sprintf(HourBuffer, PSTR("%02d:%02d:%02d"), hour(), minute(), second());
    myNex.writeStr(F(GLOBAL".vaTime.txt"),HourBuffer);
    sprintf(DateBuffer, PSTR("%02d/%02d/%02d"), day(), month(), year()-2000);
    myNex.writeStr(F(GLOBAL".vaDate.txt"),DateBuffer);
    myNex.writeNum(F(GLOBAL".vaNetW.val"),MQTTConnection ? 1:0);

    // Do not update switch status during 1s after a change of switch to allow the system to reflect it in real life
    if (((unsigned long)(millis() - LastAction) > 1000) || (CurrentPage==1))
    {
      // Update all bistable switches
      if(TFT_Automode != storage.AutoMode) 
      { myNex.writeNum(F(GLOBAL".vaMode.val"),storage.AutoMode);
        TFT_Automode = storage.AutoMode;
      }
      if(TFT_Filt != FiltrationPump.IsRunning())
      { TFT_Filt = FiltrationPump.IsRunning();
        myNex.writeNum(F(GLOBAL".vaFilt.val"), TFT_Filt);
      }
      if(TFT_Robot != RobotPump.IsRunning())
      { TFT_Robot = RobotPump.IsRunning();
        myNex.writeNum(F(GLOBAL".vaRobot.val"), TFT_Robot);
      }
      if(TFT_R0 != RELAYR0.IsActive())
      { TFT_R0 = RELAYR0.IsActive();
        myNex.writeNum(F(GLOBAL".vaR0.val"), TFT_R0);
      }
      if(TFT_R1 != RELAYR1.IsActive())
      { TFT_R1 = RELAYR1.IsActive();
        myNex.writeNum(F(GLOBAL".vaR1.val"), TFT_R1);
      }
      if(TFT_Winter != storage.WinterMode) 
      { myNex.writeNum(F(GLOBAL".vaWinter.val"),storage.WinterMode);
        TFT_Winter = storage.WinterMode;
      }  
      if(TFT_Electro != SWG.IsRunning())
      { TFT_Electro = SWG.IsRunning();
        myNex.writeNum(F(GLOBAL".vaElectroOn.val"), TFT_Electro);
      } 
      if(TFT_Electro_Mode != storage.ElectrolyseMode)
      { TFT_Electro_Mode = storage.ElectrolyseMode;
        myNex.writeNum(F(GLOBAL".vaElectrolyse.val"), TFT_Electro_Mode);
      } 
      if(TFT_pHAutoMode != storage.pHAutoMode)
      { TFT_pHAutoMode = storage.pHAutoMode;
        myNex.writeNum(F(GLOBAL".vapHAutoMode.val"), TFT_pHAutoMode);
      }
      if(TFT_OrpAutoMode != storage.OrpAutoMode)
      { TFT_OrpAutoMode = storage.OrpAutoMode;
        myNex.writeNum(F(GLOBAL".vaOrpAutoMode.val"), TFT_OrpAutoMode);
      }

      myNex.writeNum(F("pageHome.vapHInject.val"), (PhPump.IsRunning())? 1 : 0);
      myNex.writeNum(F("pageHome.vaChlInject.val"), (ChlPump.IsRunning())? 1 : 0);
    }

    if(CurrentPage==1)
    {
      // Set bootup parameters
      myNex.writeNum(F(GLOBAL".vaFullload.val"),PoolMaster_FullyLoaded);
      myNex.writeNum(F(GLOBAL".vaBOARDReady.val"),PoolMaster_BoardReady);
      myNex.writeNum(F(GLOBAL".vaWIFIReady.val"),PoolMaster_WifiReady);
      myNex.writeNum(F(GLOBAL".vaMQTTReady.val"),PoolMaster_MQTTReady);
      myNex.writeNum(F(GLOBAL".vaNTPReady.val"),PoolMaster_FullyLoaded);
    }

    if(CurrentPage==1 || CurrentPage==2)    //Splash & Home
    {
      // Home page data is loaded during splash screen to avoid lag when Home page appears
      snprintf_P(temp,sizeof(temp),PSTR("%02d-%02dh"),storage.FiltrationStart,storage.FiltrationStop);
      myNex.writeStr(F(GLOBAL".vaStaSto.txt"),temp);

      snprintf_P(temp,sizeof(temp),PSTR("%4.2f"),storage.PhValue);
      myNex.writeStr(F(GLOBAL".vapH.txt"),temp);
      snprintf_P(temp,sizeof(temp),PSTR("%3.0f"),storage.OrpValue);
      myNex.writeStr(F(GLOBAL".vaOrp.txt"),temp);
      snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.Ph_SetPoint);
      myNex.writeStr(F(GLOBAL".vapHSP.txt"),temp);
      snprintf_P(temp,sizeof(temp),PSTR("%3.0f"),storage.Orp_SetPoint);
      myNex.writeStr(F(GLOBAL".vaOrpSP.txt"),temp);
      snprintf_P(temp,sizeof(temp),PSTR("%3.0f"),storage.WaterTemp_SetPoint);
      myNex.writeStr(F(GLOBAL".vaWTSP.txt"),temp);
      snprintf_P(temp,sizeof(temp),PSTR("%3.0f"),storage.WaterTempLowThreshold);
      myNex.writeStr(F(GLOBAL".vaWTLT.txt"),temp);
      snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.PSI_MedThreshold);
      myNex.writeStr(F(GLOBAL".vaPSIMin.txt"),temp);
      snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.PSI_HighThreshold);
      myNex.writeStr(F(GLOBAL".vaPSIMax.txt"),temp);
      snprintf_P(temp,sizeof(temp),PSTR("%4.1f"),storage.WaterTemp);
      myNex.writeStr(F("pageHome.vaWT.txt"),temp);
      snprintf_P(temp,sizeof(temp),PSTR("%4.1f"),storage.AirTemp);
      myNex.writeStr(F("pageHome.vaAT.txt"),temp);
      snprintf_P(temp,sizeof(temp),PSTR("%4.2f"),storage.PSIValue);
      myNex.writeStr(F("pageHome.vaPSI.txt"),temp);

      myNex.writeNum(F("pageHome.vapHPID.val"), (PhPID.GetMode() == AUTOMATIC)? 1 : 0);
      myNex.writeNum(F("pageHome.vaOrpPID.val"), (OrpPID.GetMode() == AUTOMATIC)? 1 : 0);

      // Update all gauge values
      long temp_gauge; 
      temp_gauge = map(storage.PhValue, storage.Ph_SetPoint - 0.5, storage.Ph_SetPoint + 0.5, 0, 154);
      temp_gauge = constrain(temp_gauge, 0, 147);
      myNex.writeNum(F("pageHome.vaPercArrowPH.val"), temp_gauge);
      temp_gauge = map(storage.OrpValue, storage.Orp_SetPoint - 50, storage.Orp_SetPoint + 50, 0, 154);
      temp_gauge = constrain(temp_gauge, 0, 147);
      myNex.writeNum(F("pageHome.vaPercArrowORP.val"), temp_gauge);
      temp_gauge = map(storage.WaterTemp, storage.WaterTempLowThreshold, storage.WaterTemp_SetPoint, 37, 110);
      temp_gauge = constrain(temp_gauge, 0, 147);
      myNex.writeNum(F("pageHome.vaPercArrowT.val"), temp_gauge);
      if (storage.PSIValue <= storage.PSI_MedThreshold) {
        temp_gauge = map(storage.PSIValue, 0, storage.PSI_MedThreshold, 0, 24);
        temp_gauge = constrain(temp_gauge, 0, 78);        
        myNex.writeNum(F("pageHome.vaPercArrowP.val"), temp_gauge);
      } else {
        temp_gauge = map(storage.PSIValue, storage.PSI_MedThreshold, storage.PSI_HighThreshold, 25, 63);
        temp_gauge = constrain(temp_gauge, 0, 78);     
        myNex.writeNum(F("pageHome.vaPercArrowP.val"), temp_gauge);
      }

      if(abs(storage.PhValue-storage.Ph_SetPoint) <= 0.1) 
        myNex.writeNum(F("pageHome.vapHErr.val"),0);
      if(abs(storage.PhValue-storage.Ph_SetPoint) > 0.1 && abs(storage.PhValue-storage.Ph_SetPoint) <= 0.2)  
        myNex.writeNum(F("pageHome.vapHErr.val"),1);
      if(abs(storage.PhValue-storage.Ph_SetPoint) > 0.2)  
        myNex.writeNum(F("pageHome.vapHErr.val"),2);
      if(abs(storage.OrpValue-storage.Orp_SetPoint) <= 20.) 
        myNex.writeNum(F("pageHome.vaOrpErr.val"),0);
      if(abs(storage.OrpValue-storage.Orp_SetPoint) > 20. && abs(storage.OrpValue-storage.Orp_SetPoint) <= 40.)  
        myNex.writeNum(F("pageHome.vaOrpErr.val"),1);
      if(abs(storage.OrpValue-storage.Orp_SetPoint) > 40.)  
        myNex.writeNum(F("pageHome.vaOrpErr.val"),2);        
    }

    if(CurrentPage == 3)     //Settings
    {  
      //snprintf_P(temp,sizeof(temp),PSTR("%d%% / %4.1fmin"),(int)round(PhPump.GetTankFill()),float(PhPump.UpTime)/1000./60.);
      snprintf_P(temp,sizeof(temp),PSTR("%4.1fmn"),float(PhPump.UpTime)/1000./60.);
      myNex.writeStr(F("pageSettings.vapHTk.txt"),temp);
      //snprintf_P(temp,sizeof(temp),PSTR("%d%% / %4.1fmin"),(int)round(ChlPump.GetTankFill()),float(ChlPump.UpTime)/1000./60.);
      snprintf_P(temp,sizeof(temp),PSTR("%4.1fmn"),float(ChlPump.UpTime)/1000./60.);
      myNex.writeStr(F("pageSettings.vaChlTk.txt"),temp);

      myNex.writeNum(F("pageSettings.vapHGauge.val"), (int)(round(PhPump.GetTankFill())));
      myNex.writeNum(F("pageSettings.vaChlGauge.val"), (int)(round(ChlPump.GetTankFill())));      
      UpdateWiFi(WiFi.status() == WL_CONNECTED);
      myNex.writeStr(F(GLOBAL".vaMCFW.txt"),FIRMW);
      myNex.writeStr(F(GLOBAL".vaTFTFW.txt"),TFT_FIRMW);
    }  

    if(CurrentPage == 4)      //Calib & Electrolyse
    {
      snprintf_P(temp,sizeof(temp),PSTR("%4.2f"),storage.PhValue);
      myNex.writeStr(F(GLOBAL".vapH.txt"),temp);
      snprintf_P(temp,sizeof(temp),PSTR("%3.0f"),storage.OrpValue);
      myNex.writeStr(F(GLOBAL".vaOrp.txt"),temp);
      myNex.writeNum(F(GLOBAL".vaElectroSec.val"), storage.SecureElectro);
      myNex.writeNum(F(GLOBAL".vaElectroDelay.val"), storage.DelayElectro);
    }

    if(CurrentPage == 5)      //Keypad & Keyboard
    {

    }
    //put TFT in sleep mode with wake up on touch and force page 0 load to trigger an event
    if((unsigned long)(millis() - LastAction) >= TFT_SLEEP && TFT_ON && CurrentPage !=4 && CurrentPage !=5)
    {
      myNex.writeStr(F("thup=1"));
      myNex.writeStr(F("wup=1"));     // Wake up on page 9 SplashScreen
      myNex.writeStr(F("usup=0"));    // Authorize auto wake up on serial data
      myNex.writeStr(F("sleep=1"));
      TFT_ON = false;
    }
  }  
}

//Page 1 has finished loading - SplashScreen
//printh 23 02 54 01
void trigger1()
{
  CurrentPage = 1;
  if(!TFT_ON)
  {
    UpdateWiFi(WiFi.status() == WL_CONNECTED);
    TFT_ON = true;  
  }
  LastAction = millis();
}

//Page 2 has finished loading - Home
//printh 23 02 54 02
void trigger2()
{
  CurrentPage = 2;
  LastAction = millis();
}

//Page 3 has finished loading - Settings & Info
//printh 23 02 54 03
void trigger3()
{
  CurrentPage = 3;
  LastAction = millis();  
}

//Page 3 has finished loading - Calib & Electrolyse
//printh 23 02 54 04
void trigger4()
{
  CurrentPage = 4;
  LastAction = millis();
}

//MODE button was toggled
//printh 23 02 54 05
void trigger5()
{
  char Cmd[100] = "{\"Mode\":1}";
  if(storage.AutoMode) Cmd[8] = '0';
  xQueueSendToBack(queueIn,&Cmd,0);
  LastAction = millis();
}

//FILT button was toggled
//printh 23 02 54 06
void trigger6()
{
  char Cmd[100] = "{\"FiltPump\":1}";
  if(FiltrationPump.IsRunning()) Cmd[12] = '0';
  else TFT_Filt = true;
  xQueueSendToBack(queueIn,&Cmd,0);
  LastAction = millis();
}

//Robot button was toggled
//printh 23 02 54 07
void trigger7()
{
  char Cmd[100] = "{\"RobotPump\":1}";
  if(RobotPump.IsRunning()) Cmd[13] = '0';
  else TFT_Robot = true;
  xQueueSendToBack(queueIn,&Cmd,0);
  LastAction = millis();
}

//Relay 0 button was toggled
//printh 23 02 54 08
void trigger8()
{
  char Cmd[100] = "{\"Relay\":[0,1]}";
  if(RELAYR0.IsActive()) Cmd[12] = '0';
  else TFT_R0 = true; 
  xQueueSendToBack(queueIn,&Cmd,0);
  LastAction = millis();
}

//Relay 1 button was toggled
//printh 23 02 54 09
void trigger9()
{
  char Cmd[100] = "{\"Relay\":[1,1]}";
  if (RELAYR1.IsActive()) Cmd[12] = '0';
  else TFT_R1 = true;
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

//Winter button was toggled
//printh 23 02 54 0A
void trigger10()
{
  char Cmd[100] = "{\"Winter\":1}";
  if(storage.WinterMode) Cmd[10] = '0';
  else TFT_Winter = true;
  xQueueSendToBack(queueIn,&Cmd,0);
  LastAction = millis();
}

//Probe calibration completed or new pH, Orp or Water Temp setpoints or New tank
//printh 23 02 54 0B
void trigger11()
{
  char Cmd[100] = "";
  strcpy(Cmd,myNex.readStr(F(GLOBAL".vaCommand.txt")).c_str());
  xQueueSendToBack(queueIn,&Cmd,0);
  Debug.print(DBG_VERBOSE,"Nextion cal page command: %s",Cmd);
  LastAction = millis();
}

//Clear Errors button pressed
//printh 23 02 54 0C
void trigger12()
{
  char Cmd[100] = "{\"Clear\":1}";
  xQueueSendToBack(queueIn,&Cmd,0);
  LastAction = millis();
}

//pH PID button pressed
//printh 23 02 54 0D
void trigger13()
{
  char Cmd[100] = "{\"PhPID\":1}";
  if(PhPID.GetMode() == 1) Cmd[9] = '0';
  xQueueSendToBack(queueIn,&Cmd,0);
  LastAction = millis();
}

//Orp PID button pressed
//printh 23 02 54 0E
void trigger14()
{
  char Cmd[100] = "{\"OrpPID\":1}";
  if(OrpPID.GetMode() == 1) Cmd[10] = '0';
  xQueueSendToBack(queueIn,&Cmd,0);
  LastAction = millis();
}

// Electrolyse option switched
// printh 23 02 54 0F
void trigger15()
{
  char Cmd[100] = "{\"Electrolyse\":1}";
  if (SWG.IsRunning())  Cmd[15] = '0';
  else TFT_Electro = true;
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

//  pH Operation Mode option switched (normal or PID)
// printh 23 02 54 10
void trigger16()
{
  char Cmd[100] = "{\"PhAutoMode\":1}";
  if (storage.pHAutoMode) Cmd[14] = '0';
  else TFT_pHAutoMode = true;
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

//  Orp Operating Mode option switched (normal or PID)
// printh 23 02 54 11
void trigger17()
{
  char Cmd[100] = "{\"OrpAutoMode\":1}";
  if (storage.OrpAutoMode) Cmd[15] = '0';
  else TFT_OrpAutoMode = true;
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

//  Electrolyse Operating Mode option switched (Electrolyser or not)
// printh 23 02 54 12
void trigger18()
{
  char Cmd[100] = "{\"ElectrolyseMode\":1}";
  if (storage.ElectrolyseMode) Cmd[19] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

//  Turn On and Off the pH Pump
// printh 23 02 54 13
void trigger19()
{
  char Cmd[100] = "{\"PhPump\":1}";
  if (PhPump.IsRunning()) Cmd[10] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

//  Turn On and Off the Chlorine Pump
// printh 23 02 54 14
void trigger20()
{
  char Cmd[100] = "{\"ChlPump\":1}";
  if (ChlPump.IsRunning()) Cmd[11] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

//Electrolyser configuration
//printh 23 02 54 15
void trigger21()
{
  char Cmd[100] = "";
  strcpy(Cmd,myNex.readStr(F(GLOBAL".vaCommand.txt")).c_str());
  xQueueSendToBack(queueIn,&Cmd,0);
  Debug.print(DBG_VERBOSE,"Nextion cal page command: %s",Cmd);
  LastAction = millis();
}

//Page 5 has finished loading - KeyPad & KeyBoard
//printh 23 02 54 16
void trigger22()
{
  CurrentPage = 5;
  LastAction = millis();
}

//DateTime information
//printh 23 02 54 17
void trigger23()
{
  char Cmd[100] = "";
  strcpy(Cmd,myNex.readStr(F(GLOBAL".vaCommand.txt")).c_str());
  xQueueSendToBack(queueIn,&Cmd,0);
  Debug.print(DBG_VERBOSE,"Nextion set date time page command: %s",Cmd);
  LastAction = millis();
}
#endif