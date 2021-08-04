#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/dac.h>

class AnalogOut
{
const struct device* m_dac_dev;
const struct dac_channel_cfg m_dac_ch_cfg;

public:
	AnalogOut(const struct device* dev_dac, const uint8_t channel_id, const uint8_t resolution);

	void Init();
	void Write(float value);
};


