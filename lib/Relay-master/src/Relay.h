/*
            Relay - a simple library to handle home-pool relays
                 (c) Christophe <christophe.belmont@gmail.com> 2024
Features: 

- keeps track of active and inactive time
- handle high level and low level active relays
- handle momentary relay operation or standard relay operation
- interlock option to prevent relay from switching under condition


NB: all timings are in milliseconds
*/

#include "Arduino_DebugUtils.h"   // Debug.print

#ifndef RELAY_h
#define RELAY_h

#define RELAY_VERSION "1.0.0"

//Constants used in some of the functions below
#define RELAY_ON  1
#define RELAY_OFF 0
#define RELAY_INTERLOCK_OK  1
#define RELAY_INTERLOCK_NOK 0
#define RELAY_NO_INTERLOCK 255
#define RELAY_STD  1            // Underlying relay works normally
#define RELAY_MOMENTARY  2      // Underlying relay activate and deactive shortly after to simulate button press
#define RELAY_MOMENTARY_SWITCH_SHORT_CLICK_DELAY  500 // In Milliseconds
#define RELAY_MOMENTARY_SWITCH_LONG_CLICK_DELAY  2000 // In Milliseconds

#define Relay_DefaultMaxUpTime 60*60*1000*24 //default value is 24 hours  

class Relay{
  public:

    Relay(uint8_t, uint8_t, uint8_t = RELAY_NO_INTERLOCK,  uint8_t = RELAY_STD, uint8_t = RELAY_ON, uint8_t = RELAY_OFF, double = 0., double = 0., double =100.);    
    void loop();
    bool Start();
    bool Stop();
    bool IsRunning();
    bool Interlock();
    void SetMaxUpTime(unsigned long Max);
    void ResetUpTime();

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
       
  private:
     
    uint8_t relaypin; 
    uint8_t isrunningsensorpin;
    uint8_t interlockpin;
    uint8_t relaytype;
    uint8_t on_state;
    uint8_t off_state;
};
#endif
