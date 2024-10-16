#include "Arduino.h"
#include "Relay.h"

// Timer Callback function for bistable relay operations
auto CallBackTimer = []( TimerHandle_t xTimer )
{
    Relay* rr = static_cast<Relay*>(pvTimerGetTimerID(xTimer));      // Get the relay related to this timer
    assert(rr); // Sanity check
    digitalWrite(rr->GetRelayPin(), rr->GetInactiveLevel());         // Stop BISTABLE switch when timer fires
};

//Constructor
//relaypin is the Arduino relay output pin number to be switched to start/stop the equipment
//It can be the same pin as "relaypin" in case there is no sensor on the pump (pressure, current, etc) which is not as robust. 
//This option is especially useful in the case where the relay is not managed by the Arduino. 
//RelayType defines whether the underlying relay should work normally on/off or bistable simulating a button press (relay switch to on then off after a predefined time)
//RelayLevels either RELAY_ACTIVE_HIGH if relay is on when pin is high state or RELAY_ACTIVE_LOW if relay is on when pin in low state

Relay::Relay(uint8_t RelayPin, uint8_t RelaySensorPin, uint8_t RelayLevels, uint8_t RelayType)
{
  relaypin = RelayPin;
  isonsensorpin = RelaySensorPin;
  relaytype = RelayType; // Standard (RELAY_MONOSTABLE) or BISTABLE (RELAY_BISTABLE)
  relay_on_level = (RelayLevels == RELAY_ACTIVE_HIGH)? HIGH : LOW;
  relay_off_level = (RelayLevels == RELAY_ACTIVE_HIGH)? LOW : HIGH;
  RelayVirtualStatus = 0;
  bistable_relay_delay = RELAY_BISTABLE_SWITCH_SHORT_CLICK_DELAY;

  // Open the port for OUTPUT and set it to down state
  pinMode(relaypin, OUTPUT);
  digitalWrite(relaypin,relay_off_level);

  // Create timer used by bistable switches to switch off after predefined period of time
  if (relaytype == RELAY_BISTABLE) {
    tmr = xTimerCreate("BistableTimer", pdMS_TO_TICKS(bistable_relay_delay), pdFALSE, static_cast<void*>(this) , CallBackTimer);
    if (tmr == nullptr ) {
          //Debug.print(DBG_ERROR,"Bistable relay timer creation failed (pin %d)",relaypin);
    }
  }
}

//Constructors with only one argument
Relay::Relay(uint8_t RelayPin) 
     :Relay(RelayPin,RelayPin) {}

//Switch the relay ON
bool Relay::Start()
{
  // If timer is active, this means that a bistable switch is currently being turned on or off
  // do nothing and return false to indicate that something went wrong
  if (tmr != nullptr )
    if ( xTimerIsTimerActive( tmr ) != pdFALSE )
      return false;

  if (!IsActive()) 
  {
    if (relaytype == RELAY_BISTABLE) 
    { // If relay is BISTABLE type
      if (tmr == nullptr )  // If timer was not properly initialized
        return false;

        // Launch timer to switch the BISTABLE relay back off after the delay
        if( xTimerStart( tmr, 0 ) != pdPASS )
          return false;

      RelayVirtualStatus = 1;
    }
    digitalWrite(relaypin, relay_on_level);
    return true; 
  } else
    return false;
}

//Switch pump OFF
bool Relay::Stop()
{
  // If timer is active, this means that a bistable switch is currently being turned on or off
  // do nothing and return false to indicate that something went wrong
  if (tmr != nullptr )
    if ( xTimerIsTimerActive( tmr ) != pdFALSE )
      return false;

  if (IsActive()) 
  {
    if (relaytype == RELAY_BISTABLE) 
    { // If relay is BISTABLE type
      if (tmr == nullptr )  // If timer was not properly initialized
        return false;

        // Launch timer to switch the BISTABLE relay back off after the delay
        if( xTimerStart( tmr, 0 ) != pdPASS )
          return false;

      RelayVirtualStatus = 0;
    }
    digitalWrite(relaypin, (relaytype == RELAY_BISTABLE)? relay_on_level : relay_off_level);
    return true; 
  } else
    return false;
}

//Toggle switch state
void Relay::Toggle()
{
  (IsActive()) ? Stop() : Start();  // Invert the position of the relay
}

// Get the value 0 or 1 corresponding to the active level of the relay
int Relay::GetActiveLevel()
{
  return (relay_on_level);
}

// Get the value 0 or 1 corresponding to the inactive level of the relay
int Relay::GetInactiveLevel()
{
  return (relay_off_level);
}

//Set pump relay to be active on HIGH or LOW state of the corresponding pin
void Relay::SetActiveLevel(uint8_t RelayLevel)
{
    relay_on_level = (RelayLevel == RELAY_ACTIVE_HIGH)? HIGH : LOW;
    relay_off_level = (RelayLevel == RELAY_ACTIVE_HIGH)? LOW : HIGH;
}

bool Relay::SetBistableDelay(int Bistable_Relay_Delay)
{
  bool isTimerActive = false;
  bistable_relay_delay = Bistable_Relay_Delay;

  // Initialize timer to new value
  if(relaytype == RELAY_BISTABLE)
  {
    // Check if timer is currently active to futher reactivate
    isTimerActive = ( xTimerIsTimerActive( tmr ) != pdFALSE )? true : false;

    if (xTimerChangePeriod( tmr, pdMS_TO_TICKS(bistable_relay_delay), 0 ) != pdPASS )
      return false;
    
    // xTimerChangePeriod causes the timer to restart. If it was dormant before stop it.
    if (!isTimerActive)
      xTimerStop( tmr, 0 ); 
    return true;
  } else
    return true;
}

// Return pin number related to this relay
uint8_t Relay::GetRelayPin()
{
  return relaypin;
}

// Set the pin number for this relay
void Relay::SetRelayPin(uint8_t RelayPin)
{
  relaypin = RelayPin;
  // Open the port for OUTPUT and set it to down state
  pinMode(relaypin, OUTPUT);
  digitalWrite(relaypin,relay_off_level);
}

// Change relay type STANDARD or BISTABLE
void Relay::SetRelayType(uint8_t RelayType)
{
  // Change from standard to bistable
  if ((relaytype == RELAY_MONOSTABLE) && (RelayType == RELAY_BISTABLE))  
  {
    // Create timer used by bistable switches to switch off after predefined period of time
    tmr = xTimerCreate("BistableTimer", pdMS_TO_TICKS(RELAY_BISTABLE_SWITCH_SHORT_CLICK_DELAY), pdFALSE, static_cast<void*>(this) , CallBackTimer);
    if (tmr == nullptr ) {
          //Debug.print(DBG_ERROR,"Bistable relay timer creation failed (pin %d)",relaypin);
    }
  }

  // Change from bistable to standard
  if ((relaytype == RELAY_BISTABLE) && (RelayType == RELAY_MONOSTABLE))  // Move from standard to bistable
  {
    // No need for a timer anymore
    xTimerDelete(tmr, 0);
  }
  relaytype = RelayType;
}

//relay status
bool Relay::IsActive()
{
  if (relaytype == RELAY_BISTABLE)
  {
    return (RelayVirtualStatus == 1);
  }
  else
  {
    return (digitalRead(isonsensorpin) == relay_on_level);
  }
}
