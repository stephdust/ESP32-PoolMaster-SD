/*
  NEXTION TFT related code, based on EasyNextion library by Seithan / Athanasios Seitanis (https://github.com/Seithan/EasyNextionLibrary)
  The trigger(s) functions at the end are called by the Nextion library on event (buttons, page change).

  Completely reworked and simplified. These functions only update Nextion variables. The display is then updated localy by the
  Nextion itself.

  Remove every usages of String in order to avoid duplication, fragmentation and random crashes.
  Note usage of snprintf_P which uses a fmt string that resides in program memory.
*/
#include "Nextion.h"

//Reset TFT at start of controller - Change transmission rate to 115200 bauds on both side (Nextion then ESP)
//could have been not in HMI file, but it is good to know that after reset the Nextion goes back to 9600 bauds
void ResetTFT()
{
  myNex.begin(115200);
  myNex.writeStr("sleep=0");
  myNex.writeStr(F("rest"));
  myNex.writeStr(F("wup=1")); // Exit from sleep on last page
  myNex.writeStr(F("usup=1")); // Authorize auto wake up on serial data
  myNex.writeStr("page pageSplash");
  delay(500);
  InitMenu();
}

// Read and Write the boolean values stored in a 32 bits number
// to be sent to Nextion in one shot
void WriteSwitches()
{
  uint32_t switches_bitmap = 0;

  switches_bitmap |= (PoolMaster_FullyLoaded & 1)   << 31;     //2 147 483 648
  switches_bitmap |= (PoolMaster_BoardReady & 1)    << 30;     //1 073 741 824
  switches_bitmap |= (PoolMaster_WifiReady & 1)     << 29;     //536 870 912
  switches_bitmap |= (PoolMaster_MQTTReady & 1)     << 28;     //268 435 456
  switches_bitmap |= (PoolMaster_FullyLoaded & 1)   << 27;     //134 217 728
  switches_bitmap |= (MQTTConnection & 1)           << 26;     //67 108 864

  switches_bitmap |= (!PhPump.TankLevel() & 1)      << 25;     //33 554 432
  switches_bitmap |= (!ChlPump.TankLevel() & 1)     << 24;     //16 777 216
  switches_bitmap |= (PSIError & 1)                 << 23;     //8 388 608
  switches_bitmap |= (PhPump.UpTimeError & 1)       << 22;     //4 194 304
  switches_bitmap |= (ChlPump.UpTimeError & 1)      << 21;     //2 097 152
//  switches_bitmap |= (PhPump.IsRunning() & 1)       << 20;   //1 048 576
  switches_bitmap |= (PhPID.GetMode() & 1)          << 13;     //8192
  switches_bitmap |= (OrpPID.GetMode() & 1)         << 12;     //4096
  switches_bitmap |= (PhPump.IsRunning() & 1)       << 11;     //2048
  switches_bitmap |= (ChlPump.IsRunning() & 1)      << 10;     //1024
  switches_bitmap |= (storage.AutoMode & 1)         << 9;      //512
  switches_bitmap |= (FiltrationPump.IsRunning() & 1) << 8;    //256
  switches_bitmap |= (RobotPump.IsRunning() & 1)    << 7;      //128
  switches_bitmap |= (RELAYR0.IsActive() & 1)       << 6;      //64
  switches_bitmap |= (RELAYR1.IsActive() & 1)       << 5;      //32
  switches_bitmap |= (storage.WinterMode & 1)       << 4;      //16
  switches_bitmap |= (SWG.IsRunning() & 1)          << 3;      //8
  switches_bitmap |= (storage.ElectrolyseMode & 1)  << 2;      //4
  switches_bitmap |= (storage.pHAutoMode & 1)       << 1;      //2
  switches_bitmap |= (storage.OrpAutoMode & 1)      << 0;      //1

  myNex.writeNum(F(GLOBAL".vaSwitches.val"),switches_bitmap);
}


