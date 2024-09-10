/*
            Pump - a simple library to handle home-pool filtration and peristaltic pumps
                 (c) Loic74 <loic74650@gmail.com> 2017-2020
Features: 

- keeps track of running time
- keeps track of Tank Levels
- set max running time limit

NB: all timings are in milliseconds
*/

#include "Arduino_DebugUtils.h"   // Debug.print

#ifndef PUMP_h
#define PUMP_h
#define PUMP_VERSION "1.0.1"

// PUMP ADDED CONFIGURATION PARAMETERS

//Constants used in some of the functions below
#define PUMP_ON  1
#define PUMP_OFF 0
#define TANK_FULL  0
#define TANK_EMPTY 1
#define INTERLOCK_OK  1
#define INTERLOCK_NOK 0
#define NO_LEVEL 170           // Pump with tank but without level switch
#define NO_TANK 255            // Pump without tank
#define NO_INTERLOCK 255  
#define PUMP_STD  1             // Underlying relay works normally
#define PUMP_MOMENTARY  2      // Underlying relay activate and deactive shortly after to simulate button press
#define MOMENTARY_SWITCH_SHORT_CLICK_DELAY  500 // In Milliseconds
#define MOMENTARY_SWITCH_LONG_CLICK_DELAY  2000 // In Milliseconds


#define DefaultMaxUpTime 60*30*1000 //default value is 24 hours  
 
class Pump{
  public:

    Pump(uint8_t, uint8_t, uint8_t = NO_TANK, uint8_t = NO_INTERLOCK,  uint8_t = PUMP_STD, uint8_t = PUMP_ON, uint8_t = PUMP_OFF, double = 0., double = 0., double =100.);    
    void loop();
    bool Start();
    bool Stop();
    bool IsRunning();
    bool TankLevel();
    double GetTankUsage();    
    void SetTankVolume(double Volume);
    void SetFlowRate(double FlowRate);
    bool Interlock();
    void SetMaxUpTime(unsigned long Max);
    void ResetUpTime();
    void SetTankFill(double);
    double GetTankFill();

    void ClearErrors();
    
    unsigned long UpTime;
    unsigned long MaxUpTime;
    unsigned long CurrMaxUpTime;
    bool UpTimeError;
    bool PumpVirtualStatus;
    bool MomentarySwitchState;
    unsigned long StartTime;
    unsigned long LastStartTime;
    unsigned long MomentarySwitchStart;
    unsigned long StopTime; 
    double flowrate, tankvolume, tankfill;          
  private:
     
    uint8_t pumppin; 
    uint8_t isrunningsensorpin;
    uint8_t tanklevelpin;
    uint8_t interlockpin;
    uint8_t pumptype;
    uint8_t on_state;
    uint8_t off_state;
};
#endif
