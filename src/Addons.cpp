#include <Arduino.h>                // Arduino framework
#include "Config.h"
#include "PoolMaster.h"

// Addons Headers
#define MaxAddons 4
#include "BCM68X.h"
//#include "RF433T.h"
//#include "Dehumidifier.h"
//#include "PoolCover.h"

void stack_mon(UBaseType_t&);
static int _NbAddons = 0;

typedef void (*function) (void);
typedef void (*function2) (void *pvParameters);

struct AddonFunctionsStruct {
    char  *name;
    function init;
    function2 action;
    function SettingsJSON;
    function MeasuresJSON;
};
static AddonFunctionsStruct AddonFunctions[MaxAddons];

void InitAddon(char* name, function init, function2 action, function SettingsJSON, function MeasuresJSON)
{
    if (_NbAddons == MaxAddons) return;
    AddonFunctions[_NbAddons].name = name;
    AddonFunctions[_NbAddons].init = init;
    AddonFunctions[_NbAddons].action = action;
    AddonFunctions[_NbAddons].SettingsJSON = SettingsJSON;
    AddonFunctions[_NbAddons].MeasuresJSON = MeasuresJSON;
    _NbAddons++;
    init();
}

//Init All Addons
void AddonsInit()
{
#ifdef BME68X
    InitAddon(BME68XName, BCM68XInit, BCM68XAction, BCM68XSettingsJSON, BCM68XMeasureJSON);
#endif
#ifdef RF433T
    InitAddon(RF4433TName, RF433TInit, RF433TAction, RF433TSettingsJSON, RT433TMeasureJSON);
#endif
    for (int i=_NbAddons-1; i<MaxAddons; i++) AddonFunctions[i] = {0, 0, 0, 0, 0};
}

int NbAddons()
{
    return _NbAddons;
}

void AddonsAction(void *pvParameters)
{
  while (!startTasks) ;
  vTaskDelay(DT12);           // Scheduling offset 

  TickType_t period = PT12;  
  TickType_t ticktime = xTaskGetTickCount();
  static UBaseType_t hwm = 0;

  #ifdef CHRONO
  unsigned long td;
  int t_act=0,t_min=999,t_max=0;
  float t_mean=0.;
  int n=1;
  #endif

  vTaskDelayUntil(&ticktime,period);
  
  for(;;)
  {        
    #ifdef CHRONO
    td = millis();
    #endif 

    for (int i=0; i<_NbAddons; i++) {
        AddonFunctions[i].action(pvParameters);
        delay(50);
    }
    #ifdef CHRONO
    t_act = millis() - td;
    if(t_act > t_max) t_max = t_act;
    if(t_act < t_min) t_min = t_act;
    t_mean += (t_act - t_mean)/n;
    ++n;
    Debug.print(DBG_INFO,"[AddonsAction] td: %d t_act: %d t_min: %d t_max: %d t_mean: %4.1f",td,t_act,t_min,t_max,t_mean);
    #endif

    stack_mon(hwm);
    vTaskDelayUntil(&ticktime,period);
  }
}

void AddonsPublishSettings(void)
{
    for (int i=0; i<_NbAddons; i++) {
        AddonFunctions[i].SettingsJSON();
        delay(50);
    }
}


void AddonsPublishMeasures(void)
{
    for (int i=0; i<_NbAddons; i++) {
        AddonFunctions[i].MeasuresJSON();
        delay(50);
    }
}