//Function to update TFT display
//update the global variables of the TFT + the widgets of the active page
//call this function at least every second to ensure fluid display
void UpdateTFT(void *pvParameters)
{
  static UBaseType_t hwm=0;     // free stack size

  while(!startTasks);
  vTaskDelay(DT10);                                // Scheduling offset 

  esp_task_wdt_add(nullptr);
  TickType_t period = PT10;  
  TickType_t ticktime = xTaskGetTickCount(); 

  #ifdef CHRONO
  unsigned long td;
  int t_act=0,t_min=999,t_max=0;
  float t_mean=0.;
  int n=1;
  #endif

  for(;;)
  {
    // reset watchdog
    esp_task_wdt_reset();

    #ifdef CHRONO
    td = millis();
    #endif    

    // Has any button been touched? If yes, one of the trigger routines
    // will fire.
    // Return  0 if Nextion is sleeping 
    //         1 if Nextion Up
    if(myNex.NextionListen())
    {
      // Send data according to current page on Nextion
      // #      Page
      // 0      Nothing started
      // 1      Splash Screen
      // 2      Home
      // 3      Settings
      // 4      Calib
      // 5      KeyPad & Keyboard
      // 6      Wifi Scan
      // 7      Electrolyse Config
      // 8      New Tank
      // 9      Info
      // 10     Set Date/Time
      // 11     Wifi Configuration
      // 12     pH & Orp tank status POPup
      // 13     Graph and History Stats
      // 14     Home Page Simplified
      // 15     Language Selection
      // 16     MQTT Configuration
      // 17-24  Overlay Control

      // 30     Help Screen PoPup


      // Do not update switch status during 1s after a change of switch to allow the system to reflect it in real life
      // avoids switches to flicker on/off on the screen
      if (((unsigned long)(millis() - myNex.LastActionMillis) > 1000) || (myNex.currentPageId==1))
      {
        WriteSwitches();
      }

      if(myNex.currentPageId==1 || myNex.currentPageId==2)    //Splash & Home
      {
        // Home page data is loaded during splash screen to avoid lag when Home page appears
        snprintf_P(temp,sizeof(temp),PSTR("%02d-%02dh"),storage.FiltrationStart,storage.FiltrationStop);
        myNex.writeStr(F(GLOBAL".vaStaSto.txt"),temp);
        sprintf(temp, PSTR("%02d:%02d:%02d"), hour(), minute(), second());
        myNex.writeStr(F(GLOBAL".vaTime.txt"),temp);
        sprintf(temp, PSTR("%02d/%02d/%02d"), day(), month(), year()-2000);
        myNex.writeStr(F(GLOBAL".vaDate.txt"),temp);
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

        // Send pH and ORP Errors
        // 0 if within +/- 0.1 of the setpoint  (20 for ORP)
        // 1 or -1 if between 0.1 and 0.2 away from setpoint (20 and 40 for ORP)
        // 2 or -2 if more than 0.2 from the setpoint (more than 40 for ORP)

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

      if(myNex.currentPageId == 3)     //Settings Menu
      {
        period=PT10/3;
        
        // Rebuild menu if language has changed
        if(storage.Lang_Locale != Current_Language)
        {
          Current_Language = storage.Lang_Locale;
          InitMenu();
          MainMenu.MenuDisplay(true);
        }

        if ((unsigned long)(millis() - myNex.LastSentCommandMillis) > 1000)
        {
          // Redraw only the submenu currently selected for status change
          MainMenu.Refresh();

          myNex.LastSentCommandMillis = millis();
        }

        // Draw complete menu only once when page appears
        if(myNex.hasPageChanged()) {
          MainMenu.MenuDisplay(true);
        }

      } else {
        period=PT10/2;
      }

      if(myNex.currentPageId == 4)      //Calib
      {
        // Translations for page
        if(myNex.hasPageChanged()) {
          myNex.Deactivate_Sleep(); // Deactivate Sleep until next page change

          myNex.writeStr(PSTR("tTitleCalib.txt"),Helpers::translated_word(FL_(NXT_CALIB_TITLE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tReference.txt"),Helpers::translated_word(FL_(NXT_CALIB_REFERENCE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tMeasured.txt"),Helpers::translated_word(FL_(NXT_CALIB_MEASURED),storage.Lang_Locale));
          myNex.writeStr(PSTR("b0.txt"),Helpers::translated_word(FL_(NXT_OK),storage.Lang_Locale));
          myNex.writeStr(PSTR("b1.txt"),Helpers::translated_word(FL_(NXT_OK),storage.Lang_Locale));
          myNex.writeStr(PSTR("b2.txt"),Helpers::translated_word(FL_(NXT_OK),storage.Lang_Locale));
          myNex.writeStr(PSTR("bApply.txt"),Helpers::translated_word(FL_(NXT_APPLY),storage.Lang_Locale));
          myNex.writeStr(PSTR("bAdd.txt"),Helpers::translated_word(FL_(NXT_ADD),storage.Lang_Locale));
          myNex.writeStr(PSTR("Calib_pH.txt"),Helpers::translated_word(FL_(NXT_CALIB_PHPROBE),storage.Lang_Locale));
          myNex.writeStr(PSTR("Calib_Orp.txt"),Helpers::translated_word(FL_(NXT_CALIB_ORPPROBE),storage.Lang_Locale));
        }
        snprintf_P(temp,sizeof(temp),PSTR("%4.2f"),storage.PhValue);
        myNex.writeStr(F(GLOBAL".vapH.txt"),temp);
        snprintf_P(temp,sizeof(temp),PSTR("%3.0f"),storage.OrpValue);
        myNex.writeStr(F(GLOBAL".vaOrp.txt"),temp);
      } 

      if(myNex.currentPageId == 5)      //Keypad & Keyboard
      {
      }

      if(myNex.currentPageId == 6)      //Wifi Scanner
      {
        // Scan every 10 seconds
        if ((unsigned long)(millis() - LastWifiScan) >= WIFI_SCAN_INTERVAL) {
          ScanWiFiNetworks();
          LastWifiScan=millis();
        }
        // check WiFi Scan Async process
        int16_t WiFiScanStatus = WiFi.scanComplete();
        if (WiFiScanStatus < 0) {  // it is busy scanning or got an error
          if (WiFiScanStatus == WIFI_SCAN_FAILED) {
            Debug.print(DBG_INFO,"[WiFi] Scan has failed. Starting again.");
            ScanWiFiNetworks();
          }
          // other option is status WIFI_SCAN_RUNNING - just wait.
        } else {  // Found Zero or more Wireless Networks
          printScannedNetworks(WiFiScanStatus);
          //ScanWiFiNetworks(); // Start over
        }
      }

      if(myNex.currentPageId == 7)      //Electrolyse
      {
      // Translations for page
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tTitleElectro.txt"),Helpers::translated_word(FL_(NXT_ELECTRO_TITLE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tTemp.txt"),Helpers::translated_word(FL_(NXT_ELECTRO_TEMP),storage.Lang_Locale));
          myNex.writeStr(PSTR("tDelay.txt"),Helpers::translated_word(FL_(NXT_ELECTRO_DELAY),storage.Lang_Locale));
          myNex.writeStr(PSTR("bApply.txt"),Helpers::translated_word(FL_(NXT_APPLY),storage.Lang_Locale));
        }
        myNex.writeNum(F(GLOBAL".vaElectroSec.val"), storage.SecureElectro);
        myNex.writeNum(F(GLOBAL".vaElectroDelay.val"), storage.DelayElectro);
      }

      if(myNex.currentPageId == 8)      //New Tank
      {
        // Translations for page
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tTitleNewTank.txt"),Helpers::translated_word(FL_(NXT_NEWTANK_TITLE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tVol.txt"),Helpers::translated_word(FL_(NXT_NEWTANK_VOL),storage.Lang_Locale));
          myNex.writeStr(PSTR("tFill.txt"),Helpers::translated_word(FL_(NXT_NEWTANK_FILL),storage.Lang_Locale));
          myNex.writeStr(PSTR("bApply.txt"),Helpers::translated_word(FL_(NXT_APPLY),storage.Lang_Locale));
        }
      }

      if(myNex.currentPageId == 9)      //Info
      {
        // Translations for page
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tInfoTitle.txt"),Helpers::translated_word(FL_(NXT_INFO_TITLE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tAlarms.txt"),Helpers::translated_word(FL_(NXT_INFO_ALARMS),storage.Lang_Locale));
          myNex.writeStr(PSTR("tCompiledTitle.txt"),Helpers::translated_word(FL_(NXT_INFO_COMPILE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tCompiledValue.txt"), compile_date); //FIRMW
          myNex.writeStr(F(GLOBAL".vaMCFW.txt"), FIRMW); 
        }
      }

      if(myNex.currentPageId == 10)      //Date Time
      {
        // Translations for page
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tDateTitle.txt"),Helpers::translated_word(FL_(NXT_DATE_TITLE),storage.Lang_Locale));
          myNex.writeStr(PSTR("bApply.txt"),Helpers::translated_word(FL_(NXT_APPLY),storage.Lang_Locale));
          myNex.writeStr(PSTR("tStatus.txt"),Helpers::translated_word(FL_(NXT_MQTT_STATUS),storage.Lang_Locale));
        }
      }

      if(myNex.currentPageId == 11)      //Wifi Config
      {
        // Translations for page
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tWifiTitle.txt"),Helpers::translated_word(FL_(NXT_WIFI_TITLE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tNetwork.txt"),Helpers::translated_word(FL_(NXT_WIFI_NETWORK),storage.Lang_Locale));
          myNex.writeStr(PSTR("tPassword.txt"),Helpers::translated_word(FL_(NXT_WIFI_PASSWORD),storage.Lang_Locale));
          myNex.writeStr(PSTR("tWifi.txt"),Helpers::translated_word(FL_(NXT_WIFI_WIFI),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIP.txt"),Helpers::translated_word(FL_(NXT_WIFI_IP),storage.Lang_Locale));
          myNex.writeStr(PSTR("tStatus.txt"),Helpers::translated_word(FL_(NXT_MQTT_STATUS),storage.Lang_Locale));
          myNex.writeStr(PSTR("bConnect.txt"),Helpers::translated_word(FL_(NXT_CONNECT),storage.Lang_Locale));
        }

        if(WiFi.status() != WL_CONNECTED) {
          myNex.writeStr(F(GLOBAL".vaSSID.txt"),Helpers::translated_word(FL_(NXT_WIFI_NOTCONNECTED),storage.Lang_Locale));
          myNex.writeStr(F(GLOBAL".vaIP.txt"),"");
        } else
        {
          snprintf_P(temp,sizeof(temp),PSTR("%s"),WiFi.SSID().c_str());
          myNex.writeStr(F(GLOBAL".vaSSID.txt"),temp);
          snprintf_P(temp,sizeof(temp),PSTR("%s"),WiFi.localIP().toString().c_str());
          myNex.writeStr(F(GLOBAL".vaIP.txt"),temp);
        } 
      }

      if(myNex.currentPageId == 13)      //Graph card
      {
        if(myNex.hasPageChanged()) {
        }           
      }

      //if(myNex.currentPageId == 14)     //Page Home Minimalist always refresh
      //{
        // Date and Time
        sprintf(temp, PSTR("%02d/%02d/%04d %02d:%02d:%02d"), day(), month(), year(), hour(), minute(), second());
        myNex.writeStr(F("pageHomeSimple.tTimeDate.txt"),temp);

        // PSI difference with Threshold
        if (storage.PSIValue <= storage.PSI_MedThreshold) {
          myNex.writeNum(F("pageHomeSimple.vaPSINiddle.val"), 0);
        } else if (storage.PSIValue > storage.PSI_HighThreshold){
          myNex.writeNum(F("pageHomeSimple.vaPSINiddle.val"), 4);
        } else {
          myNex.writeNum(F("pageHomeSimple.vaPSINiddle.val"), 2);
        }

        // pH & Orp niddle position
        if(abs(storage.PhValue-storage.Ph_SetPoint) <= 0.1) 
          myNex.writeNum(F("pageHomeSimple.vaPHNiddle.val"),0);
        if((storage.PhValue-storage.Ph_SetPoint) > 0.1 && (storage.PhValue-storage.Ph_SetPoint) <= 0.3)  
          myNex.writeNum(F("pageHomeSimple.vaPHNiddle.val"),1);
        if((storage.PhValue-storage.Ph_SetPoint) < -0.1 && (storage.PhValue-storage.Ph_SetPoint) >= -0.3)  
          myNex.writeNum(F("pageHomeSimple.vaPHNiddle.val"),-1);
        if((storage.PhValue-storage.Ph_SetPoint) > 0.3)  
          myNex.writeNum(F("pageHomeSimple.vaPHNiddle.val"),2);
        if((storage.PhValue-storage.Ph_SetPoint) < -0.3)  
          myNex.writeNum(F("pageHomeSimple.vaPHNiddle.val"),-2);

        if(abs(storage.OrpValue-storage.Orp_SetPoint) <= 70.) 
          myNex.writeNum(F("pageHomeSimple.vaOrpNiddle.val"),0);
        if((storage.OrpValue-storage.Orp_SetPoint) > 70. && (storage.OrpValue-storage.Orp_SetPoint) <= 200.)  
          myNex.writeNum(F("pageHomeSimple.vaOrpNiddle.val"),1);
        if((storage.OrpValue-storage.Orp_SetPoint) < -70. && (storage.OrpValue-storage.Orp_SetPoint) >= -200.)  
          myNex.writeNum(F("pageHomeSimple.vaOrpNiddle.val"),-1);
        if((storage.OrpValue-storage.Orp_SetPoint) > 200.)  
          myNex.writeNum(F("pageHomeSimple.vaOrpNiddle.val"),2);    
        if((storage.OrpValue-storage.Orp_SetPoint) < -200.)  
          myNex.writeNum(F("pageHomeSimple.vaOrpNiddle.val"),-2); 

        // pH & Orp Values
        snprintf_P(temp,sizeof(temp),PSTR("%4.2f"),storage.PhValue);
        myNex.writeStr(F(GLOBAL".vapH.txt"),temp);
        snprintf_P(temp,sizeof(temp),PSTR("%3.0f"),storage.OrpValue);
        myNex.writeStr(F(GLOBAL".vaOrp.txt"),temp);
  
        // Water Temperature
        snprintf_P(temp,sizeof(temp),PSTR("%4.1f°C"),storage.WaterTemp);
        myNex.writeStr(F("pageHomeSimple.tTemp.txt"),temp);
      //}

      if(myNex.currentPageId == 15)     //Page Language Selection
      {
        if(myNex.hasPageChanged()) {
          Debug.print(DBG_WARNING,"Language Requested");
          myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_LANG_TITLE),storage.Lang_Locale));
          printLanguages();
        }        
      }

      if(myNex.currentPageId == 16)     //MQTT Configuration
      {
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tMQTTTitle.txt"),Helpers::translated_word(FL_(NXT_MQTT_TITLE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tServer.txt"),Helpers::translated_word(FL_(NXT_MQTT_SERVER),storage.Lang_Locale));
          myNex.writeStr(PSTR("tLogin.txt"),Helpers::translated_word(FL_(NXT_MQTT_LOGIN),storage.Lang_Locale));
          myNex.writeStr(PSTR("tPassword.txt"),Helpers::translated_word(FL_(NXT_MQTT_PASSWORD),storage.Lang_Locale));
          myNex.writeStr(PSTR("tSrvID.txt"),Helpers::translated_word(FL_(NXT_MQTT_ID),storage.Lang_Locale));
          myNex.writeStr(PSTR("tTopic.txt"),Helpers::translated_word(FL_(NXT_MQTT_TOPIC),storage.Lang_Locale));
          myNex.writeStr(PSTR("tMQTTStatus.txt"),Helpers::translated_word(FL_(NXT_MQTT_STATUS),storage.Lang_Locale));
          myNex.writeStr(PSTR("bConnect.txt"),Helpers::translated_word(FL_(NXT_CONNECT),storage.Lang_Locale));

          // Update MQTT parameters
          snprintf_P(temp,sizeof(temp),PSTR("%s"),storage.MQTT_IP.toString().c_str());
          myNex.writeStr(F("vaMQTTSERVER.txt"),temp);
          snprintf_P(temp,sizeof(temp),PSTR("%d"),storage.MQTT_PORT);
          myNex.writeStr(F("vaMQTTPORT.txt"),temp);
          snprintf_P(temp,sizeof(temp),PSTR("%s"),storage.MQTT_LOGIN);
          myNex.writeStr(F("vaMQTTLOGIN.txt"),temp);   
          snprintf_P(temp,sizeof(temp),PSTR("%s"),storage.MQTT_PASS);
          myNex.writeStr(F("vaMQTTPASSWORD.txt"),temp);   
          snprintf_P(temp,sizeof(temp),PSTR("%s"),storage.MQTT_ID);
          myNex.writeStr(F("vaMQTTSRVID.txt"),temp);   
          snprintf_P(temp,sizeof(temp),PSTR("%s"),storage.MQTT_TOPIC);
          myNex.writeStr(F("vaMQTTTopic.txt"),temp);   
        }
      }

      /////////////////////////////////////////////////
      ///////// ACTIONS LINKED TO OVERLAY CONTROL PAGE 
      if(myNex.currentPageId == 17)     //pH Regulation
      {
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_REGULATION_PH),storage.Lang_Locale));
          myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_PUMP_PH),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_0.txt"),"╄");
          myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_SETPPOINTS_PH),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_1.txt"),"╆");
          myNex.writeStr(PSTR("vaDecimal_1.txt"),"2|1|1|");
          myNex.writeStr(PSTR("vaCommand_1.txt"),"PhSetPoint");
          myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_PUMPFLOW_PH),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_2.txt"),"├");
          myNex.writeStr(PSTR("vaDecimal_2.txt"),"1|1|1|l/mn");
          myNex.writeStr(PSTR("vaCommand_2.txt"),"pHPumpFR");
          myNex.writeStr(F("tsw 255,1"));
        }

        if (myNex.readNumber(F("vaUpdAuth.val"))==1) {
          //Update Button when authorized
          myNex.writeNum(F("btAUTO_0.val"),storage.pHAutoMode);
          myNex.writeNum(F("btON_0.val"),(PhPump.IsRunning()&&!storage.pHAutoMode));
          myNex.writeNum(F("btOFF_0.val"),(!PhPump.IsRunning()&&!storage.pHAutoMode));
        }

        //Update Values
        snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.Ph_SetPoint);
        myNex.writeStr(F("vaValueSrc_1.txt"),temp);
        snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.pHPumpFR);
        myNex.writeStr(F("vaValueSrc_2.txt"),temp);
      }

      if(myNex.currentPageId == 18)     //Orp Regulation
      {
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_REGULATION_ORP),storage.Lang_Locale));
          myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_PUMP_CHL),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_0.txt"),"╅");
          myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_SETPPOINTS_ORP),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_1.txt"),"╇");
          myNex.writeStr(PSTR("vaDecimal_1.txt"),"3|0|5|mV");
          myNex.writeStr(PSTR("vaCommand_1.txt"),"OrpSetPoint");
          myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_PUMPFLOW_CHL),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_2.txt"),"├");
          myNex.writeStr(PSTR("vaDecimal_2.txt"),"1|1|1|l/mn");
          myNex.writeStr(PSTR("vaCommand_2.txt"),"ChlPumpFR");
          myNex.writeStr(F("tsw 255,1"));
        }

        if (myNex.readNumber(F("vaUpdAuth.val"))==1) {
          //Update Button when authorized
          myNex.writeNum(F("btAUTO_0.val"),storage.OrpAutoMode);
          myNex.writeNum(F("btON_0.val"),(ChlPump.IsRunning()&&!storage.OrpAutoMode));
          myNex.writeNum(F("btOFF_0.val"),(!ChlPump.IsRunning()&&!storage.OrpAutoMode));
        }

        snprintf_P(temp,sizeof(temp),PSTR("%3.0f"),storage.Orp_SetPoint);
        myNex.writeStr(F("vaValueSrc_1.txt"),temp);
        snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.ChlPumpFR);
        myNex.writeStr(F("vaValueSrc_2.txt"),temp);
      }

      if(myNex.currentPageId == 19)     // Filtration Control
      {
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_SUBMENU1),storage.Lang_Locale));
          myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_MODE_PUMP),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_0.txt"),"╛");
          myNex.writeStr(PSTR("vis tValue_0,1"));
          myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_MODE_PUMP_START),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_1.txt"),"╃");
          myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_MODE_PUMP_END),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_2.txt"),"╹");
          myNex.writeStr(PSTR("vaDecimal_1.txt"),"2|0|1|h");
          myNex.writeStr(PSTR("vaCommand_1.txt"),"FiltT0");
          myNex.writeStr(PSTR("vaDecimal_2.txt"),"2|0|1|h");
          myNex.writeStr(PSTR("vaCommand_2.txt"),"FiltT1");
          myNex.writeStr(F("tsw 255,1"));
        }
        

        snprintf_P(temp,sizeof(temp),PSTR("%02d-%02dh"),storage.FiltrationStart,storage.FiltrationStop);
        myNex.writeStr(F("btAUTO_0.txt"),temp);

        if (myNex.readNumber(F("vaUpdAuth.val"))==1) {
          //Update Button when authorized
          myNex.writeNum(F("btAUTO_0.val"),storage.AutoMode);
          myNex.writeNum(F("btON_0.val"),(FiltrationPump.IsRunning()&&!storage.AutoMode));
          myNex.writeNum(F("btOFF_0.val"),(!FiltrationPump.IsRunning()&&!storage.AutoMode));
        }  

        //Update Values
        snprintf_P(temp,sizeof(temp),PSTR("%d"),storage.FiltrationStartMin);
        myNex.writeStr(F("vaValueSrc_1.txt"),temp);
        snprintf_P(temp,sizeof(temp),PSTR("%d"),storage.FiltrationStopMax);
        myNex.writeStr(F("vaValueSrc_2.txt"),temp);

      }

      if(myNex.currentPageId == 21)     // Heat Control
      {
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_MODE_HEAT_TITLE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_MODE_HEAT),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_0.txt"),"▮");
          myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_MODE_HEAT_TMP_LOW),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_1.txt"),"┬");
          myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_MODE_HEAT_TMP_HIGH),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_2.txt"),"┬");
          myNex.writeStr(PSTR("vaDecimal_1.txt"),"2|1|5|°C");
          myNex.writeStr(PSTR("vaCommand_1.txt"),"WTempLow");
          myNex.writeStr(PSTR("vaDecimal_2.txt"),"2|1|5|°C");
          myNex.writeStr(PSTR("vaCommand_2.txt"),"WSetPoint");
          myNex.writeStr(F("tsw 255,1"));
        }

        snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.WaterTempLowThreshold);
        myNex.writeStr(F("vaValueSrc_1.txt"),temp);
        snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.WaterTemp_SetPoint);
        myNex.writeStr(F("vaValueSrc_2.txt"),temp);

      }

      if(myNex.currentPageId == 22)     // PSI Control
      {
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_MODE_PSI_TITLE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_MODE_PSI_MIN),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_0.txt"),"▫");
          myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_MODE_PSI_MAX),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_1.txt"),"▫");
          myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_MODE_PSI_CURRENT),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_2.txt"),"├");
          myNex.writeStr(PSTR("vaDecimal_0.txt"),"1|1|1|psi");
          myNex.writeStr(PSTR("vaCommand_0.txt"),"PSILow");
          myNex.writeStr(PSTR("vaDecimal_1.txt"),"1|1|1|psi");
          myNex.writeStr(PSTR("vaCommand_1.txt"),"PSIHigh");
          myNex.writeStr(PSTR("vaDecimal_2.txt"),"1|1|1|psi");
          myNex.writeStr(PSTR("vaCommand_2.txt"),"");
          myNex.writeStr(F("tsw 255,1"));
        }

        snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.PSI_MedThreshold);
        myNex.writeStr(F("vaValueSrc_0.txt"),temp);
        snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.PSI_HighThreshold);
        myNex.writeStr(F("vaValueSrc_1.txt"),temp);
        snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.PSIValue);
        myNex.writeStr(F("tValue_2.txt"),temp);
        myNex.writeNum(F("jGauge_2.val"),(int)constrain(((storage.PSIValue/storage.PSI_HighThreshold)*100),0,100));
      }
      if(myNex.currentPageId == 23)     // SWG Control
      {
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_REGULATION_SWG_TITLE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_REGULATION_SWG_CONTROL),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_0.txt"),"▭");
          myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_REGULATION_SWG_MINI),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_1.txt"),"┬");
          myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_REGULATION_SWG_START),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_2.txt"),"╃");
          myNex.writeStr(PSTR("vaDecimal_1.txt"),"2|0|1|°C");
          myNex.writeStr(PSTR("vaCommand_1.txt"),"SecureElectro");
          myNex.writeStr(PSTR("vaDecimal_2.txt"),"2|0|1|mn");
          myNex.writeStr(PSTR("vaCommand_2.txt"),"DelayElectro");
          myNex.writeStr(F("tsw 255,1"));
        }

        if (myNex.readNumber(F("vaUpdAuth.val"))==1) {
          //Update Button when authorized
          myNex.writeNum(F("btAUTO_0.val"),storage.ElectrolyseMode);
          myNex.writeNum(F("btON_0.val"),(SWG.IsRunning()&&!storage.ElectrolyseMode));
          myNex.writeNum(F("btOFF_0.val"),(!SWG.IsRunning()&&!storage.ElectrolyseMode));
        }

        snprintf_P(temp,sizeof(temp),PSTR("%d"),storage.SecureElectro);
        myNex.writeStr(F("vaValueSrc_1.txt"),temp);
        snprintf_P(temp,sizeof(temp),PSTR("%d"),storage.DelayElectro);
        myNex.writeStr(F("vaValueSrc_2.txt"),temp);

      }
      if(myNex.currentPageId == 24)     // Tank Status
      {
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_PHORPTANKS_TITLE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_PHORPTANKS_PH),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_0.txt"),"╄");
          myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_PHORPTANKS_CHL),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_1.txt"),"╅");
          myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_PHORPTANKS_FILL),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_2.txt"),"▓");
          myNex.writeStr(PSTR("vaDecimal_0.txt"),"2|0|1|mn");
          myNex.writeStr(PSTR("vaCommand_0.txt"),"");
          myNex.writeStr(PSTR("vaDecimal_1.txt"),"2|0|1|mn");
          myNex.writeStr(PSTR("btOFF_2.txt"),Helpers::translated_word(FL_(NXT_PHORPTANKS_FILL_PH),storage.Lang_Locale));
          myNex.writeStr(PSTR("btON_2.txt"),Helpers::translated_word(FL_(NXT_PHORPTANKS_FILL_CHL),storage.Lang_Locale));

          myNex.writeStr(F("tsw 255,1"));
        }

        snprintf_P(temp,sizeof(temp),PSTR("%4.1f"),float(PhPump.UpTime)/1000./60.);
        myNex.writeStr(F("tValue_0.txt"),temp);
        snprintf_P(temp,sizeof(temp),PSTR("%4.1f"),float(ChlPump.UpTime)/1000./60.);
        myNex.writeStr(F("tValue_1.txt"),temp);

        myNex.writeNum(F("jGauge_0.val"), constrain((int)(PhPump.GetTankFill()),0,100));
        myNex.writeNum(F("jGauge_1.val"), constrain((int)(ChlPump.GetTankFill()),0,100));
      }

      if(myNex.currentPageId == 25)     // Relays
      {
        if(myNex.hasPageChanged()) {
          myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_RELAYS_TITLE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_RELAYS_ROBOT),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_0.txt"),"▱");
          myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_RELAYS_LIGHTS),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_1.txt"),"▰");
          myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_RELAYS_SPARE),storage.Lang_Locale));
          myNex.writeStr(PSTR("tIcon_2.txt"),"▲");
          myNex.writeStr(PSTR("vaDecimal_0.txt"),"");
          myNex.writeStr(PSTR("vaCommand_0.txt"),"");
          myNex.writeStr(PSTR("vaDecimal_1.txt"),"");
          myNex.writeStr(PSTR("vaCommand_1.txt"),"");
          myNex.writeStr(PSTR("vaDecimal_2.txt"),"");
          myNex.writeStr(PSTR("vaCommand_2.txt"),"");
          myNex.writeStr(F("tsw 255,1"));
        }

        if (myNex.readNumber(F("vaUpdAuth.val"))==1) {
          //Update Button when authorized
          myNex.writeNum(F("btON_0.val"),RobotPump.IsRunning());
          myNex.writeNum(F("btOFF_0.val"),!RobotPump.IsRunning());

          myNex.writeNum(F("btON_1.val"),RELAYR0.IsActive());
          myNex.writeNum(F("btOFF_1.val"),!RELAYR0.IsActive());

          myNex.writeNum(F("btON_2.val"),RELAYR1.IsActive());
          myNex.writeNum(F("btOFF_2.val"),!RELAYR1.IsActive());
        }
      }
      // vaControl Main Configuration
      // Menu Type
      // -1: Hide Control
      //  0: SLIDER with value indication
      //     A: <unused>
      //     B: Minimum slider value
      //     C: Maximum slider value
      //  1: ON/OFF without value indication
      //     X: Custom Command index as in "easyNexReadCustomCommand" (values returned are OFF=0, ON=1, AUTO=2)
      //  2: ON/AUTO:OFF without value indication
      //     X: Custom Command index as in "easyNexReadCustomCommand" (values returnedare OFF=0, ON=1, AUTO=2)
      //  3: GAUGE with value indication
      //     A: <unused>
      if(myNex.currentPageId == 20)     // Empty Control Overlay page (to be initialized)
      {
        if(myNex.hasPageChanged()) {
          uint32_t page_index = myNex.readNumber(F("pageOVControls.vaOverlayIndex.val"));
          switch(page_index) {
            case 17: // pH Regulation
              myNex.writeStr(PSTR("vaControl_0.txt"),"2|11");
              myNex.writeStr(PSTR("vaControl_1.txt"),"0|0|62|82");
              myNex.writeStr(PSTR("vaControl_2.txt"),"0|0|10|50");
              myNex.writeStr(F("click btInitialize,1"));
              break;
            case 18: // Orp Regulation
              myNex.writeStr(PSTR("vaControl_0.txt"),"2|12");
              myNex.writeStr(PSTR("vaControl_1.txt"),"0|0|120|160");
              myNex.writeStr(PSTR("vaControl_2.txt"),"0|0|10|50");
              myNex.writeStr(F("click btInitialize,1"));
            break;
          case 19: // Pump Regulation
              myNex.writeStr(PSTR("vaControl_0.txt"),"2|13");
              myNex.writeStr(PSTR("vaControl_1.txt"),"0|0|0|24");
              myNex.writeStr(PSTR("vaControl_2.txt"),"0|0|0|24");
              myNex.writeStr(F("click btInitialize,1"));
            break;
          case 21: // Heat Regulation
            myNex.writeStr(PSTR("vaControl_0.txt"),"2|14");
            myNex.writeStr(PSTR("vaControl_1.txt"),"0|0|1|64");
            myNex.writeStr(PSTR("vaControl_2.txt"),"0|0|1|64");
            myNex.writeStr(F("click btInitialize,1"));
          break;
          case 22: // PSI Regulation
            myNex.writeStr(PSTR("vaControl_0.txt"),"0|0|01|30");
            myNex.writeStr(PSTR("vaControl_1.txt"),"0|0|01|30");
            myNex.writeStr(PSTR("vaControl_2.txt"),"3|0");  // PSI Gauge
            myNex.writeStr(F("click btInitialize,1"));
          break;
          case 23: // SWG Regulation
            myNex.writeStr(PSTR("vaControl_0.txt"),"2|15");
            myNex.writeStr(PSTR("vaControl_1.txt"),"0|0|10|20");  //temp Start
            myNex.writeStr(PSTR("vaControl_2.txt"),"0|0|01|30");  //mn delay
            myNex.writeStr(F("click btInitialize,1"));
          break;
          case 24: // Tank Status
            myNex.writeStr(PSTR("vaControl_0.txt"),"3|0");  // Gauge pH
            myNex.writeStr(PSTR("vaControl_1.txt"),"3|0");  //Gauge Orp
            myNex.writeStr(PSTR("vaControl_2.txt"),"1|16");  //Send Command
            myNex.writeStr(F("click btInitialize,1"));
          break;
          case 25: // Relays
            myNex.writeStr(PSTR("vaControl_0.txt"),"1|17");  // Robot
            myNex.writeStr(PSTR("vaControl_1.txt"),"1|18");  // Lights
            myNex.writeStr(PSTR("vaControl_2.txt"),"1|19");  // Spare
            myNex.writeStr(F("click btInitialize,1"));
          break;
          }
        }
      }

      ///////// ACTIONS LINKED TO HELP
      if(myNex.currentPageId == 30)      //Help Popup
      {
        if(myNex.hasPageChanged()) {    // Trigger once when page changes
          uint32_t help_index = myNex.readNumber(F("pageHelpPopup.vaHelpIndex.val"));

          switch(help_index) {
            case 1: // Calibration Help
              myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_HELP_1_TITLE),storage.Lang_Locale));
              myNex.writeStr(PSTR("tContent.txt"),Helpers::translated_word(FL_(NXT_HELP_1_CONTENT),storage.Lang_Locale));
              break;
          }
        }
      }
    } else {
      period = PT10;
    }
    #ifdef CHRONO
    t_act = millis() - td;
    if(t_act > t_max) t_max = t_act;
    if(t_act < t_min) t_min = t_act;
    t_mean += (t_act - t_mean)/n;
    ++n;
    Debug.print(DBG_INFO,"[PoolMaster] td: %d t_act: %d t_min: %d t_max: %d t_mean: %4.1f",td,t_act,t_min,t_max,t_mean);
    #endif 

    stack_mon(hwm);
    xWasDelayed = xTaskDelayUntil(&ticktime,period);

    // Check if the task was delayed normally. Error means that the 
    // next scheduled time is in the past and, thus, that the task
    // code took too long to execute.
    if (xWasDelayed == pdFALSE)
      Debug.print(DBG_ERROR,"Error delaying Nextion task. Possibly took too long to execute.");
  } 
}
/*******************************************************************
 * ******************** END OF TASK LOOP ***************************
 *******************************************************************/


