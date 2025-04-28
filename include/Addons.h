#ifndef ADDONS_H
#define ADDONS_H

#define _ADDONS_
typedef void (*function) (void*);
struct AddonStruct {
    const char *name;
    TickType_t frequency;
    function Task;
    function LoadConfig;
    function SaveConfig;
    function SettingsJSON;
    function MeasuresJSON;
    function HistoryStats;
};

void AddonsInit();
void AddonsLoadConfig(void*);
void AddonsSaveConfig(void*);
void AddonsPublishSettings(void*);
void AddonsPublishMeasures(void*);
void AddonsHistoryStats(void*);
int  AddonsNb();
void AddonLoop(void*);
#endif

