#include <Arduino.h>                // Arduino framework
#include "Config.h"
#include "PoolMaster.h"
#include "BCM68X.h"
//#include "RF433T.h"

void stack_mon(UBaseType_t&);

//Init All Addons
void AddonsInit()
{
#ifdef BME68X
    BCM68XInit();
#endif
#ifdef RF433T
    RF433TInit();
#endif
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

#ifdef BME68X
    BCM68XAction(pvParameters);
#endif
#ifdef RF433T
    RF433TAction(pvParameters);
#endif

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