/**************** CUSTOME COMMANDS *********************
 ***************************************************************/
// Custom Command used for quick actions triggered via
// printh 23 02 XX YY ZZ (XX=CmdGroup) then
//    for Switch change, YY=SwitchIndex ZZ=Value
//    for Graph request, YY=graphIndex
// Read Custom Commands used to force value of configuration parameter
void easyNexReadCustomCommand()
{
  int value;
  LastAction = millis();
  switch(myNex.cmdGroup)
  {
    case 'C': // Or <case 0x43:>  If 'C' matches Direct Command 
    {
      // from Nextion printh 23 01 43
      char Cmd[100] = "";
      strcpy(Cmd,myNex.readStr(F(GLOBAL".vaCommand.txt")).c_str());
      xQueueSendToBack(queueIn,&Cmd,0);
      Debug.print(DBG_INFO,"Nextion direct command: %s",Cmd);
      break;
    }

    case 'S': // Or <case 0x53:>  If 'S' matches Update Switches 
    {
      // from Nextion printh 23 03 53 00 00
      //                               |  |
      //                               |  | ---> Value
      //                               |----> Command Index
      // read the next byte that determines the position on the table
      int SwitchIndex;
      SwitchIndex = myNex.readByte();
      // read the next byte that keeps the value for the position
      value = myNex.readByte();

      Debug.print(DBG_VERBOSE,"Comma Sent 53 %d %d",SwitchIndex,value);
      switch(SwitchIndex)
      {
        case 0x00:  // Filtration Mode
          SetValue("Mode",value);
          break;
        case 0x01:  // pH Auto Mode
          SetValue("PhAutoMode",value);
          break;
        case 0x02:  // Orp Auto Mode
          SetValue("OrpAutoMode",value);
          break;
        case 0x03:  // Future Use for Heat Auto
          break;
        case 0x04:  // Filtration Pump -- DO NOT CHANGE IDs WITHOUTH CHANGING Nextion FTF
          SetValue("FiltPump",value);
          break;
        case 0x05:  // pH Pump
          SetValue("PhPump",value);
          break;
        case 0x06:  // Chl Pump
          SetValue("ChlPump",value);
          break;
        case 0x07:  // Heating
          break;
        case 0x08:  // Winter Mode
          SetValue("Winter",value);
        break;
        case 0x0A:  // Change Language
          SetValue("Lang_Locale",value);
        break;
        case 0x11:  // pH Pump
          if(value==BUTTON_AUTO) {
            SetValue("PhAutoMode",1);
          } else {
            SetValue("PhPump",value);
          }
        break;
        case 0x12:  // Chl Pump
          if(value==BUTTON_AUTO) {
            SetValue("OrpAutoMode",1);
          } else {
            SetValue("ChlPump",value);
          }
        break;
        case 0x13:  // Pump Menu Change
          if(value==BUTTON_AUTO) {
            SetValue("Mode",1);
          } else {
            SetValue("FiltPump",value);
          }
        break;
        case 0x14:  // Heat Regulation
          if(value==1) {
            // Not implemented
          } else {
            // Not implemented
          }
        break;
        case 0x15:  // SWG Regulation
          if(value==BUTTON_AUTO) {
            SetValue("ElectrolyseMode",1);
          } else {
            SetValue("Electrolyse",value);
          }
        break;
        case 0x16:  // Tank Status
          if(value==0) {  //pH
            myNex.writeStr(F("pageFillTank.vaTankBut.txt"),"pH");
            myNex.writeStr(F("page pageFillTank"));
          } else if(value==2) { //Chl
            myNex.writeStr(F("pageFillTank.vaTankBut.txt"),"Chl");
            myNex.writeStr(F("page pageFillTank"));
          }
        break;
        case 0x17:  // Robot
          SetValue("RobotPump",value);
        break;
        case 0x18:  // Lights
          SetValue("Relay",value,0);
        break;
        case 0x19:  // Spare
          SetValue("Relay",value,1);
        break;
      }
      break;
    }

    case 'G': // Or <case 0x47:> If 'G' send graph data
    {
      // from Nextion printh 23 02 47 XX (graph reference)
      // Read graph reference
      int graphIndex;
      graphIndex = myNex.readByte();
      char buf[NUMBER_OF_HISTORY_SAMPLES];
      switch(graphIndex)
      {
        case 0x00:  // pH
          Debug.print(DBG_INFO,"pH Graph Requested");
          //Change Title
          myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_PH_GRAPH_TITLE),storage.Lang_Locale));
          // Change scale values
          snprintf_P(temp,sizeof(temp),PSTR("%4.2f"),((float(GRAPH_PH_BASELINE)+200)/100));
          myNex.writeStr(F("tMax.txt"),temp);
          snprintf_P(temp,sizeof(temp),PSTR("%4.2f"),float(GRAPH_PH_BASELINE)/100);
          myNex.writeStr(F("tMin.txt"),temp);
          snprintf_P(temp,sizeof(temp),PSTR("%4.2f"),(float(GRAPH_PH_BASELINE)+100)/100);
          myNex.writeStr(F("tMed.txt"),temp);

          // Initialize table
          for(int i=0;i<NUMBER_OF_HISTORY_SAMPLES;i++)
          {
          if(i<pH_Samples.size())
            // Get the pH Sample with baseline reference
            buf[i] = (char)(pH_Samples[i]-GRAPH_PH_BASELINE);
          else
            buf[i] = 0;
          }
          snprintf_P(temp_command,sizeof(temp_command),PSTR("addt %d,%d,%d"),2,0,pH_Samples.size());
          myNex.writeStr(temp_command);
          vTaskDelay(5 / portTICK_PERIOD_MS);
          Serial2.write(buf,pH_Samples.size());
        break;
        case 0x01:  // Orp
          Debug.print(DBG_INFO,"Orp Graph Requested");
          //Change Title
          myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_ORP_GRAPH_TITLE),storage.Lang_Locale));
          // Change scale values
          snprintf_P(temp,sizeof(temp),PSTR("%d"),(GRAPH_ORP_BASELINE+200));
          myNex.writeStr(F("tMax.txt"),temp);
          snprintf_P(temp,sizeof(temp),PSTR("%d"),GRAPH_ORP_BASELINE);
          myNex.writeStr(F("tMin.txt"),temp);
          snprintf_P(temp,sizeof(temp),PSTR("%d"),(GRAPH_ORP_BASELINE+100));
          myNex.writeStr(F("tMed.txt"),temp);

          // Initialize table
          for(int i=0;i<NUMBER_OF_HISTORY_SAMPLES;i++)
          {
          if(i<Orp_Samples.size())
            // Get the Orp Sample with baseline reference
            buf[i] = (char)(Orp_Samples[i]-GRAPH_ORP_BASELINE);
          else
            buf[i] = 0;
          }
          snprintf_P(temp_command,sizeof(temp_command),PSTR("addt %d,%d,%d"),2,0,Orp_Samples.size());
          myNex.writeStr(temp_command);
          vTaskDelay(5 / portTICK_PERIOD_MS);
          Serial2.write(buf,Orp_Samples.size());
        break;
        case 0x02:  // pH
          Debug.print(DBG_INFO,"Temp Graph Requested");
          //Change Title
          myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_TEMP_GRAPH_TITLE),storage.Lang_Locale));
          // Change scale values
          snprintf_P(temp,sizeof(temp),PSTR("%4.1f"),((float(GRAPH_TEMP_BASELINE)+200)/10));
          myNex.writeStr(F("tMax.txt"),temp);
          snprintf_P(temp,sizeof(temp),PSTR("%4.1f"),float(GRAPH_TEMP_BASELINE)/10);
          myNex.writeStr(F("tMin.txt"),temp);
          snprintf_P(temp,sizeof(temp),PSTR("%4.1f"),(float(GRAPH_TEMP_BASELINE)+100)/10);
          myNex.writeStr(F("tMed.txt"),temp);

          // Initialize table
          for(int i=0;i<NUMBER_OF_HISTORY_SAMPLES;i++)
          {
          if(i<pH_Samples.size())
            // Get the Water Temperature Sample with baseline reference
            buf[i] = (char)(WTemp_Samples[i]-GRAPH_TEMP_BASELINE);
          else
            buf[i] = 0;
          }
          snprintf_P(temp_command,sizeof(temp_command),PSTR("addt %d,%d,%d"),2,0,WTemp_Samples.size());
          myNex.writeStr(temp_command);
          vTaskDelay(5 / portTICK_PERIOD_MS);
          Serial2.write(buf,WTemp_Samples.size());
        break;
      }
    } // And of Case
  }  // End of Switch
} // End of function

