
#include <Arduino.h>
#include "Config.h"
#include "PoolMaster.h"


#if defined(_EXTENSIONS_)

// *************************
// Addons on Extension Ports
// *************************
// Check GPIO IDs possibilities and limitations 
// https://www.upesy.fr/blogs/tutorials/esp32-pinout-reference-gpio-pins-ultimate-guide
//
// GPIOs Available :
//  GPIO O5   --> Input+Output + pullup/down
//  GPIO 12   --> Input+Output + pullup/down
//  GPIO 14   --> Input+Output + pullup/down
//  GPIO 15   --> Input(pullup) + output 
//  GPIO 35   --> Input only, no pullup/pulldown
//  I2C       --> Address

#include "Extension_Ports.h"
#include "Extension_TFAVenice_RF433T.h"
#include "Extension_WaterMeter_Pulse.h"
#include "Extension_BME680_0x77.h"

#define _GPIO_TFA_RF433T_        5  // Water Temperature broadcasted to TFA 433Mhz receiver
#define _GPIO_WATERMETER_PULSE_ 14  // Count WaterMeter pulse from external reader.
#define _I2C_                    0
#define _OTHER_                 -1

struct ListExtensions
{
    const char *name;   // name of the extension
    initfunction init;  // function to init the extension
    int port;           // Port (GPIO, I2C, etc...)
};

// *******************************************
// * This is where we declare our extensions *
// *******************************************
struct ListExtensions myListExtensions[] = {
    {"PoolRoom/TFAVenice",      TFAVenice_RF433TInit,  _GPIO_TFA_RF433T_ },
    {"TechRoom/TempHumidity",   BME680_0x77Init,       _I2C_ },
    {"TechRoom/WaterMeter",     WaterMeterPulseInit,   _GPIO_WATERMETER_PULSE_ }
//    {"PoolRoom/MiLight",        MiLightInit,           _I2C_ },
//    {"PoolRoom/PoolCover",      PoolCoverInit,         _OTHER_ },
//    {"PoolRoom/TempHumidity",   BME680_0x76Init,       _I2C_ },
//    {"TechRoom/DisplayOLED",    OLED_SSD1367_0x3cInit, _I2C_ },
//    {"TechRoom/PoolHeatPump",   PoolHeatPumpInit,      _OTHER_ }
};
// *******************************************


void stack_mon(UBaseType_t &);
ExtensionStruct *myExtensions = 0;
static int _NbExtensions = 0;


/* ********************************************************
 * The Async MQTT engine for Extensions
 * Can't use the Poolmaster mqtt instance
 * because of how we store and read values in mqtt broker
*/

AsyncMqttClient ExtensionsMqttClient;
#define PAYLOAD_BUFFER_LENGTH 200
static bool ExtensionsMqttOnRead = false;
static char ExtensionsMqttMsg[PAYLOAD_BUFFER_LENGTH] = "";
static char *ExtensionsMqttMsgTopic = 0;
TimerHandle_t mqttExtensionsReconnectTimer;


// Removes the duplicates "/" from the string provided as an argument
static void mqttRemoveDoubleSlash(char *str)
{
  int i,j,len,len1;
  /*calculating length*/
  for(len=0; str[len]!='\0'; len++);
  /*assign 0 to len1 - length of removed characters*/
  len1=0;
  /*Removing consecutive repeated characters from string*/
  for(i=0; i<(len-len1);) {
      if((str[i]==str[i+1]) && (str[i] == '/')) {
          /*shift all characters*/
          for(j=i;j<(len-len1);j++)
              str[j]=str[j+1];
          len1++;
      }
      else i++;
  }
}

void connectExtensionsToMqtt() {
    Serial.println("Extensions Connecting to MQTT...");
    ExtensionsMqttClient.connect();
}
void onExtensionsMqttConnect(bool sessionPresent) {
    Serial.println("Extensions Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);
}
void onExtensionsMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.println("Extensions Disconnected from MQTT.");
  
    if (WiFi.isConnected()) {
      xTimerStart(mqttExtensionsReconnectTimer, 0);
    }
  }
