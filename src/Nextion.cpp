/*
  NEXTION TFT related code, based on EasyNextion library by Seithan / Athanasios Seitanis (https://github.com/Seithan/EasyNextionLibrary)
  The trigger(s) functions at the end are called by the Nextion library on event (buttons, page change).

  Completely reworked and simplified.
*/

#include <Arduino.h>
#include "Config.h"
#include "PoolMaster.h"
#include "EasyNextionLibrary.h"

static volatile int CurrentPage = 0;
static volatile bool TFT_ON = true; // display status

static String temp;
static unsigned long LastAction = 0; // Last action time done on TFT. Go to sleep after TFT_SLEEP
static char HourBuffer[9];
static char DateBuffer[11];

// Variables used to refresh buttons only on status change
static bool TFT_Automode = false;
static bool TFT_Filt = false;
static bool TFT_Robot = false;
static bool TFT_R0 = false;
static bool TFT_R1 = false;
static bool TFT_R2_Winter = false;
static bool TFT_Electro = false; // ajout
static bool TFT_ElectroMode = false; // ajout
static bool TFT_pHPIDEnabled = false; // ajout
static bool TFT_OrpPIDEnabled = false; // ajout

// Nextion TFT object. Choose which ever Serial port
// you wish to connect to (not "Serial" which is used for debug), here Serial2 UART
static EasyNex myNex(Serial2);

// Functions prototypes
void InitTFT(void);
void ResetTFT(void);
void UpdateTFT(void);
void UpdateWiFi(bool);


// Reset TFT at start of controller - Change transmission rate to 115200 bauds on both side (Nextion then ESP)
// could have been not in HMI file, but it is good to know that after reset the Nextion goes back to 9600 bauds
void ResetTFT()
{
  myNex.begin(115200);
  myNex.writeStr("sleep=0");
  myNex.writeStr(F("rest"));
  myNex.writeStr(F("wup=1")); // Exit from sleep on page 1
  delay(1000);
}

void InitTFT()
{
  myNex.writeNum(F("pageHome.vaMode.val"), storage.AutoMode);
  myNex.writeNum(F("storage.vaElectrolyse.val"), storage.ElectrolyseMode);
  myNex.writeNum(F("pageSwitch.vaFilt.val"), 0);
  myNex.writeNum(F("pageSwitch.vaRobot.val"), 0);
  myNex.writeNum(F("pageSwitch.vaR0.val"), 0); // could be light
  myNex.writeNum(F("pageSwitch.vaR1.val"), 0); // could be relay ou electrolyser
  myNex.writeNum(F("pageSwitch.vaR2.val"), 0); // winter mode
  myNex.writeStr(F("storage.vaMCFW.txt"), FIRMW);
  myNex.writeStr(F("storage.vaTFTFW.txt"), TFT_FIRMW);
}

void UpdateWiFi(bool wifi)
{
  if (wifi)
  {
    temp = "WiFi: " + WiFi.SSID();
    myNex.writeStr("storage.vaSSID.txt", temp);
    temp = "IP: " + WiFi.localIP().toString();
    myNex.writeStr("storage.vaIP.txt", temp);
  }
  else
  {
    myNex.writeStr("storage.vaSSID.txt", "Not connected");
    myNex.writeStr("storage.vaIP.txt", "");
  }
}