/********************** MENU TRIGGERS  *********************
 ***************************************************************/
// Main Menu Item triggered via 
// printh 23 02 4D 00 xx (_menu_item)
void triggermainmenu(uint8_t _menu_item)
{
  //MainMenu.MenuDisplay(false);  // Redraw main menu selection (useless as nothing new will be drawn, select/unselect handled by Nextion)
  MainMenu.Select(_menu_item);
}

// Sub Menu Item triggered via
// printh 23 02 4D 01 xx (_menu_item)
void triggersubmenu(uint8_t _menu_item)
{
  MainMenu.GetItemRef()->submenu->Select(_menu_item);
}

/********************* WIFI Scanning **********************
 **********************************************************/
// Functions used to scan and print WIfi Networks
void ScanWiFiNetworks(){
  // WiFi.scanNetworks will return the number of networks found.
  if(WiFi.status() != WL_CONNECTED)
    DisconnectFromWiFi(true); // Disconnect from Wifi and do not try to reconnect
  Debug.print(DBG_INFO,"Scanning Networks. Start");
  int n = WiFi.scanNetworks(true);  //Async mode
}

// Send  result of Wifi Scanning to Nextion screen
void printScannedNetworks(uint16_t networksFound) {
  if (networksFound == 0) {
    Debug.print(DBG_INFO,"[WiFi] No network found");
    snprintf_P(temp_command,sizeof(temp_command),PSTR("pageWifiPopup.tNetwork%d.txt"),1);
    snprintf_P(temp,sizeof(temp),PSTR("%s"),Helpers::translated_word(FL_(NXT_WIFI_NONETWORKFOUND),storage.Lang_Locale));
    myNex.writeStr(temp_command,temp);
  } else {
    // Print only first MAX_SHOWN_NETWORKS networks (depending on how Nextion page is designed)
    networksFound = (networksFound > MAX_SHOWN_NETWORKS? MAX_SHOWN_NETWORKS : networksFound);

    for (int i = 0; i < networksFound; ++i) {
      // Print SSID each network found
      snprintf_P(temp_command,sizeof(temp_command),PSTR("pageWifiPopup.tNetwork%d.txt"),i+1);
      snprintf_P(temp,sizeof(temp),PSTR("%s"),WiFi.SSID(i).c_str());
      myNex.writeStr(temp_command,temp);
      // Print RSSI Icons
      snprintf_P(temp_command,sizeof(temp_command),PSTR("pageWifiPopup.tNetSignal%d.txt"),i+1);
      if(WiFi.RSSI(i)>-70)
        snprintf_P(temp,sizeof(temp),PSTR("%s"),"┹"); // Highest signal
        else if(WiFi.RSSI(i)>-80)
          snprintf_P(temp,sizeof(temp),PSTR("%s"),"┶");
          else if(WiFi.RSSI(i)>-90)
            snprintf_P(temp,sizeof(temp),PSTR("%s"),"┳");
            else
              snprintf_P(temp,sizeof(temp),PSTR("%s"),"┰"); // Lowest Signal
      myNex.writeStr(temp_command,temp);
    }
    if(networksFound > 0){
      myNex.writeNum(F("pageWifiPopup.vaScanEnded.val"), 1);
    }
    // Delete the scan result to free memory for code.
    WiFi.scanDelete();
  }
  reconnectToWiFi();
}