void onExtensionsMqttSubscribe(uint16_t packetId, uint8_t qos) {
    Serial.println("Extensions Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
  }
void onExtensionsMqttUnsubscribe(uint16_t packetId) {
    Serial.println("Extensions Unsubscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}
// Extension MQTT callback reading RETAINED values
void onExtensionsMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    if (!ExtensionsMqttMsgTopic) return;
    if (!topic) return;
       
Debug.print(DBG_DEBUG,"[onExtensionsMqttMessage]  topic=%s mqttsmg=%s",topic, ExtensionsMqttMsgTopic );

    if (strcmp(topic, ExtensionsMqttMsgTopic) == 0)
        for (uint8_t i = 0; i < len; i++)
            ExtensionsMqttMsg[i] = payload[i];

Debug.print(DBG_DEBUG,"[onExtensionsMqttMessage]  ExtensionsMqttMsg=%s ",ExtensionsMqttMsg );            
}
void onExtensionsMqttPublish(uint16_t packetId) {
  Serial.println("Extensions Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}
void ExtensionsMqttInit()
{
    // Init 2nd Async MQTT session for Extensions
    mqttExtensionsReconnectTimer = xTimerCreate("mqttExtensionsTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectExtensionsToMqtt));
 
    ExtensionsMqttClient.onConnect(onExtensionsMqttConnect);
    ExtensionsMqttClient.onDisconnect(onExtensionsMqttDisconnect);
    ExtensionsMqttClient.onSubscribe(onExtensionsMqttSubscribe);
    ExtensionsMqttClient.onUnsubscribe(onExtensionsMqttUnsubscribe);
    ExtensionsMqttClient.onMessage(onExtensionsMqttMessage);
    ExtensionsMqttClient.onPublish(onExtensionsMqttPublish);
    ExtensionsMqttClient.setServer(storage.MQTT_IP, storage.MQTT_PORT);
    if (strlen(storage.MQTT_LOGIN) > 0)
        ExtensionsMqttClient.setCredentials(storage.MQTT_LOGIN, storage.MQTT_PASS);

    Debug.print(DBG_INFO,"[ExtensionsMqttInit] Connect to MQTT %s, %d, %s, %s, %s %s",
                storage.MQTT_IP.toString().c_str(),
                storage.MQTT_PORT,storage.MQTT_LOGIN,storage.MQTT_PASS,
                storage.MQTT_ID,storage.MQTT_TOPIC);

    ExtensionsMqttClient.setClientId(storage.MQTT_ID);
    ExtensionsMqttClient.connect();
    //Debug.print(DBG_INFO, "[ExtensionsMqttInit] Connect to MQTT rc=%d", ExtensionsMqttClient.state());
}

char *ExtensionsCreateMQTTTopic(const char *t1, const char *t2)
{
    char *topic = (char *)malloc(strlen(POOLTOPIC) + strlen(t1) + strlen(t2) + 1);
    sprintf(topic, "%s/%s%s", POOLTOPIC, t1, t2);
    return topic;
}
void ExtensionsPublishTopic(char *topic, JsonDocument &root)
{
    ExtensionsMqttClient.connect();
    if (!ExtensionsMqttClient.connected()) {
        Debug.print(DBG_ERROR, "[ExtensionsPublishTopic] Failed to connect to the MQTT broker for %s", topic);
        return;
    }
    char Payload[PAYLOAD_BUFFER_LENGTH];
    size_t n = serializeJson(root, Payload);
    mqttRemoveDoubleSlash(topic);

    if (ExtensionsMqttClient.publish(topic, 1, true, Payload, n) != 0) {
        delay(50);
        Debug.print(DBG_DEBUG, "Publish Extensions: %s - size: %d/%d", Payload, root.size(), n);
        return;
    }

    Debug.print(DBG_DEBUG, "Unable to publish: %s", Payload);
}

int ExtensionsReadRetainedTopic(char *topic, JsonDocument &root)
{
    int nbTry = 5;
    while (ExtensionsMqttOnRead)
    {
        delay(200);
        nbTry--;
        if (!nbTry)
            ExtensionsMqttOnRead = false;
    }

    if (!ExtensionsMqttClient.connected())
    {
        Debug.print(DBG_ERROR, "[ExtensionsReadRetainedTopic] Failed to connect to the MQTT broker for %s", topic);
        return 0;
    }
    mqttRemoveDoubleSlash(topic);
    ExtensionsMqttOnRead = true;
    ExtensionsMqttMsgTopic = topic;
    ExtensionsMqttClient.subscribe(topic, 1);
    ExtensionsMqttClient.unsubscribe(topic);
    ExtensionsMqttOnRead = false;
    if (!ExtensionsMqttMsg[0]) return 0;

    deserializeJson(root, ExtensionsMqttMsg);
    ExtensionsMqttOnRead = false;
    ExtensionsMqttMsgTopic = 0;
    ExtensionsMqttMsg[0] = '\0';
    return 1;
}

/* **********************************
 * End of MQTT 
 * **********************************
 * /

/* **********************************
 * Init All Extensions and run loops
 * **********************************
*/

void ExtensionsLoop(void *pvParameters)
{
    ExtensionStruct *myExtension = (ExtensionStruct *)pvParameters;
    TickType_t period = myExtensions->frequency;

    while (!startTasks)
        delay(200);
    vTaskDelay(500 / portTICK_PERIOD_MS); // Scheduling offset

    TickType_t ticktime = xTaskGetTickCount();
    UBaseType_t hwm = 0;

#ifdef CHRONO
    unsigned long td;
    int t_act = 0, t_min = 999, t_max = 0;
    float t_mean = 0.;
    int n = 1;
#endif

    Debug.print(DBG_INFO, "[Start ExtensionLoop] name %s freq: %d ms", myExtension->name, myExtension->frequency);
    for (;;)
    {

#ifdef CHRONO
        td = millis();
#endif
 //       Debug.print(DBG_INFO, "[In ExtensionLoop] name %s freq: %d ms", myExtension->name, myExtension->frequency);
        if (myExtension->Task)
            myExtension->Task(pvParameters);
        SANITYDELAY;

#ifdef CHRONO
        t_act = millis() - td;
        if (t_act > t_max)
            t_max = t_act;
        if (t_act < t_min)
            t_min = t_act;
        t_mean += (t_act - t_mean) / n;
        ++n;
 //       Debug.print(DBG_INFO, "[Extensions Action] name:%s td: %d t_act: %d t_min: %d t_max: %d t_mean: %4.1f", myExtension->name, td, t_act, t_min, t_max, t_mean);
#endif

        stack_mon(hwm);
        vTaskDelayUntil(&ticktime, period);
    }
}


void ExtensionsInit()
{
    // https://i2cdevices.org/addresses/
    Debug.print(DBG_INFO, "\nI2C Scanner...scanning...");
    byte error, address;
    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0)      Debug.print(DBG_INFO,"\tI2C device found at address 0x%x", address < 16 ? 0 : address);
        else if (error == 4) Debug.print(DBG_INFO,"\tI2C device Unknow error at address 0x%x", address < 16 ? 0 : address);
    }
    Debug.print(DBG_INFO, "...done");

    ExtensionsMqttInit();
    
    Debug.print(DBG_INFO, "[Extensions Init Action]");
    if (sizeof(myListExtensions[0]))
        _NbExtensions = sizeof(myListExtensions)  / sizeof(myListExtensions[0]);
    Debug.print(DBG_INFO, "[Extensions Init] with %d extensions", _NbExtensions);

    if (_NbExtensions) myExtensions = (ExtensionStruct*) malloc(_NbExtensions * sizeof(ExtensionStruct));

    for (int i = 0; i < _NbExtensions; i++) {    
        myExtensions[i] = myListExtensions[i].init(myListExtensions[i].name, myListExtensions[i].port);
        if (myExtensions[i].detected)
            xTaskCreatePinnedToCore(
                ExtensionsLoop,
                myExtensions[i].name,
                3072, // can we decrease this value ??
                &myExtensions[i],
                1,
                nullptr,
                xPortGetCoreID());
        }
}

void ExtensionsPublishSettings(void *pvParameters)
{
    for (int i = 0; i < _NbExtensions; i++)
    {
        if (myExtensions[i].SaveSettings)
        {
            myExtensions[i].SaveSettings(pvParameters);
            SANITYDELAY;
        }
    }
}

void ExtensionsPublishMeasures(void *pvParameters)
{
    for (int i = 0; i < _NbExtensions; i++)
    {
        if (myExtensions[i].SaveMeasures)
        {
            myExtensions[i].SaveMeasures(pvParameters);
            SANITYDELAY;
        }
    }
}

void ExtensionsHistoryStats(void *pvParameters)
{
    for (int i = 0; i < _NbExtensions; i++)
    {
        if (myExtensions[i].HistoryStats)
        {
            myExtensions[i].HistoryStats(pvParameters);
            SANITYDELAY;
        }
    }
}

int ExtensionsNb()
{
    return _NbExtensions;
}

#endif