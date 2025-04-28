#ifndef ADDONS_H
#define ADDONS_H
#define _ADDONS_

typedef void (*function) (void*);

struct AddonStruct {
    const char *name;
    TickType_t frequency;
    function Task;
    function LoadSettings;
    function SaveSettings;
    function LoadMeasures;
    function SaveMeasures;
    function HistoryStats;
};

void AddonsInit(void);
void AddonsLoadSettings(void*);
void AddonsSaveSettings(void*);
void AddonsPublishSettings(void*);
void AddonsPublishMeasures(void*);
void AddonsHistoryStats(void*);
int  AddonsNb(void);
//void AddonsPublishTopic(char*, JsonDocument&);
//void AddonsReadRetainedTopic(char*, JsonDocument&);
void AddonsLoop(void*);
#endif