/***************** LANGUAGE Scanning **********************
 **********************************************************/
void printLanguages()
{
  for (int i = 0; i < NUM_LANGUAGES; ++i) {
    // Print SSID each network found
    snprintf_P(temp_command,sizeof(temp_command),PSTR("Lang_%d.txt"),i);
    snprintf_P(temp,sizeof(temp),PSTR("%s"),languages[i]);
    myNex.writeStr(temp_command,temp);
  }
}

/************************ MENU DESIGN **************************
 ***************************************************************/
void InitMenu()
{
  // ENM_ACTION CONFIG (on pageMenu)
  // 1: page pageOVControls + control page index (as described in page "pageOVControls.vaOverlayIndex.val")
  // 121: Calib
  // 141: Lannguage
  // 143: SysInfo
  // 144: Date/Time
  // 145: Wifi Settings
  // 146: MQTT Settings

  //Delete all previously defined menu
  MainMenu.Reinitialize();
  SubMenu1.Reinitialize();
  SubMenu2.Reinitialize();
  SubMenu3.Reinitialize();
  SubMenu4.Reinitialize();

  // Main Menu
  MainMenu.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_MENU_LEFT1),storage.Lang_Locale),nullptr,nullptr,&SubMenu1);
  MainMenu.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_MENU_LEFT2),storage.Lang_Locale),nullptr,nullptr,&SubMenu2);
  MainMenu.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_MENU_LEFT3),storage.Lang_Locale),nullptr,nullptr,&SubMenu3);
  MainMenu.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_MENU_LEFT4),storage.Lang_Locale),nullptr,nullptr,&SubMenu4);
  MainMenu.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_MENU_LEFT5),storage.Lang_Locale),nullptr,nullptr,ENM_NONE);
  
  // Sub Menus
  SubMenu1.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU1),storage.Lang_Locale),"┖",nullptr,ENM_ACTION,1,19);   // Filtration Options
  SubMenu1.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU2),storage.Lang_Locale),"▫",nullptr,ENM_ACTION,1,22);   // PSI  Options
  SubMenu1.AddItem([]() {ToggleValue("Winter",storage.WinterMode);},nullptr,Helpers::translated_word(FL_(NXT_SUBMENU3),storage.Lang_Locale),"┡","┢",ENM_BISTABLE, []() {return (storage.WinterMode==1);});
  
  SubMenu2.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU8),storage.Lang_Locale),"▦",nullptr,ENM_ACTION,121);   // Calibrate Probes
  SubMenu2.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU9),storage.Lang_Locale),"╆",nullptr,ENM_ACTION,1,17);  // pH Regulation
  SubMenu2.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU10),storage.Lang_Locale),"╇",nullptr,ENM_ACTION,1,18); // Orp Regulation
  SubMenu2.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU11),storage.Lang_Locale),"▓",nullptr,ENM_ACTION,1,24); // Tank Status
  SubMenu2.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU12),storage.Lang_Locale),"▭",nullptr,ENM_ACTION,1,23);// SWG Options
  SubMenu2.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU13),storage.Lang_Locale),"▮",nullptr,ENM_ACTION,1,21); // Heat Options

  SubMenu3.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU17),storage.Lang_Locale),"╂",nullptr,ENM_ACTION,1,25); // Control Relays

  SubMenu4.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU22),storage.Lang_Locale),"╴",nullptr,ENM_ACTION,141); // Language
  SubMenu4.AddItem([]() {ToggleValue("Clear",false);},nullptr,Helpers::translated_word(FL_(NXT_SUBMENU23),storage.Lang_Locale),"△","△",ENM_BISTABLE, []() {return (false);}); // Ackowledge Alerts
  SubMenu4.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU26),storage.Lang_Locale),"┮",nullptr,ENM_ACTION,145); // Wifi Settings
  SubMenu4.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU27),storage.Lang_Locale),"▪",nullptr,ENM_ACTION,146); // MQTT Settings
  SubMenu4.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU25),storage.Lang_Locale),"╃",nullptr,ENM_ACTION,144); // Set Date/Time
  SubMenu4.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU24),storage.Lang_Locale),"▴",nullptr,ENM_ACTION,143); // System Info
}

