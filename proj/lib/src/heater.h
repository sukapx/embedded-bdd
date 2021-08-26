#pragma once

class IHeater
{
public:
  virtual void SetEnabled(const bool enabled) = 0;
  virtual bool GetEnabled() const = 0;
  virtual void SetCurrentTemperature(const float temp) = 0;
  virtual bool GetHeaterState() const = 0;
  virtual void Process() = 0;
  virtual void SetConfigTemperatureMin(const float temp) = 0;
  virtual void SetConfigTemperatureMax(const float temp) = 0;
};

class Heater : public IHeater
{
  bool  m_isHeating;
  bool  m_isEnabled;
  float m_temperature;
  float m_temperature_min;
  float m_temperature_max;

  public:
    Heater():
      m_isHeating(false),
      m_isEnabled(false),
      m_temperature(0),
      m_temperature_min(0.75F),
      m_temperature_max(0.25F)
    {
    }

    virtual void SetEnabled(const bool enabled) override
    {
      m_isEnabled = enabled;
    }

    virtual bool GetEnabled() const override
    {
      return m_isEnabled;
    }

    virtual void SetCurrentTemperature(const float temp) override
    {
      m_temperature = temp;
    }

    virtual void SetConfigTemperatureMin(const float temp) override
    {
      m_temperature_min = temp;
    }

    virtual void SetConfigTemperatureMax(const float temp) override
    {
      m_temperature_max = temp;
    }

    virtual bool GetHeaterState() const override
    {
      return m_isHeating;
    }

    virtual void Process() override
    {
      if(m_isEnabled != 0) {
        if(m_temperature > m_temperature_max && m_isHeating) {
          m_isHeating = false;
        }else if(m_temperature < m_temperature_min && !m_isHeating) {
          m_isHeating = true;
        }
      }else{
        m_isHeating = false;
      }
    }
};
