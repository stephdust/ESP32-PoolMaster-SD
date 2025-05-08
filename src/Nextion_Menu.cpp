/*
  NEXTION MENU TFT related code, based on EasyNextion library by Seithan / Athanasios Seitanis.
  Used to display the menu and the status of the pool.
  The trigger(s) functions at the end are called by the Nextion library on event (buttons, page change).
    Completely reworked and simplified. These functions only update Nextion variables. The display is then updated localy by the
    Nextion itself.
*/
#include "Nextion_Menu.h"

EasyNextionMenus  MainMenu(15,ENM_MAIN);
EasyNextionMenus  SubMenu1(7,ENM_SUB);
EasyNextionMenus  SubMenu2(7,ENM_SUB);
EasyNextionMenus  SubMenu3(7,ENM_SUB);
EasyNextionMenus  SubMenu4(7,ENM_SUB);
EasyNextionMenus  SubMenu5(7,ENM_SUB);
EasyNextionMenus  SubMenu6(7,ENM_SUB);
EasyNextionMenus  SubMenu7(7,ENM_SUB);
EasyNextionMenus  SubMenu8(7,ENM_SUB);
EasyNextionMenus  SubMenu9(7,ENM_SUB);
EasyNextionMenus  SubMenu10(7,ENM_SUB);
EasyNextionMenus  SubMenu11(7,ENM_SUB);
EasyNextionMenus  SubMenu12(7,ENM_SUB);

/**
 * @brief Initialize the Nextion menu system.
 * @param _myNex The EasyNex object to use for the menu.
 */
