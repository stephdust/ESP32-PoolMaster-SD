
// Credits :


#include <Arduino.h>
#include "Config.h"
#include "PoolMaster.h"

#if defined(_ADDONS_) && defined(_IO_ADDON_WATERMETER_PULSE_)

#include "Addon_WaterMeter_Pulse.h"
#include <driver/pcnt.h>

const char *WaterMeterPulseName = "WaterMeter";
static bool WaterMeterPulse = false;    // no WaterMeterPulse transmitter by default
AddonStruct myWaterMeterPulse;
static int16_t InitPulseCount = 0;
#define PulseToLiter    1               // 1 pulse each liter
static long int myWaterMeterCounter = 0;

void WaterMeterPulseTask(void *pvParameters)
{
    if (!WaterMeterPulse) return;
    int16_t pulseCount = 0;
    InitPulseCount = 0;

    // loop till counter does not change anymore
    pcnt_get_counter_value(PCNT_UNIT_0, &pulseCount);
    if (!pulseCount) return;

    while (pulseCount != InitPulseCount) {
        InitPulseCount = pulseCount;
        delay(2000);
        pcnt_get_counter_value(PCNT_UNIT_0, &pulseCount);
    }
    
    // save Counter to MQTT and ready for next loop
    myWaterMeterCounter += pulseCount * PulseToLiter;
    

    //SaveConfig

    
    pcnt_counter_pause(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);
    pcnt_counter_resume(PCNT_UNIT_0);
}

AddonStruct WaterMeterPulseInit(void)
{
    // PCNT counter

    // verif pullup ?
    // verif sens input 
    pcnt_config_t pcnt_config = {};
    pcnt_config.pulse_gpio_num = _IO_ADDON_WATERMETER_PULSE_;
    pcnt_config.ctrl_gpio_num = PCNT_PIN_NOT_USED;
    pcnt_config.channel = PCNT_CHANNEL_0;
    pcnt_config.unit = PCNT_UNIT_0;
    pcnt_config.pos_mode = PCNT_COUNT_INC;
    pcnt_config.neg_mode = PCNT_COUNT_DIS;
    pcnt_config.lctrl_mode = PCNT_MODE_KEEP;
    pcnt_config.hctrl_mode = PCNT_MODE_KEEP;
    pcnt_unit_config(&pcnt_config);

    // verif filter
    pcnt_set_filter_value(PCNT_UNIT_0, 1);
    pcnt_filter_enable(PCNT_UNIT_0);
    pcnt_counter_pause(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);
    pcnt_counter_resume(PCNT_UNIT_0);

    // read Counter value from MQTT if exists
    //myWaterMeterCounter = 

    // Init xTask
    WaterMeterPulse                 = true;
    myWaterMeterPulse.name          = WaterMeterPulseName;
    myWaterMeterPulse.Task          = WaterMeterPulseTask;
    myWaterMeterPulse.frequency     = 800;     // check value every 800 ms
    myWaterMeterPulse.SettingsJSON  = 0;
    myWaterMeterPulse.MeasuresJSON  = 0;
    myWaterMeterPulse.HistoryStats  = 0;
    myWaterMeterPulse.LoadConfig    = 0;
    myWaterMeterPulse.SaveConfig    = 0;

    if (WaterMeterPulse) 
        xTaskCreatePinnedToCore(
            AddonLoop,
            myWaterMeterPulse.name,
            3072,
            &myWaterMeterPulse,
            1,
            nullptr,
            xPortGetCoreID()
        );
    return myWaterMeterPulse;
}

#endif