// Function to update TFT display
// update the global variables of the TFT + the widgets of the active page
// call this function at least every second to ensure fluid display
void UpdateTFT()
{
  // Has any button been touched? If yes, one of the trigger routines
  // will fire
  myNex.NextionListen();

  // Updates done only if TFT ON, useless (and not taken into account) if not
  if (TFT_ON)
  {
    sprintf(HourBuffer, "%02d:%02d:%02d", hour(), minute(), second());
    myNex.writeStr("pageHome.vaTime.txt", HourBuffer);
    sprintf(DateBuffer, "%02d/%02d/%04d", day(), month(), year());
    myNex.writeStr("pageHome.vaDate.txt", DateBuffer);
    myNex.writeNum("pageHome.vaNetW.val", MQTTConnection ? 1 : 0);
    temp = String(storage.FiltrationStart) + F("/") + String(storage.FiltrationStop) + F("h");
    myNex.writeStr(F("pageHome.vaStaSto.txt"), temp);

    // Need to update values in status bar independantly of the active page
    if (TFT_Automode != storage.AutoMode)
    { TFT_Automode = storage.AutoMode;
      myNex.writeNum(F("pageHome.vaMode.val"), storage.AutoMode);
    }
    if (TFT_R2_Winter != storage.WinterMode)
    { TFT_R2_Winter = storage.WinterMode;
      myNex.writeNum(F("pageSwitch.vaR2.val"), storage.WinterMode);
    }
    if(TFT_R0 != RELAYR0.IsRunning())
      { TFT_R0 = RELAYR0.IsRunning();
        myNex.writeNum(F("pageSwitch.vaR0.val"), TFT_R0);
      }
    if(TFT_pHPIDEnabled != storage.pHPIDEnabled)
      { TFT_pHPIDEnabled = storage.pHPIDEnabled;
        myNex.writeNum(F("storage.vapHPIDEnable.val"), TFT_R0);
      }
    if(TFT_OrpPIDEnabled != storage.OrpPIDEnabled)
      { TFT_OrpPIDEnabled = storage.OrpPIDEnabled;
        myNex.writeNum(F("storage.vaOrpPIDEnable.val"), TFT_R0);
      }


    myNex.writeNum(F("pageHome.vapHLevel.val"), PhPump.TankLevel() ? 0 : 1);
    myNex.writeNum(F("pageHome.vaChlLevel.val"), ChlPump.TankLevel() ? 0 : 1);
    myNex.writeNum(F("pageHome.vaPSIErr.val"), PSIError ? 1 : 0);
    myNex.writeNum(F("pageHome.vaChlUTErr.val"), ChlPump.UpTimeError ? 1 : 0);
    myNex.writeNum(F("pageHome.vapHUTErr.val"), PhPump.UpTimeError ? 1 : 0);

    if (CurrentPage == 0)
    {
      myNex.writeStr(F("pageHome.vapH.txt"), String(storage.PhValue, 2));
      myNex.writeStr(F("pageHome.vaOrp.txt"), String(storage.OrpValue, 0));
      temp = String(storage.Ph_SetPoint, 1);
      myNex.writeStr(F("pageHome.vapHSP.txt"), temp);
      temp = String((int)storage.Orp_SetPoint);
      myNex.writeStr(F("pageHome.vaOrpSP.txt"), temp);
      
      myNex.writeNum(F("pageHome.vapHPID.val"), (PhPID.GetMode() == AUTOMATIC)? 1 : 0);
      myNex.writeNum(F("pageHome.vaOrpPID.val"), (OrpPID.GetMode() == AUTOMATIC)? 1 : 0);
      myNex.writeNum(F("pageHome.vapHInject.val"), (PhPump.IsRunning())? 1 : 0);
      myNex.writeNum(F("pageHome.vaChlInject.val"), (ChlPump.IsRunning())? 1 : 0);

      temp = String(storage.TempValue, 1) + (char)176 + F("C");
      myNex.writeStr(F("pageHome.vaWT.txt"), temp);
      temp = String(storage.TempExternal, 1) + (char)176 + F("C");
      myNex.writeStr(F("pageHome.vaAT.txt"), temp);
      temp = String(storage.PSIValue, 2) + F("b");
      myNex.writeStr(F("pageHome.vaPSI.txt"), temp);
      //temp = String((int)round(PhPump.GetTankFill())) + (char)37 + F(" / ") + String(float(PhPump.UpTime) / 1000. / 60., 1) + F("min");
      temp = String(float(PhPump.UpTime) / 1000. / 60., 0) + F("mn");
      myNex.writeStr(F("pageHome.vapHTk.txt"), temp);
      //temp = String((int)round(ChlPump.GetTankFill())) + (char)37 + F(" / ") + String(float(ChlPump.UpTime) / 1000. / 60., 1) + F("min");
      temp = String(float(ChlPump.UpTime) / 1000. / 60., 0) + F("mn");
      myNex.writeStr(F("pageHome.vaChlTk.txt"), temp);

      // Added for graphic Gauge
      myNex.writeNum("pageHome.va0pHGauge.val", (int)(round(PhPump.GetTankFill())));
      myNex.writeNum("pageHome.va0ChlGauge.val", (int)(round(ChlPump.GetTankFill())));

      // pH and Orp color based on set point
      if (abs(storage.PhValue - storage.Ph_SetPoint) <= 0.1)
        myNex.writeNum("pageHome.vapHErr.val", 0);
      if (abs(storage.PhValue - storage.Ph_SetPoint) > 0.1 && abs(storage.PhValue - storage.Ph_SetPoint) <= 0.2)
        myNex.writeNum("pageHome.vapHErr.val", 1);
      if (abs(storage.PhValue - storage.Ph_SetPoint) > 0.2)
        myNex.writeNum("pageHome.vapHErr.val", 2);
      if (abs(storage.OrpValue - storage.Orp_SetPoint) <= 20.)
        myNex.writeNum("pageHome.vaOrpErr.val", 0);
      if (abs(storage.OrpValue - storage.Orp_SetPoint) > 20. && abs(storage.OrpValue - storage.Orp_SetPoint) <= 40.)
        myNex.writeNum("pageHome.vaOrpErr.val", 1);
      if (abs(storage.OrpValue - storage.Orp_SetPoint) > 40.)
        myNex.writeNum("pageHome.vaOrpErr.val", 2);
    }

    if (CurrentPage == 1)
    {
      if(TFT_Filt != FiltrationPump.IsRunning())
      { TFT_Filt = FiltrationPump.IsRunning();
        myNex.writeNum(F("pageSwitch.vaFilt.val"), TFT_Filt);
      }
      if(TFT_Robot != RobotPump.IsRunning())
      { TFT_Robot = RobotPump.IsRunning();
        myNex.writeNum(F("pageSwitch.vaRobot.val"), TFT_Robot);
      }
      if(TFT_R1 != RELAYR1.IsRunning())
      { TFT_R1 = RELAYR1.IsRunning();
        myNex.writeNum(F("pageSwitch.vaR1.val"), TFT_R1);
      }
      if(TFT_Electro != OrpProd.IsRunning())
      { TFT_Electro = OrpProd.IsRunning();
        myNex.writeNum(F("pageSwitch.vaElectroOn.val"), TFT_Electro);
      }  
    }

    if (CurrentPage == 1||CurrentPage == 2)
    {
        myNex.writeNum("storage.vaElectrolyse.val", storage.ElectrolyseMode);
    }

    if (CurrentPage == 2)
    { UpdateWiFi(WiFi.status() == WL_CONNECTED);
      myNex.writeStr(F("storage.vaMCFW.txt"), FIRMW);
      myNex.writeStr(F("storage.vaTFTFW.txt"), TFT_FIRMW);
      myNex.writeStr(F("pageHome.vapH.txt"), String(storage.PhValue, 2));
      myNex.writeStr(F("pageHome.vaOrp.txt"), String(storage.OrpValue, 0));
      myNex.writeNum("storage.vaElectroSec.val", storage.SecureElectro);
      myNex.writeNum("storage.vaElectroDelay.val", storage.DelayElectro);
    }

    // put TFT in sleep mode with wake up on touch and force page 1 load to trigger an event
    if ((unsigned long)(millis() - LastAction) >= TFT_SLEEP && TFT_ON && CurrentPage != 4)  // Unless we are on page 4 (calib)
    {
      myNex.writeStr("thup=1");
      myNex.writeStr("wup=1");    // Wake Up On page 1
      myNex.writeStr("sleep=1");
      TFT_ON = false;
    }
  }
}

