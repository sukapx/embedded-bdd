#pragma once

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/adc.h>


class AnalogIn
{
	const struct device* m_dev_adc;
	const uint8_t m_channel_id;

public:
	AnalogIn(const struct device *dev_adc, const uint8_t channel_id);

	void Init();
	float Read();
};

