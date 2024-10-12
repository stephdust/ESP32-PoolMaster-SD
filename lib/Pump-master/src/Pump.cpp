#include "Arduino.h"
#include "Pump.h"

//Constructor
//PumpPin is the Arduino relay output pin number to be switched to start/stop the pump
//IsRunningSensorPin is the pin which is checked to know whether the pump is running or not. 
//It can be the same pin as "PumpPin" in case there is no sensor on the pump (pressure, current, etc) which is not as robust. 
//This option is especially useful in the case where the filtration pump is not managed by the Arduino. 
//TankLevelPin is the Arduino digital input pin number connected to the tank level switch
//Interlockpin is the Arduino digital input number connected to an "interlock". 
//If this input is Inactive, pump is stopped and/or cannot start. This is used for instance to stop
//the Orp or pH pumps in case filtration pump is not running
//PumpRelayLevel choose whether pump relay is considered active at high level or low level
//InterLockRelayLevel choose whether interlock is considered active at high level or low level
//FlowRate is the flow rate of the pump in Liters/Hour, typically 1.5 or 3.0 L/hour for peristaltic pumps for pools. This is used to compute how much of the tank we have emptied out
//TankVolume is used here to compute the percentage fill used

Pump::Pump(uint8_t PumpPin, uint8_t IsRunningSensorPin, uint8_t TankLevelPin, uint8_t Interlockpin,
           uint8_t PumpRelayLevel, uint8_t InterLockRelayLevel, double FlowRate, double TankVolume, double TankFill) : pumprelay(PumpPin, IsRunningSensorPin, PumpRelayLevel)
{
  tanklevelpin = TankLevelPin;
  interlockpin = Interlockpin;
  flowrate = FlowRate; //in Liters per hour
  tankvolume = TankVolume; //in Liters
  tankfill = TankFill; // in percent

  interlock_on_level = (InterLockRelayLevel == RELAY_ACTIVE_HIGH)? HIGH : LOW;
  interlock_off_level = (InterLockRelayLevel == RELAY_ACTIVE_HIGH)? LOW : HIGH;
  StartTime = 0;
  LastStartTime = 0;
  StopTime = 0;
  UpTime = 0;        
  UpTimeError = 0;
  MaxUpTime = DefaultMaxUpTime;
  CurrMaxUpTime = MaxUpTime;
}

//Constructors with variable arguments list
Pump::Pump(uint8_t PumpPin) 
     :Pump( PumpPin, PumpPin,
            (uint8_t)NO_TANK, (uint8_t)NO_INTERLOCK, 
            (uint8_t)RELAY_ACTIVE_LOW, (uint8_t)RELAY_ACTIVE_LOW) {}

Pump::Pump(uint8_t PumpPin, uint8_t IsRunningSensorPin) 
     :Pump( PumpPin, IsRunningSensorPin,
            (uint8_t)NO_TANK, (uint8_t)NO_INTERLOCK, 
            (uint8_t)RELAY_ACTIVE_LOW, (uint8_t)RELAY_ACTIVE_LOW) {}

Pump::Pump(uint8_t PumpPin, uint8_t IsRunningSensorPin, uint8_t TankLevelPin, 
           Relay* InterLockRelay_ ,  double FlowRate, double TankVolume, double TankFill) 
     :Pump( PumpPin, IsRunningSensorPin, 
            TankLevelPin, (uint8_t)INTERLOCK_REFERENCE,  
            (uint8_t)RELAY_ACTIVE_LOW, (uint8_t)RELAY_ACTIVE_LOW,  
            FlowRate,  TankVolume,  TankFill)
            {
              this->interlockrelay_ = InterLockRelay_;
            }

Pump::Pump(uint8_t PumpPin, uint8_t IsRunningSensorPin, uint8_t TankLevelPin, 
           uint8_t Interlockpin,  double FlowRate, double TankVolume, double TankFill)
     :Pump( PumpPin, IsRunningSensorPin, 
            TankLevelPin, Interlockpin,  
            (uint8_t)RELAY_ACTIVE_LOW, (uint8_t)RELAY_ACTIVE_LOW,  
            FlowRate,  TankVolume,  TankFill) {}

