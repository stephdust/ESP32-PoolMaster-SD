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
#include "Relay.h"                // To handle underlying pump relays

#ifndef PUMP_h
#define PUMP_h
#define PUMP_VERSION "1.0.2"

//Constants used in some of the functions below
#define TANK_FULL  0
#define TANK_EMPTY 1
#define NO_LEVEL 170          // Pump with tank but without level switch
#define NO_TANK 255           // Pump without tank
#define NO_INTERLOCK 255  
#define INTERLOCK_REFERENCE 170  //Interlock object was passed as a reference Pump&

#define DefaultMaxUpTime 60*30*1000 //default value is 30 minutes

class Pump{
  public:

    Pump(uint8_t);
    Pump(uint8_t, uint8_t);
    Pump(uint8_t, uint8_t, uint8_t, Relay* = NULL,  double = 0., double = 0., double =100.);                         // Constructor to pass Interlock pump object for runtime change in the future
    Pump(uint8_t, uint8_t, uint8_t, uint8_t,  double = 0., double = 0., double =100. );                             // Constructor compatible with version 1.0.1
    Pump(uint8_t, uint8_t, uint8_t, uint8_t,  uint8_t , uint8_t ,  double = 0., double = 0., double =100. );        // Version 1.0.2 constructor

    void loop();
    bool Start();
    bool Stop();    

    void SetRelayPin(uint8_t);
    uint8_t GetRelayPin();
    void SetRelayType(uint8_t); // Set Standard or bistable
    Relay* GetRelayReference();
    
    bool IsRunning();
    
    bool TankLevel();
    void SetTankFill(double);
    double GetTankFill();
    void SetTankVolume(double Volume);
    double GetTankUsage();    
    void SetFlowRate(double FlowRate);
    void SetMaxUpTime(unsigned long Max);
    void ResetUpTime();
    void ClearErrors();

    void SetActiveLevel(uint8_t);
    int GetActiveLevel();
    int GetInactiveLevel();
    void SetInterlockActiveLevel(uint8_t);

    bool Interlock();
       
    unsigned long UpTime;
    unsigned long MaxUpTime;
    unsigned long CurrMaxUpTime;
    bool UpTimeError;
    unsigned long StartTime;
    unsigned long LastStartTime;
    unsigned long StopTime; 
    double flowrate, tankvolume, tankfill;          
  private:
    Relay   pumprelay;
    Relay*  interlockrelay_; // Interlock can be passed 
    uint8_t isrunningsensorpin;
    uint8_t tanklevelpin;
    uint8_t interlockpin;
    int interlock_on_level;
    int interlock_off_level;
};
#endif
