
#define BCM68X  // Air Plant Temp., Pressure and Humidity  

void BCM68XInit(void);
void BCM68XAction(void*);

// BCM68X data structure
struct BCM68XStoreStruct
{
double PlantTemp, PlantHumidity, PlantPressure;
};

