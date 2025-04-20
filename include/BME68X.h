// Air Plant Temp., Pressure and Humidity  

#define _BME68X_

extern const char* BME68XName;

void BME68XInit(void);
void BME68XAction(void*);
void BME68XSettingsJSON(void);
void BME68XMeasureJSON(void);
