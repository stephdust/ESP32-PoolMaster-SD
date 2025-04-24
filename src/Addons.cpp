#include <Arduino.h>                // Arduino framework
#include "Config.h"
#include "PoolMaster.h"

#ifdef _ADDONS_

#define MaxAddons 5
#include "TR_BME68X.h"
//#include "PR_BME68X.h"
#include "TFA_RF433T.h"
//#include "WaterMeter_Pulse.h"
//#include "Dehumidifier.h"
//#include "PoolCover.h"

void stack_mon(UBaseType_t&);

static int _NbAddons = 0;
static AddonStruct myAddons[MaxAddons];


//Init All Addons
void AddonsInit()
{
#ifdef _TR_BME68X_
    myAddons[_NbAddons++] = TR_BME68XInit();
#endif
#ifdef _PR_BME68X_
    myAddons[_NbAddons++] = InitAddon(PR_BME68XInit);
#endif
#ifdef _TFA_RF433T_
    myAddons[_NbAddons++] = TFA_RF433TInit();
#endif
#ifdef _WATERMETER_PULSE_
#endif

}

/*
int NbAddons()
{
    return _NbAddons;
}
*/

void AddonLoop(void *pvParameters)
{
  AddonStruct *myAddon  = (AddonStruct*)pvParameters;
  TickType_t period     = myAddons->frequency;

  while (!startTasks) ;
  vTaskDelay(500/portTICK_PERIOD_MS);           // Scheduling offset 

  TickType_t ticktime = xTaskGetTickCount();
  UBaseType_t hwm = 0;
 
  #ifdef CHRONO
  unsigned long td;
  int t_act=0,t_min=999,t_max=0;
  float t_mean=0.;
  int n=1;
  #endif

  Debug.print(DBG_INFO,"[Start AddonLoop %s] freq: %d",myAddon->name,myAddon->frequency);
  for(;;)
  {       
    vTaskDelayUntil(&ticktime,period);

    #ifdef CHRONO
    td = millis();
    #endif 

    if (myAddon->Task) myAddon->Task(pvParameters);
    delay(50);
    
    #ifdef CHRONO
    t_act = millis() - td;
    if(t_act > t_max) t_max = t_act;
    if(t_act < t_min) t_min = t_act;
    t_mean += (t_act - t_mean)/n;
    ++n;
    Debug.print(DBG_INFO,"[Addons Action %s] td: %d t_act: %d t_min: %d t_max: %d t_mean: %4.1f",myAddon->name,td,t_act,t_min,t_max,t_mean);
    #endif

    stack_mon(hwm);
  }
}


void AddonsPublishSettings(void *pvParameters)
{
    for (int i=0; i<_NbAddons; i++) {
        if (myAddons[i].SettingsJSON) {
            myAddons[i].SettingsJSON(pvParameters);
            delay(50);
        }
    }
}


void AddonsPublishMeasures(void *pvParameters)
{
    for (int i=0; i<_NbAddons; i++) {
        if (myAddons[i].MeasuresJSON) {
            myAddons[i].MeasuresJSON(pvParameters);
            delay(50);
        }
    }
}

#endif