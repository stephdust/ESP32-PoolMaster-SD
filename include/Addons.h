#ifndef ADDONS_H
#define ADDONS_H
#define _ADDONS_
typedef void (*function) (void*);

struct AddonStruct {
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

void  AddonsInit(void);
void  AddonsLoadSettings(void*);
void  AddonsSaveSettings(void*);
void  AddonsPublishSettings(void*);
void  AddonsPublishMeasures(void*);
void  AddonsHistoryStats(void*);
int   AddonsNb(void);
char* AddonsCreateMQTTTopic(const char*,const char*);

#define SANITYDELAY delay(50);

#endif

