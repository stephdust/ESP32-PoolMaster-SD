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
void AddonLoop(void*);
void AddonsPublishSettings(void*);
void AddonsPublishMeasures(void*);

#endif

