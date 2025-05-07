#ifndef _EXTENSIONS_
#define _EXTENSIONS_

typedef void (*function) (void*);

struct ExtensionStruct {
    const char* name;
    TickType_t  frequency;
    bool        detected;
    function    Task;
    function    LoadSettings;
    function    SaveSettings;
    function    LoadMeasures;
    function    SaveMeasures;
    function    HistoryStats;
    char*       MQTTTopicSettings;
    char*       MQTTTopicMeasures;
};

typedef ExtensionStruct (*initfunction) (const char*, int);

void  ExtensionsInit(void);
void  ExtensionsLoadSettings(void*);
void  ExtensionsSaveSettings(void*);
void  ExtensionsPublishSettings(void*);
void  ExtensionsPublishMeasures(void*);
void  ExtensionsHistoryStats(void*);
int   ExtensionsNb(void);
char* ExtensionsCreateMQTTTopic(const char*,const char*);

#define SANITYDELAY delay(50);
 
#endif

