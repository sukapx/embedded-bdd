#pragma once

class MockHardware
{
  bool m_heating;
  float m_temperature;

public:
  MockHardware():
    m_heating(false),
    m_temperature(0.F)
  {}

  void SetHeating(const bool state)
  {
    m_heating = state;
  }

  float GetTemperature() const
  {
    return m_temperature;
  }

  void Process()
  {
    if(m_heating){
      if(m_temperature < 0.950F)
        m_temperature += 0.001F;
      else m_temperature = 0.950F;
    }else{
      if(m_temperature > 0.050F)
        m_temperature -= 0.001F;
      else m_temperature = 0.050F;
    }
  }
};
