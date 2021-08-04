class IHeater
{
public:
  virtual void SetEnabled(const bool enabled) = 0;
  virtual void SetCurrentTemperature(const float temp) = 0;
  virtual bool GetHeaterState() const = 0;
  virtual void Process() = 0;
};

class Heater : public IHeater
{
  bool  m_isHeating;
  bool  m_isEnabled;
  float m_temperature;

  public:
    Heater():
      m_isHeating(false),
      m_isEnabled(false),
      m_temperature(0)
    {
    }

    virtual void SetEnabled(const bool enabled) override
    {
      m_isEnabled = enabled;
    }

    virtual void SetCurrentTemperature(const float temp) override
    {
      m_temperature = temp;
    }

    virtual bool GetHeaterState() const override
    {
      return m_isHeating;
    }

    virtual void Process() override
    {
      if(m_isEnabled != 0) {
        if(m_temperature > 0.7F && m_isHeating) {
          m_isHeating = false;
        }else if(m_temperature < 0.3F && !m_isHeating) {
          m_isHeating = true;
        }
      }else{
        m_isHeating = false;
      }
    }
};
