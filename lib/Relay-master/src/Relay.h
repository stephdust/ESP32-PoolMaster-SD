/*
            Relay - a simple library to handle relays
                 (c) Christophe <christophe.belmont@gmail.com> 2024
Features: 

- handle high level and low level active relays
- handle bistable relay operation or standard relay operation
  In case of bistable relays, please note that there is no possibility to know the real underlying state of the relay
  more specifically in case of system restart. This must be dealt with at higher level in your code.

NB: all timings are in milliseconds
*/

#ifndef RELAY_h
#define RELAY_h

#define RELAY_VERSION "1.0.1"

//Class constants
#define RELAY_ACTIVE_HIGH  1
#define RELAY_ACTIVE_LOW  0
#define RELAY_MONOSTABLE  1    // Underlying relay turns ON and OFF equipment by switching between LOW/HIGH states (depending on level parameters)
#define RELAY_BISTABLE  2      // Underlying relay turns ON and OFF equipment by simulating a button press 
                               // switch to HIGH then LOW after a short period of time to switch ON and switch to HIGH then LOW again to switch OFF)
#define RELAY_BISTABLE_SWITCH_SHORT_CLICK_DELAY  500 // In Milliseconds

class Relay{
  public:
    Relay(uint8_t);
    Relay(uint8_t, uint8_t, uint8_t = RELAY_ACTIVE_LOW, uint8_t = RELAY_MONOSTABLE);

    bool Start();
    bool Stop();
    void Toggle();

    int GetActiveLevel();
    int GetInactiveLevel();
    void SetActiveLevel(uint8_t);
    bool SetBistableDelay(int);

    uint8_t GetRelayPin();
    void SetRelayPin(uint8_t);
    void SetRelayType(uint8_t); // Set Standard or bistable
    bool IsActive();

  private:
    TimerHandle_t tmr; // Used for bistable relay callback function timer
    uint8_t relaypin; 
    uint8_t isonsensorpin;
    uint8_t relaytype;  // Standard or bistable
    int bistable_relay_delay;
    int relay_on_level;
    int relay_off_level;
    bool RelayVirtualStatus; // Retain status in case of bistable relay
};
#endif
