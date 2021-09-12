#include "LogicIO.h"
#include <logging/log.h>

LOG_MODULE_REGISTER(LogicIO, LOG_LEVEL_INF);


Button::Button(const struct gpio_dt_spec button, const gpio_flags_t extra_flags):
  m_button(button),
  m_extra_flags(extra_flags)
{}
  
void Button::Init() {
  if (!device_is_ready(m_button.port)) {
    LOG_ERR("Error: button device %s is not ready\n",
          m_button.port->name);
    return;
  }
  int ret = gpio_pin_configure_dt(&m_button, m_extra_flags);
  if (ret != 0) {
    LOG_ERR("Error %d: failed to configure %s pin %d\n",
          ret, m_button.port->name, m_button.pin);
    return;
  }
}

uint32_t Button::Read() const
{
  if (!device_is_ready(m_button.port)) return 0;
  return gpio_pin_get_dt(&m_button);
}

void Button::Write(uint32_t state) {
  if (device_is_ready(m_button.port))
    gpio_pin_set(m_button.port, m_button.pin, state);
}
