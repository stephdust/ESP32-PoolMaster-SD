#include <Arduino.h>                // Arduino framework
#include "Config.h"
#include "PoolMaster.h"

// Addons Headers
#define MaxAddons 3
#include "BCM68X.h"
//#include "RF433T.h"

void stack_mon(UBaseType_t&);
static int _NbAddons = 0;

struct AddonsFunctionsStruct {
    char  name[7];  // MAX 6 chars for the name
    void *init(void);
    void *action(void*);
    void *SettingsJSON(void*);
    void *MeasuresJSON(void*);
};
static AddonsFunctionsStruct AddonsFunctions[MaxAddons];

void AddAddons(char* name, void* init, void* action, void *SettingsJSON, void *MeasuresJSON)
{
    if (_NbAddons == MaxAddons) return;
    AddonsFunctions[_NbAddons].name = name;
    AddonsFunctions[_NbAddons].init = init;
    AddonsFunctions[_NbAddons].action = action;
    AddonsFunctions[_NbAddons].SettingsJSON = SettingsJSON;
    AddonsFunctions[_NbAddons].MeasuresJSON = MeasuresJSON;
    _NbAddons++;
}

//Init All Addons
void AddonsInit()
{
#ifdef BME68X
    AddAddons("BME68x", BCM68XInit, BCM68XAction, BCM68XSettingsJSON, BCM68XMeasureJSON);
#endif
#ifdef RF433T
    AddAddons("RF433T", RF433TInit, RF433TAction, RF433TSettingsJSON, RT433TMeasureJSON);
#endif
    for (int i=_NbAddons-1; i<MaxAddons; i++) AddonsFunctions[i] = {"", 0, 0, 0, 0};
}

int NbAddons()
{
    return NbAddons;
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

    for (int i=0; i<_NbAddons; i++) *AddonsFunctions[i].action(pvParameters);

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
