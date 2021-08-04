#pragma once

#include <stdint.h>


class IOLogic {
public:
	virtual uint32_t Read() const;
	virtual void Write(uint32_t state);
};

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

class Button : public IOLogic
{
	const struct gpio_dt_spec m_button;
	const gpio_flags_t m_extra_flags;

public:
	Button(const struct gpio_dt_spec button, const gpio_flags_t extra_flags = GPIO_INPUT);
		
  void Init();
	virtual uint32_t Read() const;
	virtual void Write(uint32_t state);};