/********************* HELPER Functions **********************
 *************************************************************/
double map(double x, double in_min, double in_max, int out_min, int out_max) {
  if ((in_max - in_min)==0)
    return 0;
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void syncESP2RTC(uint32_t _second, uint32_t _minute, uint32_t _hour, uint32_t _day, uint32_t _month, uint32_t _year) {
  Debug.print(DBG_INFO,"[TIME] ESP/NTP -> RTC");

  myNex.writeNum(F("rtc5"),_second);
  myNex.writeNum(F("rtc4"),_minute);
  myNex.writeNum(F("rtc3"),_hour);
  myNex.writeNum(F("rtc2"),_day);
  myNex.writeNum(F("rtc1"),_month);
  myNex.writeNum(F("rtc0"),_year);
}

void syncRTC2ESP() {
  Debug.print(DBG_INFO,"[TIME] RTC Time -> ESP");

  setTime(
    (int)myNex.readNumber(F("rtc3")),
    (int)myNex.readNumber(F("rtc4")),
    (int)myNex.readNumber(F("rtc5")),
    (int)myNex.readNumber(F("rtc2")),
    (int)myNex.readNumber(F("rtc1")),
    (int)myNex.readNumber(F("rtc0"))
  );
}

// Function to send standard commands to PoolServer
// either toggle or force set a value
void ToggleValue(const char* _server_command, int _current_state)
{
  char Cmd[100];
  _current_state = (_current_state)? false:true;
  sprintf(Cmd,"{\"%s\":%d}",_server_command,_current_state); // Build JSON Command
  Debug.print(DBG_INFO,"[NEXTION] Sending command to server %s = %d",_server_command,_current_state);
  xQueueSendToBack(queueIn,&Cmd,0);
  LastAction = millis();
}

void SetValue(const char* _server_command, int _force_state, int _state_table)
{
  char Cmd[100];
  if(_state_table == -1) {
    sprintf(Cmd,"{\"%s\":%d}",_server_command,_force_state); // Build JSON Command
    Debug.print(DBG_INFO,"[NEXTION] Sending command to server %s = %d",_server_command,_force_state);
  } else {
    sprintf(Cmd,"{\"%s\":[%d,%d]}",_server_command,_state_table,_force_state); // Build JSON Command
    Debug.print(DBG_INFO,"[NEXTION] Sending command to server %s = [%d,%d]",_server_command,_state_table,_force_state);
  }
  xQueueSendToBack(queueIn,&Cmd,0);
  LastAction = millis();
}

