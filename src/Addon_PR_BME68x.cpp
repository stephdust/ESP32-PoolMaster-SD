// Tech Room : Air Temp., Pressure, Humidity BASED ON BME68X - i2c
// from https://github.com/m5stack/M5Unit-ENV/blob/master/examples/ENV_PRO/ENV_PRO.ino
// BME68x Sensor library: https://github.com/boschsensortec/Bosch-BME68x-Library
// BSEC2 Software Library: https://github.com/boschsensortec/Bosch-BSEC2-Library


#include <Arduino.h>                // Arduino framework
#include "Config.h"
#include "PoolMaster.h"

#if defined(_ADDONS_) && defined(_IO_ADDON_PR_BME68X_)
#include "Addon_PR_BME68X.h"                     
#include <bsec2.h>

const char *PR_BME68XName = "PoolRoom";
static bool PR_BME688 = false; // no BME688 sensor by default
AddonStruct myPR_BME68X;

static double PR_Temp, PR_Humidity, PR_Pressure;
static Bsec2 PR_envSensorBME688;

void PR_BME688newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);

extern void PublishTopic(const char*, JsonDocument&);
static char *PoolTopicMeas=0, *PoolTopicSet=0;

void lockI2C();
void unlockI2C();

void PR_BME68XTask(void *pvParameters)
{
    if (!PR_BME688) return;

    lockI2C();
    PR_envSensorBME688.run();
    unlockI2C();
}

//  Publish BME688 -> MQTT
void PR_BME68XMeasureJSON (void *pvParameters)
{
    //send a JSON to MQTT broker. /!\ Split JSON if longer than 100 bytes
    const int capacity = JSON_OBJECT_SIZE(3); // only 3 values to publish
    StaticJsonDocument<capacity> root;

    if (!PR_BME688) return;

    root["Temperature"] = PR_Temp;
    root["Humidity"]    = PR_Humidity;
    root["Pressure"]    = PR_Pressure;

    PublishTopic(PoolTopicMeas, root);
}

void PR_BME68XSettingsJSON(void *pvParameters)
{
    if (!PR_BME688) return;
    // No settings to store in MQTT
}


AddonStruct PR_BME68XInit(void)
{
/* Desired subscription list of BSEC2 outputs */
bsecSensor sensorList[] = {
    BSEC_OUTPUT_IAQ,          BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE, BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,      BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS};

/* Initialize the library and interfaces */
lockI2C();
if (PR_envSensorBME688.begin(BME68X_I2C_ADDR_HIGH, Wire)) PR_BME688 = true;
unlockI2C();

/* Subsribe to the desired BSEC2 outputs */
if (!PR_envSensorBME688.updateSubscription(sensorList, ARRAY_LEN(sensorList), BSEC_SAMPLE_RATE_LP)) 
    PR_BME688 = false;

if (!PR_BME688) {
    Debug.print(DBG_INFO,"no BME68X");
    Debug.print(DBG_ERROR,"BME688 Init : BSEC Error : %d", PR_envSensorBME688.status);
    Debug.print(DBG_ERROR,"BME688 Init : BME68x Error : %d", PR_envSensorBME688.sensor.status);

}
else {

    /* Whenever new data is available call the newDataCallback function */
    PR_envSensorBME688.attachCallback(PR_BME688newDataCallback);

    Debug.print(DBG_INFO,"BSEC library version %d.%d.%d.%d",
            PR_envSensorBME688.version.major, PR_envSensorBME688.version.minor,
            PR_envSensorBME688.version.major_bugfix, PR_envSensorBME688.version.minor_bugfix);

    }

if (!PoolTopicMeas) PoolTopicMeas = (char*)malloc(strlen(POOLTOPIC)+strlen(PR_BME68XName)+5);
sprintf(PoolTopicMeas, "%sMeas%s", POOLTOPIC, PR_BME68XName);
if (!PoolTopicSet)  PoolTopicSet = (char*)malloc(strlen(POOLTOPIC)+strlen(PR_BME68XName)+4);
sprintf(PoolTopicSet,  "%sSet%s", POOLTOPIC, PR_BME68XName);

myPR_BME68X.name         = PR_BME68XName;
myPR_BME68X.Task         = PR_BME68XTask;
myPR_BME68X.frequency    = 5000;     // Update values every 5 secs.
myPR_BME68X.SettingsJSON = PR_BME68XSettingsJSON;
myPR_BME68X.MeasuresJSON = PR_BME68XMeasureJSON;
myPR_BME68X.HistoryStats = 0;

if (PR_BME688) 
    xTaskCreatePinnedToCore(
        AddonLoop,
        myPR_BME68X.name,
        3072,
        &myPR_BME68X,
        1,
        nullptr,
        xPortGetCoreID()
    );

  return myPR_BME68X;
}



void PR_BME688newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec)
{
    if (!outputs.nOutputs) return;

    for (uint8_t i = 0; i < outputs.nOutputs; i++) {
        const bsecData output = outputs.output[i];
        switch (output.sensor_id) {
            case BSEC_OUTPUT_IAQ:
                Debug.print(DBG_ERROR,"BME688 IAQ=%s, IAQ Accurary=%s", String(output.signal), String((int)output.accuracy));
                break;
            case BSEC_OUTPUT_RAW_TEMPERATURE:
                PR_Temp = (double)(output.signal);
                Debug.print(DBG_ERROR,"BME688 Temp.=%f", PR_Temp);
                break;
            case BSEC_OUTPUT_RAW_PRESSURE:
                PR_Pressure = (double)(output.signal);
                Debug.print(DBG_ERROR,"BME688 Pressure=%f", PR_Pressure);
                break;
            case BSEC_OUTPUT_RAW_HUMIDITY:
                PR_Humidity = (double)(output.signal);
                Debug.print(DBG_ERROR,"BME688 Humidity=%f", PR_Humidity);
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