void NexMenu_Init(EasyNex& _myNex)
{
    // Set the Nextion object to use for the menu
    MainMenu.SetNextionScreen(&_myNex);
    SubMenu1.SetNextionScreen(&_myNex);
    SubMenu2.SetNextionScreen(&_myNex);
    SubMenu3.SetNextionScreen(&_myNex);
    SubMenu4.SetNextionScreen(&_myNex);
    SubMenu5.SetNextionScreen(&_myNex);
    SubMenu6.SetNextionScreen(&_myNex);
    SubMenu7.SetNextionScreen(&_myNex);
    SubMenu8.SetNextionScreen(&_myNex);
    SubMenu9.SetNextionScreen(&_myNex);
    SubMenu10.SetNextionScreen(&_myNex);   
    SubMenu11.SetNextionScreen(&_myNex);  
    SubMenu12.SetNextionScreen(&_myNex);  

    // ENM_ACTION CONFIG (on pageMenu)
    // 1: page pageOVControls + control page index (as described in object "pageOVControls.vaOverlayIndex.val")
    // 121: Calib
    // 141: Language
    // 143: SysInfo
    // 144: Date/Time
    // 145: Wifi Settings
    // 146: MQTT Settings
    // 147: Alerts Settings

    //Delete all previously defined menu
    MainMenu.Reinitialize();
    SubMenu1.Reinitialize();
    SubMenu2.Reinitialize();
    SubMenu3.Reinitialize();
    SubMenu4.Reinitialize();
    SubMenu5.Reinitialize();

    // Main Menu
    MainMenu.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_MENU_LEFT1),storage.Lang_Locale),nullptr,nullptr,&SubMenu1);
    MainMenu.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_MENU_LEFT2),storage.Lang_Locale),nullptr,nullptr,&SubMenu2);
    MainMenu.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_MENU_LEFT3),storage.Lang_Locale),nullptr,nullptr,&SubMenu3);
    MainMenu.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_MENU_LEFT4),storage.Lang_Locale),nullptr,nullptr,&SubMenu4);
    MainMenu.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_MENU_LEFT5),storage.Lang_Locale),nullptr,nullptr,ENM_NONE);

    // Sub Menus
    SubMenu1.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU1),storage.Lang_Locale),"┖",nullptr,ENM_ACTION,1,ENMP_FILTRATION);       // Filtration Options
    SubMenu1.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU2),storage.Lang_Locale),"▫",nullptr,ENM_ACTION,1,ENMP_PSI_OPTIONS);      // PSI  Options
    SubMenu1.AddItem([]() {SetValue("Winter",SETVALUE_TOGGLE,SETVALUE_DIRECT,storage.WinterMode);},nullptr,Helpers::translated_word(FL_(NXT_SUBMENU3),storage.Lang_Locale),"┡","┢",ENM_BISTABLE, []() {return (storage.WinterMode==1);});

    SubMenu2.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU8),storage.Lang_Locale),"▦",nullptr,ENM_ACTION,121);                     // Calibrate Probes
    SubMenu2.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU9),storage.Lang_Locale),"╆",nullptr,ENM_ACTION,1,ENMP_PH_REGULATION);    // pH Regulation
    SubMenu2.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU10),storage.Lang_Locale),"╇",nullptr,ENM_ACTION,1,ENMP_ORP_REGULATION);  // Orp Regulation
    SubMenu2.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU11),storage.Lang_Locale),"▓",nullptr,ENM_ACTION,1,ENMP_TANK_STATUS);     // Tank Status
    SubMenu2.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU13),storage.Lang_Locale),"▮",nullptr,ENM_ACTION,1,ENMP_HEAT_REGULATION);// Heat Options
    SubMenu2.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU14),storage.Lang_Locale),"╒",nullptr,ENM_ACTION,1,ENMP_WATER_LEVEL);     // Water Level Options

    SubMenu3.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU30),storage.Lang_Locale),"▭",nullptr,ENM_ACTION,1,ENMP_SWG_MODES);      // SWG Mode
    SubMenu3.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU31),storage.Lang_Locale),"╃",nullptr,ENM_ACTION,1,ENMP_SWG_REGULATION);  // SWG Regulation

    SubMenu4.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU17),storage.Lang_Locale),"╂",nullptr,ENM_ACTION,1,ENMP_RELAYS_CONTROL);  // Control Relays
    SubMenu4.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU18),storage.Lang_Locale),"╲",nullptr,ENM_ACTION,149);                    // Assign PINs

    SubMenu5.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU22),storage.Lang_Locale),"╴",nullptr,ENM_ACTION,141);                    // Language
    SubMenu5.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU23),storage.Lang_Locale),"▘",nullptr,ENM_ACTION,147);                   // Alerts Settings
    SubMenu5.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU26),storage.Lang_Locale),"┮",nullptr,ENM_ACTION,145);                    // Wifi Settings
    SubMenu5.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU27),storage.Lang_Locale),"▪",nullptr,ENM_ACTION,146);                    // MQTT Settings
    #ifdef SMTP
    SubMenu5.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU28),storage.Lang_Locale),"◁",nullptr,ENM_ACTION,148);                   // SMTP Settings
    #endif
    SubMenu5.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU25),storage.Lang_Locale),"╃",nullptr,ENM_ACTION,144);                    // Set Date/Time
    SubMenu5.AddItem(nullptr,nullptr,Helpers::translated_word(FL_(NXT_SUBMENU24),storage.Lang_Locale),"▴",nullptr,ENM_ACTION,143);                    // System Info
}

/** 
 * @brief  This function is called in the main Nextion loop to handle menu related actions.
 *         It handles the actions linked to the overlay control page.
 * @param  _myNex: Reference to the EasyNex object.
 * @retval None
 */
