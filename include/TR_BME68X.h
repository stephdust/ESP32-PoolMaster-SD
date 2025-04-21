// Air TechRoom Temp., Pressure and Humidity  

#ifdef _TR_BME68X_

extern const char* TR_BME68XName;
void TR_BME68XInit(void);
void TR_BME68XAction(void*);
void TR_BME68XSettingsJSON(void);
void TR_BME68XMeasureJSON(void);

#endif
