#include "Arduino.h"
#include "Relay.h"

//Constructor
//relaypin is the Arduino relay output pin number to be switched to start/stop the equipment
//TankLevelPin is the Arduino digital input pin number connected to the tank level switch
//Interlockpin is the Arduino digital input number connected to an "interlock". 
//If this input is LOW, pump is stopped and/or cannot start. This is used for instance to stop
//the Orp or pH pumps in case filtration pump is not running
//PumpRelayType, InterLockRelayType choose whether pump relay and interlock is considered active at high level or low level
// RelayType can be normal or momentary when used to simulate a button pressed when closing relay for a short time and reopen it
//IsRunningSensorPin is the pin which is checked to know whether the pump is running or not. 
//It can be the same pin as "relaypin" in case there is no sensor on the pump (pressure, current, etc) which is not as robust. 
//This option is especially useful in the case where the filtration pump is not managed by the Arduino. 
//FlowRate is the flow rate of the pump in Liters/Hour, typically 1.5 or 3.0 L/hour for peristaltic pumps for pools. This is used to compute how much of the tank we have emptied out
//TankVolume is used here to compute the percentage fill used
//PumpType defines whether the underlying relay should work normally or momentarilly simulating a button press
Relay::Relay(uint8_t RelayPin, uint8_t IsRunningSensorPin,  
           uint8_t Interlockpin,  uint8_t RelayType, uint8_t PumpRelayType, uint8_t InterLockRelayType)
{
  relaypin = RelayPin;
  isrunningsensorpin = IsRunningSensorPin;
  interlockpin = Interlockpin;
  relaytype = RelayType; // Standard (RELAY_STD) or Momentary (RELAY_MOMENTARY)
  relay_on_state = (PumpRelayType == RELAY_ACTIVE_HIGH)? STATE_ON : STATE_OFF;
  relay_off_state = (PumpRelayType == RELAY_ACTIVE_HIGH)? STATE_OFF : STATE_ON;
  interlock_on_state = (InterLockRelayType == RELAY_ACTIVE_HIGH)? STATE_ON : STATE_OFF;
  interlock_off_state = (InterLockRelayType == RELAY_ACTIVE_HIGH)? STATE_OFF : STATE_ON;
  StartTime = 0;
  LastStartTime = 0;
  StopTime = 0;
  UpTime = 0;        
  UpTimeError = 0;
  MomentarySwitchStart = 0; // Contain the millis when the underlying relay was set to ON
  RelayVirtualStatus = 0;
  MomentarySwitchState = 0; // Is True when the underlying relay of the momentary switch is ON
  MaxUpTime = Relay_DefaultMaxUpTime;
  CurrMaxUpTime = MaxUpTime;

  // Open the port for OUTPUT and set it to down state
  pinMode(relaypin, OUTPUT);
  digitalWrite(relaypin,relay_off_state);
}

//Call this in the main loop, for every loop, as often as possible
void Relay::loop()
{
  if(digitalRead(isrunningsensorpin) == relay_on_state || ((relaytype == RELAY_MOMENTARY) && (RelayVirtualStatus == 1)))
  {
    UpTime += millis() - StartTime;
    StartTime = millis();
  }

  // Reset the underlying momentary switch relay  OFF
  if ((relaytype == RELAY_MOMENTARY) && (MomentarySwitchState == 1) && ((millis() - MomentarySwitchStart) >= RELAY_MOMENTARY_SWITCH_SHORT_CLICK_DELAY))
  {
    digitalWrite(relaypin, relay_off_state);  // Stop momentary switch after RELAY_MOMENTARY_SWITCH_SHORT_CLICK_DELAY ms
    MomentarySwitchState = 0;
  }

  if((CurrMaxUpTime > 0) && (UpTime >= CurrMaxUpTime))
  {
    Stop();
    UpTimeError = true;
  }

  if(interlockpin != RELAY_NO_INTERLOCK)
  {
    if(digitalRead(interlockpin) == interlock_off_state)
       Stop();
  }
}

//Switch pump ON if over time was not reached, tank is not empty and interlock is OK
bool Relay::Start()
{
  if (relaytype == RELAY_MOMENTARY) // If Pump is Momentary
  {
    Debug.print(DBG_VERBOSE,"Start : Relay Momentary");
    if((RelayVirtualStatus == 0) // Check that the Status is not already ON
      && !UpTimeError
      && ((interlockpin == RELAY_NO_INTERLOCK) || (digitalRead(interlockpin) == interlock_on_state)))    // if((digitalRead(relaypin) == false))
    {
        Debug.print(DBG_VERBOSE,"Start : Start Really");
        MomentarySwitchStart = millis();
        digitalWrite(relaypin, relay_on_state);
        StartTime = LastStartTime = millis();
        RelayVirtualStatus = 1;
        MomentarySwitchState = 1;
        return true; 
    }
    else
    {
      return false;
    }
  }
  else 
  {
    if((digitalRead(isrunningsensorpin) == relay_off_state) 
      && !UpTimeError
      && ((interlockpin == RELAY_NO_INTERLOCK) || (digitalRead(interlockpin) == interlock_on_state)))    //if((digitalRead(relaypin) == false))
    {
      digitalWrite(relaypin, relay_on_state);
      StartTime = LastStartTime = millis(); 
      return true; 
    }
    else return false;
  }
}

//Switch pump OFF
bool Relay::Stop()
{
  if (relaytype == RELAY_MOMENTARY)
  {
    if(RelayVirtualStatus == 1)
    {
      MomentarySwitchStart = millis();
      digitalWrite(relaypin, relay_on_state);
      UpTime += millis() - StartTime;
      RelayVirtualStatus = 0;
      MomentarySwitchState = 1;
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {  
    if(digitalRead(isrunningsensorpin) == relay_on_state)
    {
      digitalWrite(relaypin, relay_off_state);
      UpTime += millis() - StartTime; 
      return true;
    }
    else return false;
  }
}

//Reset the tracking of running time
//This is typically called every day at midnight
void Relay::ResetUpTime()
{
  StartTime = 0;
  StopTime = 0;
  UpTime = 0;
  CurrMaxUpTime = MaxUpTime;
}

// Get the value 0 or 1 corresponding to the inactive state of the pump
bool Relay::GetOffLevel()
{
  return (relay_off_state);
}

//Set a maximum running time (in millisecs) per day (in case ResetUpTime() is called once per day)
//Once reached, pump is stopped and "UpTimeError" error flag is raised
//Set "Max" to 0 to disable limit
void Relay::SetMaxUpTime(unsigned long Max)
{
  MaxUpTime = Max;
  CurrMaxUpTime = MaxUpTime;
}

//Clear "UpTimeError" error flag and allow the pump to run for an extra MaxUpTime
void Relay::ClearErrors()
{
  if(UpTimeError)
  {
    CurrMaxUpTime += MaxUpTime;
    UpTimeError = false;
  }
}

//interlock status
bool Relay::Interlock()
{
  return (digitalRead(interlockpin) == interlock_on_state);
}

//pump status
bool Relay::IsRunning()
{
  if (relaytype == RELAY_MOMENTARY)
  {
    return (RelayVirtualStatus == 1);
  }
  else
  {
    return (digitalRead(isrunningsensorpin) == relay_on_state);
  }
}