void NexMenu_Loop(EasyNex& _myNex)
{
    switch(_myNex.currentPageId)
    {
        case ENMP_PH_REGULATION:     // pH Regulation
            if(_myNex.hasPageChanged()) {}

            if (_myNex.readNumber(F("vaUpdAuth.val"))==1) {
                //Update Button when authorized
                _myNex.writeNum(F("btAUTO_0.val"),storage.pHAutoMode);
                _myNex.writeNum(F("btON_0.val"),(PhPump.IsRunning()&&!storage.pHAutoMode));
                _myNex.writeNum(F("btOFF_0.val"),(!PhPump.IsRunning()&&!storage.pHAutoMode));
            }

            //Update Values
            snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.Ph_SetPoint);
            _myNex.writeStr(F("vaValueSrc_1.txt"),temp);
            snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.PumpsConfig[PUMP_PH].pump_flow_rate);
            _myNex.writeStr(F("vaValueSrc_2.txt"),temp);
            
        break;

        case ENMP_ORP_REGULATION:     // ORP Regulation
            if(_myNex.hasPageChanged()) {}

            if (_myNex.readNumber(F("vaUpdAuth.val"))==1) {
                //Update Button when authorized
                _myNex.writeNum(F("btAUTO_0.val"),storage.OrpAutoMode);
                _myNex.writeNum(F("btON_0.val"),(ChlPump.IsRunning()&&!storage.OrpAutoMode));
                _myNex.writeNum(F("btOFF_0.val"),(!ChlPump.IsRunning()&&!storage.OrpAutoMode));
            }

            snprintf_P(temp,sizeof(temp),PSTR("%3.0f"),storage.Orp_SetPoint);
            _myNex.writeStr(F("vaValueSrc_1.txt"),temp);
            snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.PumpsConfig[PUMP_CHL].pump_flow_rate);
            _myNex.writeStr(F("vaValueSrc_2.txt"),temp);
            
        break;

        case ENMP_FILTRATION:     // Filtration Regulation
            if(_myNex.hasPageChanged()) {}

            snprintf_P(temp,sizeof(temp),PSTR("%02d-%02dh"),storage.FiltrationStart,storage.FiltrationStop);
            _myNex.writeStr(F("btAUTO_0.txt"),temp);

            if (_myNex.readNumber(F("vaUpdAuth.val"))==1) {
                //Update Button when authorized
                _myNex.writeNum(F("btAUTO_0.val"),storage.AutoMode);
                _myNex.writeNum(F("btON_0.val"),(FiltrationPump.IsRunning()&&!storage.AutoMode));
                _myNex.writeNum(F("btOFF_0.val"),(!FiltrationPump.IsRunning()&&!storage.AutoMode));
            }  

            //Update Values
            snprintf_P(temp,sizeof(temp),PSTR("%d"),storage.FiltrationStartMin);
            _myNex.writeStr(F("vaValueSrc_1.txt"),temp);
            snprintf_P(temp,sizeof(temp),PSTR("%d"),storage.FiltrationStopMax);
            _myNex.writeStr(F("vaValueSrc_2.txt"),temp);

        break;

        case ENMP_HEAT_REGULATION:     // HEAT Regulation
            if(_myNex.hasPageChanged()) {}

            snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.WaterTempLowThreshold);
            _myNex.writeStr(F("vaValueSrc_1.txt"),temp);
            snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.WaterTemp_SetPoint);
            _myNex.writeStr(F("vaValueSrc_2.txt"),temp);

        break;

        case ENMP_PSI_OPTIONS:     // PSI Regulation
            if(_myNex.hasPageChanged()) {}

            snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.PSI_MedThreshold);
            _myNex.writeStr(F("vaValueSrc_0.txt"),temp);
            snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.PSI_HighThreshold);
            _myNex.writeStr(F("vaValueSrc_1.txt"),temp);
            snprintf_P(temp,sizeof(temp),PSTR("%3.1f"),storage.PSIValue);
            _myNex.writeStr(F("tValue_2.txt"),temp);
            _myNex.writeNum(F("jGauge_2.val"),(int)constrain(((storage.PSIValue/storage.PSI_HighThreshold)*100),0,100));

        break;

        case ENMP_SWG_MODES:     // SWG Modes
            if(_myNex.hasPageChanged()) {}

            if (_myNex.readNumber(F("vaUpdAuth.val"))==1) {
                //Update Button when authorized
                _myNex.writeNum(F("btAUTO_0.val"),storage.ElectrolyseMode);
                _myNex.writeNum(F("btON_0.val"),(SWGPump.IsRunning()&&!storage.ElectrolyseMode));
                _myNex.writeNum(F("btOFF_0.val"),(!SWGPump.IsRunning()&&!storage.ElectrolyseMode));
            }

            snprintf_P(temp,sizeof(temp),PSTR("%d"),storage.SecureElectro);
            _myNex.writeStr(F("vaValueSrc_1.txt"),temp);
            snprintf_P(temp,sizeof(temp),PSTR("%d"),storage.DelayElectro);
            _myNex.writeStr(F("vaValueSrc_2.txt"),temp);

        break;

        case ENMP_TANK_STATUS:     // TANK Status
            if(_myNex.hasPageChanged()) {}

            snprintf_P(temp,sizeof(temp),PSTR("%4.1f"),float(PhPump.UpTime)/1000./60.);
            _myNex.writeStr(F("tValue_0.txt"),temp);
            snprintf_P(temp,sizeof(temp),PSTR("%4.1f"),float(ChlPump.UpTime)/1000./60.);
            _myNex.writeStr(F("tValue_1.txt"),temp);

            _myNex.writeNum(F("jGauge_0.val"), constrain((int)(PhPump.GetTankFill()),0,100));
            _myNex.writeNum(F("jGauge_1.val"), constrain((int)(ChlPump.GetTankFill()),0,100));

        break;

        case ENMP_RELAYS_CONTROL:     // RELAYS Control
            if(_myNex.hasPageChanged()) {}

            if (_myNex.readNumber(F("vaUpdAuth.val"))==1) {
                //Update Button when authorized
                _myNex.writeNum(F("btON_0.val"),RobotPump.IsRunning());
                _myNex.writeNum(F("btOFF_0.val"),!RobotPump.IsRunning());

                _myNex.writeNum(F("btON_1.val"),RELAYR0.IsActive());
                _myNex.writeNum(F("btOFF_1.val"),!RELAYR0.IsActive());

                _myNex.writeNum(F("btON_2.val"),RELAYR1.IsActive());
                _myNex.writeNum(F("btOFF_2.val"),!RELAYR1.IsActive());
            }
        break;

        case ENMP_WATER_LEVEL:     // WATER LEVEL Control
            if(_myNex.hasPageChanged()) {}

            //Update Values
            snprintf_P(temp,sizeof(temp),PSTR("%d"),storage.PumpsConfig[PUMP_FILL].pump_min_uptime / 60);
            _myNex.writeStr(F("vaValueSrc_0.txt"),temp);
            snprintf_P(temp,sizeof(temp),PSTR("%d"),storage.PumpsConfig[PUMP_FILL].pump_max_uptime / 60);
            _myNex.writeStr(F("vaValueSrc_1.txt"),temp);
            
        break;

        case ENMP_SWG_REGULATION:     // SWG Regulation
            if(_myNex.hasPageChanged()) {}

            if (_myNex.readNumber(F("vaUpdAuth.val"))==1) {
                //Update Button when authorized
                _myNex.writeNum(F("btON_0.val"),storage.ElectroRunMode);
                _myNex.writeNum(F("btOFF_0.val"),!storage.ElectroRunMode);
            }        
            //Update Values
            snprintf_P(temp,sizeof(temp),PSTR("%d"),storage.ElectroRuntime);
            _myNex.writeStr(F("vaValueSrc_1.txt"),temp);
            
        break;

        // vaControl & vaDecimal Main Configuration
        //  This is where to declare the content of configuration page overlay (controls to be shown)
        //*****************************/
        //// "vaControl" meaning: A|B|C|D
        // A=:
        //  ENM_CTRL_NONE=-1: Hide Control (default value)
        //  ENM_CTRL_SLIDER=0: SLIDER with value indication
        //     B: <unused>
        //     C: Minimum slider value
        //     D: Maximum slider value
        //  ENM_CTRL_ONOFF=1: ON/OFF without value indication
        //     B: Custom Command index as in "easyNexReadCustomCommand" (values returned are OFF=0, ON=1, AUTO=2)
        //  ENM_CTRL_ONAUTOOFF=2: ON/AUTO/OFF without value indication
        //     B: Custom Command index as in "easyNexReadCustomCommand" (values returnedare OFF=0, ON=1, AUTO=2)
        //  ENM_CTRL_GAUGE=3: GAUGE with value indication
        //     B: <unused>
        //*****************************/
        /// "vaDecimal" Meaning:
        // - A: Before comma # of digits max
        // - B: After comma # of digits
        // - C: Value Multiplier
        // - D: unit name
        case ENMP_EMPTY_CONTROLS:     //INITIALIZE CONTROLS
            if(_myNex.hasPageChanged()) {
                char TmpControl[10];
                uint32_t page_index = _myNex.readNumber(F(MENU_ACTION_NEXT_PAGE_OBJECT".val"));
                switch(page_index) {
                    case ENMP_PH_REGULATION: // pH Regulation
                        _myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_REGULATION_PH),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_PUMP_PH),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_0.txt"),"╄");
                        _myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_SETPPOINTS_PH),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_1.txt"),"╆");
                        _myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_PUMPFLOW_PH),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_2.txt"),"├");

                        sprintf(TmpControl,"%s|%d",ENM_CTRL_ONAUTOOFF,ENMC_PH_PUMP_MENU);
                        _myNex.writeStr(PSTR("vaControl_0.txt"),TmpControl);
                        _myNex.writeStr(PSTR("vaControl_1.txt"),ENM_CTRL_SLIDER"|0|62|82");
                        _myNex.writeStr(PSTR("vaControl_2.txt"),ENM_CTRL_SLIDER"|0|10|50");

                        _myNex.writeStr(PSTR("vaDecimal_1.txt"),"2|1|1|");
                        _myNex.writeStr(PSTR("vaCommand_1.txt"),"PhSetPoint");
                        _myNex.writeStr(PSTR("vaDecimal_2.txt"),"1|1|1|l/mn");
                        _myNex.writeStr(PSTR("vaCommand_2.txt"),"pHPumpFR");
                    break;
                    case ENMP_ORP_REGULATION: // Orp Regulation
                        _myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_REGULATION_ORP),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_PUMP_CHL),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_0.txt"),"╅");
                        _myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_SETPPOINTS_ORP),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_1.txt"),"╇");
                        _myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_PUMPFLOW_CHL),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_2.txt"),"├");

                        sprintf(TmpControl,"%s|%d",ENM_CTRL_ONAUTOOFF,ENMC_CHL_PUMP_MENU);
                        _myNex.writeStr(PSTR("vaControl_0.txt"),TmpControl);
                        _myNex.writeStr(PSTR("vaControl_1.txt"),ENM_CTRL_SLIDER"|0|120|160");
                        _myNex.writeStr(PSTR("vaControl_2.txt"),ENM_CTRL_SLIDER"|0|10|50");

                        _myNex.writeStr(PSTR("vaDecimal_1.txt"),"3|0|5|mV");
                        _myNex.writeStr(PSTR("vaCommand_1.txt"),"OrpSetPoint");
                        _myNex.writeStr(PSTR("vaDecimal_2.txt"),"1|1|1|l/mn");
                        _myNex.writeStr(PSTR("vaCommand_2.txt"),"ChlPumpFR");
                    break;
                    case ENMP_FILTRATION: // Filtration Regulation
                        _myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_SUBMENU1),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_MODE_PUMP),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_0.txt"),"╛");
                        _myNex.writeStr(PSTR("vis tValue_0,1"));
                        _myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_MODE_PUMP_START),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_1.txt"),"╃");
                        _myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_MODE_PUMP_END),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_2.txt"),"╹");
                        
                        sprintf(TmpControl,"%s|%d",ENM_CTRL_ONAUTOOFF,ENMC_FILT_PUMP_MENU);
                        _myNex.writeStr(PSTR("vaControl_0.txt"),TmpControl);
                        _myNex.writeStr(PSTR("vaControl_1.txt"),ENM_CTRL_SLIDER"|0|0|24");
                        _myNex.writeStr(PSTR("vaControl_2.txt"),ENM_CTRL_SLIDER"|0|0|24");

                        _myNex.writeStr(PSTR("vaDecimal_1.txt"),"2|0|1|h");
                        _myNex.writeStr(PSTR("vaCommand_1.txt"),"FiltT0");
                        _myNex.writeStr(PSTR("vaDecimal_2.txt"),"2|0|1|h");
                        _myNex.writeStr(PSTR("vaCommand_2.txt"),"FiltT1");
                    break;
                    case ENMP_HEAT_REGULATION: // Heat Regulation
                        _myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_MODE_HEAT_TITLE),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_MODE_HEAT),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_0.txt"),"▮");
                        _myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_MODE_HEAT_TMP_LOW),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_1.txt"),"┬");
                        _myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_MODE_HEAT_TMP_HIGH),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_2.txt"),"┬");

                        sprintf(TmpControl,"%s|%d",ENM_CTRL_NONE,ENMC_HEAT_MENU);
                        _myNex.writeStr(PSTR("vaControl_0.txt"),TmpControl);
                        _myNex.writeStr(PSTR("vaControl_1.txt"),ENM_CTRL_SLIDER"|0|1|64");
                        _myNex.writeStr(PSTR("vaControl_2.txt"),ENM_CTRL_SLIDER"|0|1|64");

                        _myNex.writeStr(PSTR("vaDecimal_1.txt"),"2|1|5|°C");
                        _myNex.writeStr(PSTR("vaCommand_1.txt"),"WTempLow");
                        _myNex.writeStr(PSTR("vaDecimal_2.txt"),"2|1|5|°C");
                        _myNex.writeStr(PSTR("vaCommand_2.txt"),"WSetPoint");
                    break;
                    case ENMP_PSI_OPTIONS: // PSI Regulation
                        _myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_MODE_PSI_TITLE),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_MODE_PSI_MIN),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_0.txt"),"▫");
                        _myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_MODE_PSI_MAX),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_1.txt"),"▫");
                        _myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_MODE_PSI_CURRENT),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_2.txt"),"├");

                        _myNex.writeStr(PSTR("vaControl_0.txt"),ENM_CTRL_SLIDER"|0|0|30");
                        _myNex.writeStr(PSTR("vaControl_1.txt"),ENM_CTRL_SLIDER"|0|0|30");
                        _myNex.writeStr(PSTR("vaControl_2.txt"),ENM_CTRL_GAUGE"|0");  // PSI Gauge

                        _myNex.writeStr(PSTR("vaDecimal_0.txt"),"1|1|1|psi");
                        _myNex.writeStr(PSTR("vaCommand_0.txt"),"PSILow");
                        _myNex.writeStr(PSTR("vaDecimal_1.txt"),"1|1|1|psi");
                        _myNex.writeStr(PSTR("vaCommand_1.txt"),"PSIHigh");
                        _myNex.writeStr(PSTR("vaDecimal_2.txt"),"1|1|1|psi");
                        _myNex.writeStr(PSTR("vaCommand_2.txt"),"");
                    break;
                    case ENMP_SWG_MODES: // SWG Mode
                        _myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_REGULATION_SWG_TITLE),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_REGULATION_SWG_CONTROL),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_0.txt"),"▭");
                        _myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_REGULATION_SWG_MINI),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_1.txt"),"┬");
                        _myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_REGULATION_SWG_START),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_2.txt"),"╃");

                        sprintf(TmpControl,"%s|%d",ENM_CTRL_ONAUTOOFF,ENMC_SWG_MODE_MENU);
                        _myNex.writeStr(PSTR("vaControl_0.txt"),TmpControl);
                        _myNex.writeStr(PSTR("vaControl_1.txt"),ENM_CTRL_SLIDER"|0|10|20");  //temp Start
                        _myNex.writeStr(PSTR("vaControl_2.txt"),ENM_CTRL_SLIDER"|0|01|30");  //mn delay

                        _myNex.writeStr(PSTR("vaDecimal_1.txt"),"2|0|1|°C");
                        _myNex.writeStr(PSTR("vaCommand_1.txt"),"SecureElectro");
                        _myNex.writeStr(PSTR("vaDecimal_2.txt"),"2|0|1|mn");
                        _myNex.writeStr(PSTR("vaCommand_2.txt"),"DelayElectro");
                    break;
                    case ENMP_TANK_STATUS: // Tank Status
                        _myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_PHORPTANKS_TITLE),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_PHORPTANKS_PH),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_0.txt"),"╄");
                        _myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_PHORPTANKS_CHL),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_1.txt"),"╅");
                        _myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_PHORPTANKS_FILL),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_2.txt"),"▓");
                        _myNex.writeStr(PSTR("btOFF_2.txt"),Helpers::translated_word(FL_(NXT_PHORPTANKS_FILL_PH),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("btON_2.txt"),Helpers::translated_word(FL_(NXT_PHORPTANKS_FILL_CHL),storage.Lang_Locale));

                        _myNex.writeStr(PSTR("vaControl_0.txt"),ENM_CTRL_GAUGE"|0");  // Gauge pH
                        _myNex.writeStr(PSTR("vaControl_1.txt"),ENM_CTRL_GAUGE"|0");  //Gauge Orp
                        sprintf(TmpControl,"%s|%d",ENM_CTRL_ONOFF,ENMC_TANK_STATUS);
                        _myNex.writeStr(PSTR("vaControl_2.txt"),TmpControl);  //Send Command

                        _myNex.writeStr(PSTR("vaDecimal_0.txt"),"2|0|1|mn");
                        _myNex.writeStr(PSTR("vaCommand_0.txt"),"");
                        _myNex.writeStr(PSTR("vaDecimal_1.txt"),"2|0|1|mn");
                    break;
                    case ENMP_RELAYS_CONTROL: // Relays
                        _myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_RELAYS_TITLE),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_RELAYS_ROBOT),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_0.txt"),"▱");
                        _myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_RELAYS_LIGHTS),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_1.txt"),"▰");
                        _myNex.writeStr(PSTR("tItem_2.txt"),Helpers::translated_word(FL_(NXT_RELAYS_SPARE),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_2.txt"),"▲");

                        sprintf(TmpControl,"%s|%d",ENM_CTRL_ONOFF,ENMC_ROBOT);
                        _myNex.writeStr(PSTR("vaControl_0.txt"),TmpControl);  // Robot
                        sprintf(TmpControl,"%s|%d",ENM_CTRL_ONOFF,ENMC_LIGHTS);
                        _myNex.writeStr(PSTR("vaControl_1.txt"),TmpControl);  // Lights
                        sprintf(TmpControl,"%s|%d",ENM_CTRL_ONOFF,ENMC_SPARE);
                        _myNex.writeStr(PSTR("vaControl_2.txt"),TmpControl);  // Spare
                    break;
                    case ENMP_WATER_LEVEL: // Water Level Regulation
                        _myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_WATERLEVEL_TITLE),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_WATERLEVEL_MIN_UPTIME),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_0.txt"),"╃");
                        _myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_WATERLEVEL_MAX_UPTIME),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_1.txt"),"╹");

                        _myNex.writeStr(PSTR("vaControl_0.txt"),ENM_CTRL_SLIDER"|0|1|60");  // Filling Pump Min Uptime
                        _myNex.writeStr(PSTR("vaControl_1.txt"),ENM_CTRL_SLIDER"|0|1|60");  // Filling Pump Max Uptime

                        _myNex.writeStr(PSTR("vaDecimal_0.txt"),"2|0|1|mn");
                        _myNex.writeStr(PSTR("vaCommand_0.txt"),"FillMinUpTime");
                        _myNex.writeStr(PSTR("vaDecimal_1.txt"),"2|0|1|mn");
                        _myNex.writeStr(PSTR("vaCommand_1.txt"),"FillMaxUpTime");
                    break;
                    case ENMP_SWG_REGULATION: // SWG Regulation
                        _myNex.writeStr(PSTR("tTitle.txt"),Helpers::translated_word(FL_(NXT_ELECTROMODE_TITLE),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tItem_0.txt"),Helpers::translated_word(FL_(NXT_ELECTROMODE_MODE),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_0.txt"),"╇");
                        _myNex.writeStr(PSTR("tItem_1.txt"),Helpers::translated_word(FL_(NXT_ELECTROMODE_UPTIME),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("tIcon_1.txt"),"╃");
                        _myNex.writeStr(PSTR("btOFF_0.txt"),Helpers::translated_word(FL_(NXT_ELECTROMODE_OFF),storage.Lang_Locale));
                        _myNex.writeStr(PSTR("btON_0.txt"),Helpers::translated_word(FL_(NXT_ELECTROMODE_ON),storage.Lang_Locale));

                        sprintf(TmpControl,"%s|%d",ENM_CTRL_ONOFF,ENMC_SWG_REGULATION);
                        _myNex.writeStr(PSTR("vaControl_0.txt"),TmpControl); // SWG Regulation Mode
                        _myNex.writeStr(PSTR("vaControl_1.txt"),ENM_CTRL_SLIDER"|0|1|12");  //SWG Duration Duration (for time mode)

                        _myNex.writeStr(PSTR("vaDecimal_1.txt"),"2|0|1|h");
                        _myNex.writeStr(PSTR("vaCommand_1.txt"),"ElectroRuntime");
                    break;
                }
                _myNex.writeStr(F("click btInitialize,1"));
                _myNex.writeStr(F("tsw 255,1"));
            }
        break;
    }   // End of switch(_myNex.currentPageId)
}   // End of NexMenu_Loop function


