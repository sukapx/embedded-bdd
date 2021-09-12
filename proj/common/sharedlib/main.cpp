#include "main.h"
#include <stdio.h>
#include "heater.h"
#include "Settings.h"

class TemperatureController
{
public:
  Heater heater;
  unsigned long m_iteration;
  bool m_externEnable;
  float m_temperature;

  TemperatureController():
    m_iteration(0),
    m_externEnable(false)
  {
  }

  void DoStep()
  {
    m_iteration++;
    heater.SetEnabled(m_externEnable);
    heater.SetCurrentTemperature(m_temperature);
    heater.SetConfigTemperatureMin(SETTINGS.Get(Settings::TEMPERATURE_MIN) * 0.001F);
    heater.SetConfigTemperatureMax(SETTINGS.Get(Settings::TEMPERATURE_MAX) * 0.001F);

    heater.Process();

    //BOARD.led2->Write(heater.GetHeaterState());
  }

  void DumpInfo()
  {  
    printf("[controlloop] Itr: %d, Enabled: %d, Heating: %d, Temperature: %f\n",
          m_iteration,
          m_externEnable,
          heater.GetHeaterState(),
          m_temperature);
  }
};

TemperatureController tContoller;

EXT_C void api_say_hello()
{
  printf("Lib: Hello!\n");
}

EXT_C void DoStep()
{
  tContoller.DoStep();
  //tContoller.DumpInfo();
}

EXT_C void SetSetting(unsigned int setting, int value)
{
  SETTINGS.Set(static_cast<Settings::Value>(setting), value);
}

EXT_C void SetInput(unsigned int input, float value)
{
  switch(input){
    case 0:
      tContoller.m_temperature = value;
      break;
    case 1:
      tContoller.m_externEnable = value > 0.5;
      break;
  }
}

EXT_C float GetOutput(unsigned int output)
{
  switch(output){
    case 0:
      return tContoller.heater.GetHeaterState();
  }
}
