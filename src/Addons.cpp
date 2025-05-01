
#include <Arduino.h>
#include "Config.h"
#include "PoolMaster.h"

#ifdef _ADDONS_
#ifdef _IO_ADDON_TR_BME68X_
#include "Addon_TR_BME68X.h"
#endif
#ifdef _IO_ADDON_PR_BME68X_
#include "Addon_PR_BME68X.h"
#endif
#ifdef _IO_ADDON_TFA_RF433T_
#include "Addon_TFA_RF433T.h"
#endif
#ifdef _IO_ADDON_WATERMETER_PULSE_
#include "Addon_WaterMeter_Pulse.h"
#endif
//#include "Addon_Dehumidifier.h"
//#include "Addon_PoolCover.h"
//#include "Addon_MiLight.h"

void stack_mon(UBaseType_t&);

static int _NbAddons = 0;
static AddonStruct myAddons[_MaxAddons_] ;


AsyncMqttClient AddonsMqttClient;
#define PAYLOAD_BUFFER_LENGTH 200
static bool AddonsMqttOnRead = false;
static char AddonsMqttMsg[PAYLOAD_BUFFER_LENGTH] = "";
static char *AddonsMqttMsgTopic = 0;

// Addon MQTT callback reading RETAINED values
void onAddonsMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  if (strcmp(topic,AddonsMqttMsgTopic)==0)
    for (uint8_t i=0 ; i<len ; i++) 
        AddonsMqttMsg[i] = payload[i];
}


void AddonsMqttInit() 
{
    //Init 2nd Async MQTT session for Addons
    AddonsMqttClient.setServer(storage.MQTT_IP,storage.MQTT_PORT);
    if(strlen(storage.MQTT_LOGIN)>0) AddonsMqttClient.setCredentials(storage.MQTT_LOGIN,storage.MQTT_PASS);
    // AddonsMqttClient.setClientId(storage.MQTT_ID);
    AddonsMqttClient.onMessage(onAddonsMqttMessage);
} 

char *AddonsCreateMQTTTopic(const char *t1, const char *t2)
{
    char *topic = (char*)malloc(strlen(POOLTOPIC)+strlen(t1)+strlen(t2)+1);
    sprintf(topic, "%s/%s%s", POOLTOPIC, t1, t2);
    return topic;
}


void AddonsPublishTopic(char* topic, JsonDocument& root)
{
    if (!AddonsMqttClient.connected()) {
        Debug.print(DBG_ERROR,"Failed to connect to the MQTT broker");
        return;
    }
    char Payload[PAYLOAD_BUFFER_LENGTH];
    size_t n=serializeJson(root,Payload);
    remove_duplicates_slash(topic);

    if (AddonsMqttClient.publish(topic, 1, true, Payload,n) != 0) {
      delay(50);
      Debug.print(DBG_DEBUG,"Publish Addons: %s - size: %d/%d",Payload,root.size(),n);
      return;
    }

    Debug.print(DBG_DEBUG,"Unable to publish: %s",Payload);
}

int AddonsReadRetainedTopic(char* topic, JsonDocument& root)
{
    int nbTry = 5;
    while (AddonsMqttOnRead) {
        delay(200);
        nbTry--;
        if (!nbTry) AddonsMqttOnRead = false;
    }

    if (!AddonsMqttClient.connected()) {
        Debug.print(DBG_ERROR,"Failed to connect to the MQTT broker");
        return 0;
    }
    remove_duplicates_slash(topic);
    AddonsMqttOnRead = true;
    AddonsMqttMsgTopic = topic;
    AddonsMqttClient.subscribe(topic, 1);
    AddonsMqttClient.unsubscribe(topic);

    if (!AddonsMqttMsg[0]) return 0; 
    
    deserializeJson(root, AddonsMqttMsg);
    AddonsMqttOnRead = false;
    AddonsMqttMsgTopic = 0;
    AddonsMqttMsg[0] = '\0';
    return 1;
}


//Init All Addons and run loops


void AddonsLoop(void *pvParameters)
{
  AddonStruct *myAddon  = (AddonStruct*)pvParameters;
  TickType_t period     = myAddons->frequency;

  while (!startTasks) delay(200);
  vTaskDelay(500/portTICK_PERIOD_MS);           // Scheduling offset 

  TickType_t ticktime = xTaskGetTickCount();
  UBaseType_t hwm = 0;
 
  #ifdef CHRONO
  unsigned long td;
  int t_act=0,t_min=999,t_max=0;
  float t_mean=0.;
  int n=1;
  #endif

  Debug.print(DBG_INFO,"[Start AddonLoop] name %s freq: %d ms",myAddon->name,myAddon->frequency);
  for(;;)
  {       
    vTaskDelayUntil(&ticktime,period);

    #ifdef CHRONO
    td = millis();
    #endif 

    if (myAddon->Task) myAddon->Task(pvParameters);
    SANITYDELAY;
    
    #ifdef CHRONO
    t_act = millis() - td;
    if(t_act > t_max) t_max = t_act;
    if(t_act < t_min) t_min = t_act;
    t_mean += (t_act - t_mean)/n;
    ++n;
    Debug.print(DBG_INFO,"[Addons Action] name:%s td: %d t_act: %d t_min: %d t_max: %d t_mean: %4.1f",myAddon->name,td,t_act,t_min,t_max,t_mean);
    #endif

    stack_mon(hwm);
  }
}

void AddonsInit()
{
    memset(myAddons, 0, sizeof(myAddons));
    AddonsMqttInit();
    Debug.print(DBG_INFO,"[Addons Action]");
#ifdef _IO_ADDON_TR_BME68X_
    myAddons[_NbAddons++] = TR_BME68XInit();
#endif
#ifdef _IO_ADDON_PR_BME68X_
    myAddons[_NbAddons++] = PR_BME68XInit();
#endif
#ifdef _IO_ADDON_TFA_RF433T_
    myAddons[_NbAddons++] = TFA_RF433TInit();
#endif
#ifdef _IO_ADDON_WATERMETER_PULSE_
    myAddons[_NbAddons++] = WaterMeterPulseInit();
#endif

    for (int i=0; i<_NbAddons; i++) {
        if (myAddons[i].detected)
            xTaskCreatePinnedToCore(
                AddonsLoop,
                myAddons[i].name,
                3072,       // can we decrease this value ??
                &myAddons[i],
                1,
                nullptr,
                xPortGetCoreID()
                );
        }
}



void AddonsPublishSettings(void *pvParameters)
{
    for (int i=0; i<_NbAddons; i++) {
        if (myAddons[i].SaveSettings) {
            myAddons[i].SaveSettings(pvParameters);
            SANITYDELAY;
        }
    }
}

void AddonsPublishMeasures(void *pvParameters)
{
    for (int i=0; i<_NbAddons; i++) {
        if (myAddons[i].SaveMeasures) {
            myAddons[i].SaveMeasures(pvParameters);
            SANITYDELAY;
        }
    }
}

void AddonsHistoryStats(void* pvParameters)
{  
    for (int i=0; i<_NbAddons; i++) {
        if (myAddons[i].HistoryStats) {
            myAddons[i].HistoryStats(pvParameters);
            SANITYDELAY;
        }
    }
}

int AddonsNb()
{
    return _NbAddons;
}


#endif
