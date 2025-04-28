
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
static int myWMPulsePerLiter = 1;               // 1 pulse each liter
static long int myWaterMeterCounter = 0;

static char *WaterMeterPulsePoolTopicMeas=0;
void AddonsPublishTopic(char*, JsonDocument&);
int AddonsReadRetainedTopic(char*, JsonDocument&);

void WaterMeterPulseSaveMeasures(void *pvParameters)
{
    if (!WaterMeterPulse) return;

    //send a JSON to MQTT broker. /!\ Split JSON if longer than 100 bytes
    const int capacity = JSON_OBJECT_SIZE(1); // only 1 value to publish
    StaticJsonDocument<capacity> root;
    root["Counter"]     = myWaterMeterCounter;
    root["Pulse-Liter"] = myWMPulsePerLiter;
    AddonsPublishTopic(WaterMeterPulsePoolTopicMeas, root);
}

void WaterMeterPulseLoadMeasures(void *pvParameters)
{
    if (!WaterMeterPulse) return;

    //send a JSON to MQTT broker. /!\ Split JSON if longer than 100 bytes
     const int capacity = JSON_OBJECT_SIZE(2); // only 1 value to publish
    StaticJsonDocument<capacity> root;
    if (!AddonsReadRetainedTopic(WaterMeterPulsePoolTopicMeas, root)) 
        return;

    myWaterMeterCounter = root["Counter"];
    myWMPulsePerLiter   = root["Pulse-Liter"];
}

void WaterMeterPulseTask(void *pvParameters)
{
    if (!WaterMeterPulse) return;
    int16_t pulseCount=0, InitPulseCount=0;

    WaterMeterPulseLoadMeasures(pvParameters);  // read mqtt values, might be externally updated

    // loop till counter does not change anymore
    pcnt_get_counter_value(PCNT_UNIT_0, &pulseCount);
    if (!pulseCount) return;

    while (pulseCount != InitPulseCount) {
        InitPulseCount = pulseCount;
        delay(2000);
        pcnt_get_counter_value(PCNT_UNIT_0, &pulseCount);
    }
    
    // save Counter to MQTT and ready for next loop
    myWaterMeterCounter += pulseCount * myWMPulsePerLiter;
    WaterMeterPulseSaveMeasures(pvParameters);
    
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

    // init MQTT Topic name
    if (!WaterMeterPulsePoolTopicMeas) {
        WaterMeterPulsePoolTopicMeas = (char*)malloc(strlen(POOLTOPIC)+strlen(WaterMeterPulseName)+5);
        sprintf(WaterMeterPulsePoolTopicMeas, "%s/Meas%s", POOLTOPIC, WaterMeterPulseName);
    }
  
    // Init xTask
    WaterMeterPulse                 = true;
    myWaterMeterPulse.name          = WaterMeterPulseName;
    myWaterMeterPulse.Task          = WaterMeterPulseTask;
    myWaterMeterPulse.frequency     = 800;     // check value every 800 ms
    myWaterMeterPulse.LoadSettings  = 0;
    myWaterMeterPulse.SaveSettings  = 0;
    myWaterMeterPulse.LoadMeasures  = WaterMeterPulseLoadMeasures;
    myWaterMeterPulse.SaveMeasures  = WaterMeterPulseSaveMeasures;
    myWaterMeterPulse.HistoryStats  = 0;

    if (WaterMeterPulse) 
        xTaskCreatePinnedToCore(
            AddonsLoop,
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