// Page 0 has finished loading  HOME
// printh 23 02 54 01
void trigger1()
{
  CurrentPage = 0;
  if (!TFT_ON)
  {
    UpdateWiFi(WiFi.status() == WL_CONNECTED);
    TFT_ON = true;
  }
  LastAction = millis();
}

// Page 1 has finished loading  SWITCH
// printh 23 02 54 02
void trigger2()
{
  CurrentPage = 1;
  LastAction = millis();
}

// Page 2 has finished loading  SETTINGS
// printh 23 02 54 03
void trigger3()
{
  CurrentPage = 2;
  LastAction = millis();
}

// MODE button was toggled
// printh 23 02 54 05
void trigger5()
{
  char Cmd[100] = "{\"Mode\":1}";
  if (storage.AutoMode)
    Cmd[8] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

// FILT button was toggled
// printh 23 02 54 06
void trigger6()
{
  char Cmd[100] = "{\"FiltPump\":1}";
  if (FiltrationPump.IsRunning())
  {
    Cmd[12] = '0';
  }
  else
    TFT_Filt = true;
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

// Robot button was toggled
// printh 23 02 54 07
void trigger7()
{
  char Cmd[100] = "{\"RobotPump\":1}";
  if (RobotPump.IsRunning())
  {
    Cmd[13] = '0';
  }
  else
    TFT_Robot = true;
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

// Relay 0 button was toggled
// printh 23 02 54 08
void trigger8()
{
  char Cmd[100] = "{\"Relay\":[0,1]}";
  if (RELAYR0.IsRunning())  Cmd[12] = '0';  // Test without changing TFT variable
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

// Relay 1 button was toggled
// printh 23 02 54 09
void trigger9()
{
  char Cmd[100] = "{\"Relay\":[1,1]}";
  if (RELAYR1.IsRunning())
  {
    Cmd[12] = '0';
  }
  else
    TFT_R1 = true;
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

// Winter button was toggled
// printh 23 02 54 0A
void trigger10()
{
  char Cmd[100] = "{\"Winter\":1}";
  if (storage.WinterMode)
  {
    Cmd[10] = '0';
  }
  else
    TFT_R2_Winter = true;
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

// Probe calibration completed or new pH, Orp or Water Temp setpoints or New tank
// printh 23 02 54 0B
void trigger11()
{
  char Cmd[100] = "";
  strcpy(Cmd, myNex.readStr(F("pageCalibs.vaCommand.txt")).c_str());
  xQueueSendToBack(queueIn, &Cmd, 0);
  Debug.print(DBG_VERBOSE, "Nextion cal page command: %s", Cmd);
  LastAction = millis();
}

// Clear Errors button pressed
// printh 23 02 54 0C
void trigger12()
{
  char Cmd[100] = "{\"Clear\":1}";
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

// pH PID button pressed
// printh 23 02 54 0D
void trigger13()
{
  char Cmd[100] = "{\"PhPID\":1}";
  if (PhPID.GetMode() == 1)
    Cmd[9] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

// Orp PID button pressed
// printh 23 02 54 0E
void trigger14()
{
  char Cmd[100] = "{\"OrpPID\":1}";
  if (OrpPID.GetMode() == 1)
    Cmd[10] = '0';
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

// Electrolyse option switched
// printh 23 02 54 0F
void trigger15()
{
  char Cmd[100] = "{\"Electrolyse\":1";
  if (OrpProd.IsRunning())
    Cmd[11] = '0';
  else TFT_Electro = true;
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

//  pH Operation Mode option switched (normal or PID)
// printh 23 02 54 10
void trigger16()
{
  char Cmd[100] = "{\"PhPIDEnabled\":1}";
  if (storage.pHPIDEnabled)
  {
    Cmd[10] = '0';
  }
  else
    TFT_pHPIDEnabled = true;
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

//  Orp Operating Mode option switched (normal or PID)
// printh 23 02 54 11
void trigger17()
{
  char Cmd[100] = "{\"OrpPIDEnabled\":1}";
  if (storage.OrpPIDEnabled)
  {
    Cmd[10] = '0';
  }
  else
    TFT_OrpPIDEnabled = true;
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}

//  Electrolyse Operating Mode option switched (Electrolyser or not)
// printh 23 02 54 12
void trigger18()
{
  char Cmd[100] = "{\"ElectrolyseMode\":1}";
  if (storage.ElectrolyseMode)
  {
    Cmd[10] = '0';
  }
  else
    TFT_ElectroMode = true;
  xQueueSendToBack(queueIn, &Cmd, 0);
  LastAction = millis();
}