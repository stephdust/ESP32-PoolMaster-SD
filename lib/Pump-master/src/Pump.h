/*
            Pump - a simple library to handle home-pool filtration and peristaltic pumps
                 (c) Loic74 <loic74650@gmail.com> 2017-2020
Features: 

- keeps track of running time
- keeps track of Tank Levels
- set max running time limit

NB: all timings are in milliseconds
*/

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
//#define INTERLOCK_REFERENCE 170  //Interlock object was passed as a reference Pump&

#define DEFAULTMAXUPTIME 60*30*1000 //default value is 30 minutes

class Pump : public Relay {
  public:
  //Constructor
  //PumpPin is the Arduino relay output pin number to be switched to start/stop the pump
  //TankLevelPin is the Arduino digital input pin number connected to the tank level switch
  //ActiveLevel is either ACTIVE_HIGH or ACTIVE_LOW depending on underlying relay module
  //Operation mode is either MODE_LATCHING or MODE_MOMENTARY
  //   - MODE_LATCHING when PUMP is ON, underlying relay is ON (idem for OFF)
  //   - MODE_MOMENTARY when PUMP turns ON, underlying relay switches ON for a short period on time then turns back OFF (idem for OFF).
  //     this is used to simulate pressing a button and allows control of a wider variety of devices
  //FlowRate is the flow rate of the pump in Liters/Hour, typically 1.5 or 3.0 L/hour for peristaltic pumps for pools. This is used to compute how much of the tank we have emptied out
  //TankVolume is used here to compute the percentage fill used
  //TankFill current tank fill percentage when object is constructed
    Pump(uint8_t _pin_number, uint8_t _pin_id, uint8_t _tank_level_pin = NO_TANK, uint8_t _active_level = ACTIVE_LOW, bool _operation_mode = MODE_LATCHING, double _flowrate= 0., double _tankvolume= 0., double _tankfill=100.) 
    : Relay(_pin_number, _pin_id, OUTPUT_DIGITAL, _active_level, _operation_mode) 
    {
      tank_level_pin = _tank_level_pin;
      flowrate = _flowrate;
      tankvolume = _tankvolume;
      tankfill = _tankfill;
    }

    void loop();

    bool Start();
    bool Stop(); 

    bool IsRunning();
   
    bool TankLevel();
    void SetTankLevelPIN(uint8_t);
    void SetTankFill(double);
    double GetTankFill();
    void SetTankVolume(double Volume);
    double GetTankUsage();
    void SetFlowRate(double FlowRate);
    void SetMaxUpTime(unsigned long Max);
    void ResetUpTime();
    void ClearErrors();

    void SetInterlock(PIN*);
    uint8_t GetInterlockId(void);
    bool CheckInterlock();  // Return true if interlock pump running
    bool IsRelay(void);

    unsigned long UpTime        = 0;
    unsigned long MaxUpTime     = DEFAULTMAXUPTIME; // Uptime for a run
    unsigned long CurrMaxUpTime = DEFAULTMAXUPTIME; // Total uptime after error were cleared an another maxuptime was authorized
    bool          UpTimeError   = false;
    unsigned long StartTime = 0;
    unsigned long StopTime      = 0; 

  private:
    PIN*  interlock_pump_ = nullptr; // Interlock can be passed 
    unsigned long LastLoopMillis     = 0;
    uint8_t tank_level_pin;
    double flowrate, tankvolume, tankfill;
    //uint8_t interlockpin;
};
#endif
