#include "Arduino.h"
#include "Pump.h"

//Constructor
//PumpPin is the Arduino relay output pin number to be switched to start/stop the pump
//TankLevelPin is the Arduino digital input pin number connected to the tank level switch
//Interlockpin is the Arduino digital input number connected to an "interlock". 
//If this input is Inactive, pump is stopped and/or cannot start. This is used for instance to stop
//the Orp or pH pumps in case filtration pump is not running
//PumpRelayType, InterLockRelayType choose whether pump relay and interlock is considered active at high level or low level
//IsRunningSensorPin is the pin which is checked to know whether the pump is running or not. 
//It can be the same pin as "PumpPin" in case there is no sensor on the pump (pressure, current, etc) which is not as robust. 
//This option is especially useful in the case where the filtration pump is not managed by the Arduino. 
//FlowRate is the flow rate of the pump in Liters/Hour, typically 1.5 or 3.0 L/hour for peristaltic pumps for pools. This is used to compute how much of the tank we have emptied out
//TankVolume is used here to compute the percentage fill used
//PumpType defines whether the underlying relay should work normally or momentarilly simulating a button press
Pump::Pump(uint8_t PumpPin, uint8_t IsRunningSensorPin, uint8_t TankLevelPin, 
           uint8_t Interlockpin,  uint8_t PumpRelayType, uint8_t InterLockRelayType, double FlowRate, double TankVolume, double TankFill)
{
  pumppin = PumpPin;
  isrunningsensorpin = IsRunningSensorPin;
  tanklevelpin = TankLevelPin;
  interlockpin = Interlockpin;
  flowrate = FlowRate; //in Liters per hour
  tankvolume = TankVolume; //in Liters
  tankfill = TankFill; // in percent
  relay_on_state = (PumpRelayType == RELAY_ACTIVE_HIGH)? STATE_ON : STATE_OFF;
  relay_off_state = (PumpRelayType == RELAY_ACTIVE_HIGH)? STATE_OFF : STATE_ON;
  interlock_on_state = (InterLockRelayType == RELAY_ACTIVE_HIGH)? STATE_ON : STATE_OFF;
  interlock_off_state = (InterLockRelayType == RELAY_ACTIVE_HIGH)? STATE_OFF : STATE_ON;
  StartTime = 0;
  LastStartTime = 0;
  StopTime = 0;
  UpTime = 0;        
  UpTimeError = 0;
  MaxUpTime = DefaultMaxUpTime;
  CurrMaxUpTime = MaxUpTime;

  // Open the port for OUTPUT and set it to down state
  pinMode(pumppin, OUTPUT);
  digitalWrite(pumppin,relay_off_state);
}

//Call this in the main loop, for every loop, as often as possible
void Pump::loop()
{
  if(digitalRead(isrunningsensorpin) == relay_on_state)
  {
    UpTime += millis() - StartTime;
    StartTime = millis();
  }
  
  if((CurrMaxUpTime > 0) && (UpTime >= CurrMaxUpTime))
  {
    Stop();
    UpTimeError = true;
  }

  if(!this->Pump::TankLevel())
  {
    Stop();
  } 

  if(interlockpin != NO_INTERLOCK)
  {
    if(digitalRead(interlockpin) == interlock_off_state)
      Stop();
  }
}

bool Pump::Start()
{
  if((digitalRead(isrunningsensorpin) == relay_off_state) 
    && !UpTimeError
    && this->Pump::TankLevel()
    && ((interlockpin == NO_INTERLOCK) || (digitalRead(interlockpin) == interlock_on_state)))    //if((digitalRead(pumppin) == false))
  {
    digitalWrite(pumppin, relay_on_state);
    StartTime = LastStartTime = millis(); 
    return true; 
  }
  else return false;
}

//Switch pump OFF
bool Pump::Stop()
{
  if(digitalRead(isrunningsensorpin) == relay_on_state)
  {
    digitalWrite(pumppin, relay_off_state);
    UpTime += millis() - StartTime; 
    return true;
  }
  else return false;
}

//Reset the tracking of running time
//This is typically called every day at midnight
void Pump::ResetUpTime()
{
  StartTime = 0;
  StopTime = 0;
  UpTime = 0;
  CurrMaxUpTime = MaxUpTime;
}

//Set a maximum running time (in millisecs) per day (in case ResetUpTime() is called once per day)
//Once reached, pump is stopped and "UpTimeError" error flag is raised
//Set "Max" to 0 to disable limit
void Pump::SetMaxUpTime(unsigned long Max)
{
  MaxUpTime = Max;
  CurrMaxUpTime = MaxUpTime;
}

// Get the value 0 or 1 corresponding to the inactive state of the pump
bool Pump::GetOffLevel()
{
  return (relay_off_state);
}

// Get the value 0 or 1 corresponding to the active state of the pump
bool Pump::GetOnLevel()
{
  return (relay_on_state);
}


//Clear "UpTimeError" error flag and allow the pump to run for an extra MaxUpTime
void Pump::ClearErrors()
{
  if(UpTimeError)
  {
    CurrMaxUpTime += MaxUpTime;
    UpTimeError = false;
  }
}

//tank level status (true = full, false = empty)
bool Pump::TankLevel()
{
  if(tanklevelpin == NO_TANK)
  {
    return true;
  }
  else if (tanklevelpin == NO_LEVEL)
  {
    return (this->Pump::GetTankFill() > 5.); //alert below 5% 
  }
  else
  {
    return (digitalRead(tanklevelpin) == TANK_FULL);
  } 
}

//Return the percentage used since last reset of UpTime
double Pump::GetTankUsage() 
{
  float PercentageUsed = -1.0;
  if((tankvolume != 0.0) && (flowrate !=0.0))
  {
    double MinutesOfUpTime = (double)UpTime/1000.0/60.0;
    double Consumption = flowrate/60.0*MinutesOfUpTime;
    PercentageUsed = Consumption/tankvolume*100.0;
  }
  return (PercentageUsed);  
}

//Return the remaining quantity in tank in %. When resetting UpTime, SetTankFill must be called accordingly
double Pump::GetTankFill()
{
  return (tankfill - this->Pump::GetTankUsage());
}

//Set Tank volume
//Typically call this function when changing tank and set it to the full volume
void Pump::SetTankVolume(double Volume)
{
  tankvolume = Volume;
}

//Set flow rate of the pump in Liters/hour
void Pump::SetFlowRate(double FlowRate)
{
  flowrate = FlowRate;
}

//Set tank fill (percentage of tank volume)
void Pump::SetTankFill(double TankFill)
{
  tankfill = TankFill;
}

//interlock status
bool Pump::Interlock()
{
  return (digitalRead(interlockpin) == interlock_on_state);
}

//pump status
bool Pump::IsRunning()
{
  return (digitalRead(isrunningsensorpin) == relay_on_state);
}
