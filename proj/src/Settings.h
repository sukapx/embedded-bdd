#pragma once

#include <stdint.h>


class Settings
{
public:
  enum Value {
    CALLBACK,
    TEMPERATURE_MIN,
    TEMPERATURE_MAX,
    CONTROL_LOG_ENABLE,
    MODCOM_LOG_ENABLE,
    SIZE
  };
  volatile int32_t configValues[Value::SIZE];

  Settings()
  {
    Set(Value::TEMPERATURE_MIN, 250);
    Set(Value::TEMPERATURE_MAX, 750);
    Set(Value::CONTROL_LOG_ENABLE, 0);
    Set(Value::MODCOM_LOG_ENABLE, 0);
  }

  int32_t Get(const Value& config) const
  {
    return configValues[static_cast<const uint32_t>(config)];
  }

  void Set(const Value& config, const int32_t value)
  {
    configValues[static_cast<const uint32_t>(config)] = value;
  }

  static uint32_t Size()
  {
    return Value::SIZE;
  }
};

extern Settings SETTINGS;