/** 
 * @brief  Function to trigger the main menu item
 * @param  _menu_item: menu item to select (0-4). Nextion command is printh 23 02 4D 00 xx (_menu_item)
 * @retval None
 */
void triggermainmenu(uint8_t _menu_item)
{
  MainMenu.Select(_menu_item);
}

/** 
 * @brief  Function to trigger the submenu item
 * @param  _menu_item: submenu item to select. Nextion command is printh 23 02 4D 01 xx (_menu_item)
 * @retval None
 */
void triggersubmenu(uint8_t _menu_item)
{
  MainMenu.GetItemRef()->submenu->Select(_menu_item);
}

/**
 * @brief  Function to send standard commands to PoolServer either toggle or force set a value
 * @param  _server_command: command to send to server (e.g. "Winter" for WinterMode)
 * @param  _force_state: -1 to toggle, 0 or 1 to force set the value
 * @param  _state_table: -1 to send direct command (e.g. "_server_command=_force_state"), or value to change in the table (e.g. "_server_command=[_state_table,_force_state]")
 * @param  _current_state: current state of the command (used when toggling value, _force_state=-1)
 * @retval None
 */
void SetValue(const char* _server_command, int _force_state, int _state_table, int _current_state)
{
  // Do we need to toggle
  if (_force_state == -1) {
    _force_state = (_current_state)? false:true;
  }
  
  char Cmd[100];
  if(_state_table == -1) {
    sprintf(Cmd,"{\"%s\":%d}",_server_command,_force_state); // Build JSON Command
    Debug.print(DBG_INFO,"[NEXTION] Sending command to server %s = %d",_server_command,_force_state);
  } else {
    sprintf(Cmd,"{\"%s\":[%d,%d]}",_server_command,_state_table,_force_state); // Build JSON Command
    Debug.print(DBG_INFO,"[NEXTION] Sending command to server %s = [%d,%d]",_server_command,_state_table,_force_state);
  }
  xQueueSendToBack(queueIn,&Cmd,0);
}
