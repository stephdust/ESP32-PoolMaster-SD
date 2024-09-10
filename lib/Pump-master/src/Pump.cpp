#include "Arduino.h"
#include "Pump.h"

//Constructor
//PumpPin is the Arduino relay output pin number to be switched to start/stop the pump
//TankLevelPin is the Arduino digital input pin number connected to the tank level switch
//Interlockpin is the Arduino digital input number connected to an "interlock". 
//If this input is LOW, pump is stopped and/or cannot start. This is used for instance to stop
//the Orp or pH pumps in case filtration pump is not running
//IsRunningSensorPin is the pin which is checked to know whether the pump is running or not. 
//It can be the same pin as "PumpPin" in case there is no sensor on the pump (pressure, current, etc) which is not as robust. 
//This option is especially useful in the case where the filtration pump is not managed by the Arduino. 
//FlowRate is the flow rate of the pump in Liters/Hour, typically 1.5 or 3.0 L/hour for peristaltic pumps for pools. This is used to compute how much of the tank we have emptied out
//TankVolume is used here to compute the percentage fill used
//PumpType defines whether the underlying relay should work normally or momentarilly simulating a button press
Pump::Pump(uint8_t PumpPin, uint8_t IsRunningSensorPin, uint8_t TankLevelPin, 
           uint8_t Interlockpin,  uint8_t PumpType, uint8_t High_State, uint8_t Low_State, double FlowRate, double TankVolume, double TankFill)
{
  pumppin = PumpPin;
  isrunningsensorpin = IsRunningSensorPin;
  tanklevelpin = TankLevelPin;
  interlockpin = Interlockpin;
  flowrate = FlowRate; //in Liters per hour
  tankvolume = TankVolume; //in Liters
  tankfill = TankFill; // in percent
  pumptype = PumpType; // Standard or momentary
  on_state = High_State;
  off_state = Low_State;
  StartTime = 0;
  LastStartTime = 0;
  StopTime = 0;
  UpTime = 0;        
  UpTimeError = 0;
  MomentarySwitchStart = 0; // Contain the millis when the underlying relay was set to ON
  PumpVirtualStatus = 0;
  MomentarySwitchState = 0; // Is True when the underlying relay of the momentary switch is ON
  MaxUpTime = DefaultMaxUpTime;
  CurrMaxUpTime = MaxUpTime;
}     

//Call this in the main loop, for every loop, as often as possible
void Pump::loop()
{
  if(digitalRead(isrunningsensorpin) == on_state || ((pumptype == PUMP_MOMENTARY) && (PumpVirtualStatus == 1)))
  {
    UpTime += millis() - StartTime;
    StartTime = millis();
  }

  // Reset the underlying momentary switch relay  OFF
  if ((pumptype == PUMP_MOMENTARY) && (MomentarySwitchState == 1) && ((millis() - MomentarySwitchStart) >= MOMENTARY_SWITCH_SHORT_CLICK_DELAY))
  {
    digitalWrite(pumppin, off_state);
    Debug.print(DBG_INFO,"Stop Momentary switch"); 
    MomentarySwitchState = 0;
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
    if(digitalRead(interlockpin) == INTERLOCK_NOK)
      Stop();
  }
}

//Switch pump ON if over time was not reached, tank is not empty and interlock is OK
bool Pump::Start()
{
  if (pumptype == PUMP_MOMENTARY) // If Pump is Momentary
  {
    if((PumpVirtualStatus == 0) // Check that the Status is not already ON
      && !UpTimeError
      && this->Pump::TankLevel()
      && ((interlockpin == NO_INTERLOCK) || (digitalRead(interlockpin) == INTERLOCK_OK)))    // if((digitalRead(pumppin) == false))
    {
        MomentarySwitchStart = millis();
        digitalWrite(pumppin, on_state);
        StartTime = LastStartTime = millis();
        PumpVirtualStatus = 1;
        MomentarySwitchState = 1;
        Debug.print(DBG_INFO,"Start Momentary switch"); 
        return true; 
    }
    else
    {
      Debug.print(DBG_DEBUG,"Momentary Pump Start called but problem to start (Status %d %d %d %d)",PumpVirtualStatus,UpTimeError,this->Pump::TankLevel(),digitalRead(interlockpin)); 
      return false;
    }
  }
  else 
  {
    if((digitalRead(isrunningsensorpin) == off_state) 
      && !UpTimeError
      && this->Pump::TankLevel()
      && ((interlockpin == NO_INTERLOCK) || (digitalRead(interlockpin) == INTERLOCK_OK)))    //if((digitalRead(pumppin) == false))
    {
      digitalWrite(pumppin, on_state);
      StartTime = LastStartTime = millis(); 
      return true; 
    }
    else return false;
  }
}

//Switch pump OFF
bool Pump::Stop()
{
  if (pumptype == PUMP_MOMENTARY)
  {
    if(PumpVirtualStatus == 1)
    {
      MomentarySwitchStart = millis();
      digitalWrite(pumppin, on_state);
      UpTime += millis() - StartTime;
      PumpVirtualStatus = 0;
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
    if(digitalRead(isrunningsensorpin) == on_state)
    {
      digitalWrite(pumppin, off_state);
      UpTime += millis() - StartTime; 
      return true;
    }
    else return false;
  }
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
  return (digitalRead(interlockpin) == INTERLOCK_OK);
}

//pump status
bool Pump::IsRunning()
{
  if (pumptype == PUMP_MOMENTARY)
  {
    return (PumpVirtualStatus == 1);
  }
  else
  {
    return (digitalRead(isrunningsensorpin) == on_state);
  }
}