//Call this in the main loop, for every loop, as often as possible
void Pump::loop()
{
  if(IsRunning())
  {
    UpTime += millis() - StartTime;
    StartTime = millis();

    if((CurrMaxUpTime > 0) && (UpTime >= CurrMaxUpTime))
      {
        Stop();
        UpTimeError = true;
      }

      if(!this->Pump::TankLevel())
      {
        Stop();
      } 

      // If Interlock has stopped being active, turn the pump off
      if (!Interlock()) {
        Stop();
      }
    }
}

//Switch pump ON
bool Pump::Start()
{
  if((!IsRunning()) && !UpTimeError && this->Pump::TankLevel() && Interlock())
  {
    if (!pumprelay.Start())
      return false;
    StartTime = LastStartTime = millis(); 
    return true; 
  } else return false;
}

//Switch pump OFF
bool Pump::Stop()
{
  if(IsRunning())
  {
    if (!pumprelay.Stop())
      return false;
    UpTime += millis() - StartTime; 
    return true;
  } else return false;
}

//Set pump relay pin
void Pump::SetRelayPin(uint8_t PumpRelayPin)
{
    pumprelay.SetRelayPin(PumpRelayPin);
}

//Get pump relay pin
uint8_t Pump::GetRelayPin(void)
{
    return pumprelay.GetRelayPin();
}

void Pump::SetRelayType(uint8_t RelayType) // Set Standard or bistable
{
  pumprelay.SetRelayType(RelayType);
}

//Get reference to underlying Relay object
Relay* Pump::GetRelayReference()
{
  return &pumprelay;
}

//Pump status
bool Pump::IsRunning()
{
  return (pumprelay.IsActive());
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

//Set tank fill (percentage of tank volume)
void Pump::SetTankFill(double TankFill)
{
  tankfill = TankFill;
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

//Set flow rate of the pump in Liters/hour
void Pump::SetFlowRate(double FlowRate)
{
  flowrate = FlowRate;
}

//Set a maximum running time (in millisecs) per day (in case ResetUpTime() is called once per day)
//Once reached, pump is stopped and "UpTimeError" error flag is raised
//Set "Max" to 0 to disable limit
void Pump::SetMaxUpTime(unsigned long Max)
{
  MaxUpTime = Max;
  CurrMaxUpTime = MaxUpTime;
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

//Clear "UpTimeError" error flag and allow the pump to run for an extra MaxUpTime
void Pump::ClearErrors()
{
  if(UpTimeError)
  {
    CurrMaxUpTime += MaxUpTime;
    UpTimeError = false;
  }
}

//Set pump relay to be active on HIGH or LOW state of the corresponding pin
void Pump::SetActiveLevel(uint8_t PumpRelayLevel)
{
    pumprelay.SetActiveLevel(PumpRelayLevel);
}

// Get the value 0 or 1 corresponding to the active state of the pump
int Pump::GetActiveLevel()
{
  return (pumprelay.GetActiveLevel());
}

// Get the value 0 or 1 corresponding to the inactive state of the pump
int Pump::GetInactiveLevel()
{
  return (pumprelay.GetInactiveLevel());
}

//Set interlock relay to be measured as active on HIGH or LOW state of the corresponding pin
void Pump::SetInterlockActiveLevel(uint8_t InterLockRelayLevel)
{
  interlock_on_level = (InterLockRelayLevel == RELAY_ACTIVE_HIGH)? HIGH : LOW;
  interlock_off_level = (InterLockRelayLevel == RELAY_ACTIVE_HIGH)? LOW : HIGH;
}

//Interlock status
bool Pump::Interlock()
{
  if(interlockpin == NO_INTERLOCK)
    return true;

  if (interlockpin == INTERLOCK_REFERENCE)
  {
    if(interlockrelay_->IsActive())
      return true;
  } else if (digitalRead(interlockpin) == interlock_on_level)
  {
      return true;
  } 
  return false;
}



