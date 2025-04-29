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

AddonStruct myPR_BME68X = {0};

static double PR_Temp, PR_Humidity, PR_Pressure;
static Bsec2 PR_envSensorBME688;

void PR_BME688newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);
void AddonsPublishTopic(char*, JsonDocument&);
void lockI2C();
void unlockI2C();

//  Publish BME688 -> MQTT
void  PR_BME68X_SaveMeasures(void *pvParameters)
{
    //send a JSON to MQTT broker. /!\ Split JSON if longer than 100 bytes
    const int capacity = JSON_OBJECT_SIZE(3); // only 3 values to publish
    StaticJsonDocument<capacity> root;

    if (!myPR_BME68X.detected) return;

    root["Temperature"] = PR_Temp;
    root["Humidity"]    = PR_Humidity;
    root["Pressure"]    = PR_Pressure;

    AddonsPublishTopic(myPR_BME68X.MQTTTopicMeasures, root);
}

void PR_BME68XTask(void *pvParameters)
{
    if (!myPR_BME68X.detected) return;

    lockI2C();
    PR_envSensorBME688.run();
    unlockI2C();
    PR_BME68X_SaveMeasures(pvParameters);
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
    if (PR_envSensorBME688.begin(BME68X_I2C_ADDR_HIGH, Wire)) myPR_BME68X.detected = true;
    unlockI2C();

    /* Subsribe to the desired BSEC2 outputs */
    if (!PR_envSensorBME688.updateSubscription(sensorList, ARRAY_LEN(sensorList), BSEC_SAMPLE_RATE_LP)) 
        myPR_BME68X.detected = false;

    if (!myPR_BME68X.detected) {
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

    myPR_BME68X.name                = "PoolRoom";
    myPR_BME68X.Task                = PR_BME68XTask;
    myPR_BME68X.frequency           = 5000;     // Update values every 5 secs.
    myPR_BME68X.LoadSettings        = 0;
    myPR_BME68X.SaveSettings        = 0;
    myPR_BME68X.LoadMeasures        = 0;
    myPR_BME68X.SaveMeasures        = PR_BME68X_SaveMeasures;
    myPR_BME68X.HistoryStats        = 0;
    myPR_BME68X.MQTTTopicSettings   = 0;
    myPR_BME68X.MQTTTopicMeasures   = AddonsCreateMQTTTopic(myPR_BME68X.name, "Meas");

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

