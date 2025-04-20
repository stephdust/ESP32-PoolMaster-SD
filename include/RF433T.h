
// 433 Mhz Temperature transmitter

#define _RF433T_

extern const char* RF433TName;

void RF433TInit(void);
void RF433TAction(void*);
void RF433TSettingsJSON(void);
void RF433TMeasureJSON(void);