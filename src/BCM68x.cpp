// Plant Room : Air Temp., Pressure, Humidity BASED ON BCM68X - i2c
// from https://github.com/m5stack/M5Unit-ENV/blob/master/examples/ENV_PRO/ENV_PRO.ino
// BME68x Sensor library: https://github.com/boschsensortec/Bosch-BME68x-Library
// BSEC2 Software Library: https://github.com/boschsensortec/Bosch-BSEC2-Library

#include <Arduino.h>                // Arduino framework
#include "Config.h"
#include "PoolMaster.h"
#include "BCM68X.h"                     
#include <bsec2.h>

// BCM68X data structure

static const char *BCM68XName = "BCM688";
static double PlantTemp, PlantHumidity, PlantPressure;
static Bsec2 PR_envSensorBME688;
static bool PR_BME688 = false; // no BME688 sensor by default
void PR_BME688newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);

extern void PublishTopic(char*, JsonDocument&);
static char PoolTopicMeas[25] = POOLTOPIC"Meas_";
static char PoolTopicSet[25]  = POOLTOPIC"Set_";

void lockI2C();
void unlockI2C();

void BCM68XInit(void)
{
/* Desired subscription list of BSEC2 outputs */
bsecSensor sensorList[] = {
    BSEC_OUTPUT_IAQ,          BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE, BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,      BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS};

/* Initialize the library and interfaces */
lockI2C();
if (PR_envSensorBME688.begin(BME68X_I2C_ADDR_HIGH, Wire)) 
    PR_BME688 = true;
unlockI2C();

/* Subsribe to the desired BSEC2 outputs */
if (!PR_envSensorBME688.updateSubscription(sensorList, ARRAY_LEN(sensorList), BSEC_SAMPLE_RATE_LP)) 
    PR_BME688 = false;

if (!PR_BME688) {
    Debug.print(DBG_INFO,"no BME68X");
    Debug.print(DBG_ERROR,"BME688 Init : BSEC Error : %d", PR_envSensorBME688.status);
    Debug.print(DBG_ERROR,"BME688 Init : BME68x Error : %d", PR_envSensorBME688.sensor.status);
    return;
}

/* Whenever new data is available call the newDataCallback function */
PR_envSensorBME688.attachCallback(PR_BME688newDataCallback);

Debug.print(DBG_INFO,"BSEC library version %d.%d.%d.%d",
            PR_envSensorBME688.version.major, PR_envSensorBME688.version.minor,
            PR_envSensorBME688.version.major_bugfix, PR_envSensorBME688.version.minor_bugfix);

strcat(PoolTopicMeas, BCM68XName);
strcat(PoolTopicSet,  BCM68XName);
}

void BCM68XAction(void *pvParameters)
{
    if (PR_BME688) {
        lockI2C();
        PR_envSensorBME688.run();
        unlockI2C();
    }
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
                PlantTemp = (double)(output.signal);
                Debug.print(DBG_ERROR,"BME688 Temp.=%f", PlantTemp);
                break;
            case BSEC_OUTPUT_RAW_PRESSURE:
                PlantPressure = (double)(output.signal);
                Debug.print(DBG_ERROR,"BME688 Pressure=%f", PlantPressure);
                break;
            case BSEC_OUTPUT_RAW_HUMIDITY:
                PlantHumidity = (double)(output.signal);
                Debug.print(DBG_ERROR,"BME688 Humidity=%f", PlantHumidity);
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

//  Publish BCM688 -> MQTT
void BCM68XMeasureJSON (void *pvParameters)
{
    //send a JSON to MQTT broker. /!\ Split JSON if longer than 100 bytes
    const int capacity = JSON_OBJECT_SIZE(3); // only 3 values to publish
    StaticJsonDocument<capacity> root;

    root["PlantTemp"]       = PlantTemp;
    root["PlantHumidity"]   = PlantHumidity;
    root["PlantPressure"]   = PlantPressure;

    PublishTopic(PoolTopicMeas, root);
}

void BCM68XSettingsJSON(void *pvParameters)
{
    return;  // No settings to store in MQTT
}
