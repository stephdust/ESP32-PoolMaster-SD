// Tech Room : Air Temp., Pressure, Humidity BASED ON BME68X - i2c
// from https://github.com/m5stack/M5Unit-ENV/blob/master/examples/ENV_PRO/ENV_PRO.ino
// BME68x Sensor library: https://github.com/boschsensortec/Bosch-BME68x-Library
// BSEC2 Software Library: https://github.com/boschsensortec/Bosch-BSEC2-Library


#include <Arduino.h>                // Arduino framework
#include "Config.h"
#include "PoolMaster.h"

#if defined(_ADDONS_) && defined(_TR_BME68X_)
#include "TR_BME68X.h"                     
#include <bsec2.h>

const char *TR_BME68XName = "TechRoom";
static bool TR_BME688 = false; // no BME688 sensor by default
AddonStruct myTR_BME68X;

static double TRTemp, TRHumidity, TRPressure;
static Bsec2 TR_envSensorBME688;

void TR_BME688newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);

extern void PublishTopic(const char*, JsonDocument&);
static char *PoolTopicMeas, *PoolTopicSet;

void lockI2C();
void unlockI2C();

void TR_BME68XTask(void *pvParameters)
{
    if (!TR_BME688) return;

    lockI2C();
    TR_envSensorBME688.run();
    unlockI2C();
}

//  Publish BME688 -> MQTT
void TR_BME68XMeasureJSON (void *pvParameters)
{
    //send a JSON to MQTT broker. /!\ Split JSON if longer than 100 bytes
    const int capacity = JSON_OBJECT_SIZE(3); // only 3 values to publish
    StaticJsonDocument<capacity> root;

    if (!TR_BME688) return;

    root["Temperature"] = TRTemp;
    root["Humidity"]    = TRHumidity;
    root["Pressure"]    = TRPressure;

    PublishTopic(PoolTopicMeas, root);
}

void TR_BME68XSettingsJSON(void *pvParameters)
{
    if (!TR_BME688) return;
    // No settings to store in MQTT
}


AddonStruct TR_BME68XInit(void)
{
/* Desired subscription list of BSEC2 outputs */
bsecSensor sensorList[] = {
    BSEC_OUTPUT_IAQ,          BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE, BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,      BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS};

/* Initialize the library and interfaces */
lockI2C();
if (TR_envSensorBME688.begin(BME68X_I2C_ADDR_HIGH, Wire)) TR_BME688 = true;
unlockI2C();

/* Subsribe to the desired BSEC2 outputs */
if (!TR_envSensorBME688.updateSubscription(sensorList, ARRAY_LEN(sensorList), BSEC_SAMPLE_RATE_LP)) 
    TR_BME688 = false;

if (!TR_BME688) {
    Debug.print(DBG_INFO,"no BME68X");
    Debug.print(DBG_ERROR,"BME688 Init : BSEC Error : %d", TR_envSensorBME688.status);
    Debug.print(DBG_ERROR,"BME688 Init : BME68x Error : %d", TR_envSensorBME688.sensor.status);

}
else {

    /* Whenever new data is available call the newDataCallback function */
    TR_envSensorBME688.attachCallback(TR_BME688newDataCallback);

    Debug.print(DBG_INFO,"BSEC library version %d.%d.%d.%d",
            TR_envSensorBME688.version.major, TR_envSensorBME688.version.minor,
            TR_envSensorBME688.version.major_bugfix, TR_envSensorBME688.version.minor_bugfix);

    }

PoolTopicMeas = (char*)malloc(strlen(POOLTOPIC)+strlen(TR_BME68XName)+5);
sprintf(PoolTopicMeas, "%sMeas%s", POOLTOPIC, TR_BME68XName);
PoolTopicSet = (char*)malloc(strlen(POOLTOPIC)+strlen(TR_BME68XName)+4);
sprintf(PoolTopicSet, "%sSet%s", POOLTOPIC, TR_BME68XName);

myTR_BME68X.name         = TR_BME68XName;
myTR_BME68X.Task         = TR_BME68XTask;
myTR_BME68X.frequency    = 5000;     // Update values every 5 secs.
myTR_BME68X.SettingsJSON = TR_BME68XSettingsJSON;
myTR_BME68X.MeasuresJSON = TR_BME68XMeasureJSON;
myTR_BME68X.HistoryStats = 0;

if (TR_BME688) 
    xTaskCreatePinnedToCore(
        AddonLoop,
        myTR_BME68X.name,
        3072,
        &myTR_BME68X,
        1,
        nullptr,
        xPortGetCoreID()
    );

  return myTR_BME68X;
}



void TR_BME688newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec)
{
    if (!outputs.nOutputs) return;

    for (uint8_t i = 0; i < outputs.nOutputs; i++) {
        const bsecData output = outputs.output[i];
        switch (output.sensor_id) {
            case BSEC_OUTPUT_IAQ:
                Debug.print(DBG_ERROR,"BME688 IAQ=%s, IAQ Accurary=%s", String(output.signal), String((int)output.accuracy));
                break;
            case BSEC_OUTPUT_RAW_TEMPERATURE:
                TRTemp = (double)(output.signal);
                Debug.print(DBG_ERROR,"BME688 Temp.=%f", TRTemp);
                break;
            case BSEC_OUTPUT_RAW_PRESSURE:
                TRPressure = (double)(output.signal);
                Debug.print(DBG_ERROR,"BME688 Pressure=%f", TRPressure);
                break;
            case BSEC_OUTPUT_RAW_HUMIDITY:
                TRHumidity = (double)(output.signal);
                Debug.print(DBG_ERROR,"BME688 Humidity=%f", TRHumidity);
                break;
            case BSEC_OUTPUT_RAW_GAS:
                Debug.print(DBG_ERROR,"BME688 Gas=%s", String(output.signal));
                break;
            case BSEC_OUTPUT_STABILIZATION_STATUS:
                Debug.print(DBG_ERROR,"BME688 Stabilization=%s", String(output.signal));
                break;
            case BSEC_OUTPUT_RUN_IN_STATUS:
                Debug.print(DBG_ERROR,"BME688 Run in status=%s", String(output.signal));
                break;
            default:
                break;
        }
      }
}

#endif
