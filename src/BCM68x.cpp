// Plant Room : Air Temp., Pressure and Humidity BASED ON BCM68X - i2c
// from https://github.com/m5stack/M5Unit-ENV/blob/master/examples/ENV_PRO/ENV_PRO.ino
// BME68x Sensor library: https://github.com/boschsensortec/Bosch-BME68x-Library
// BSEC2 Software Library: https://github.com/boschsensortec/Bosch-BSEC2-Library

#include <Arduino.h>                // Arduino framework
#include "Config.h"
#include "PoolMaster.h"
#include "BCM68X.h"                     
#include <bsec2.h>

// BCM68X data structure
static BCM68XStoreStruct BCM688_Values;
static Bsec2 PR_envSensorBME688;
static bool PR_BME688 = false; // no BME688 sensor by default
void PR_BME688newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);

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
if (PR_envSensorBME688.begin(BME68X_I2C_ADDR_HIGH, Wire)) {
    PR_BME688 = true;
}
unlockI2C();

/* Subsribe to the desired BSEC2 outputs */
if (!PR_envSensorBME688.updateSubscription(sensorList, ARRAY_LEN(sensorList), BSEC_SAMPLE_RATE_LP)) {
    PR_BME688 = false;
}

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

}

void BCM68XAction(void *pvParameters)
{
    if (PR_BME688) {
        lockI2C();
        PR_envSensorBME688.run();
        unlockI2C();
}


void PR_BME688newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec)
{
    if (!outputs.nOutputs) {
        return;
    }

/*

    Serial.println(
        "BSEC outputs:\n\ttimestamp = " +
        String((int)(outputs.output[0].time_stamp / INT64_C(1000000))));
    for (uint8_t i = 0; i < outputs.nOutputs; i++) {
        const bsecData output = outputs.output[i];
        switch (output.sensor_id) {
            case BSEC_OUTPUT_IAQ:
                Serial.println("\tiaq = " + String(output.signal));
                Serial.println("\tiaq accuracy = " +
                               String((int)output.accuracy));
                break;
            case BSEC_OUTPUT_RAW_TEMPERATURE:
                Serial.println("\ttemperature = " + String(output.signal));
                break;
            case BSEC_OUTPUT_RAW_PRESSURE:
                Serial.println("\tpressure = " + String(output.signal));
                break;
            case BSEC_OUTPUT_RAW_HUMIDITY:
                Serial.println("\thumidity = " + String(output.signal));
                break;
            case BSEC_OUTPUT_RAW_GAS:
                Serial.println("\tgas resistance = " + String(output.signal));
                break;
            case BSEC_OUTPUT_STABILIZATION_STATUS:
                Serial.println("\tstabilization status = " +
                               String(output.signal));
                break;
            case BSEC_OUTPUT_RUN_IN_STATUS:
                Serial.println("\trun in status = " + String(output.signal));
                break;
            default:
                break;
        }
      }
    }

*/

}

//  Publish MQTT BCM
