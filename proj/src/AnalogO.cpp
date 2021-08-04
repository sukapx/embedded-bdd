#include "AnalogO.h"
#include <logging/log.h>

LOG_MODULE_REGISTER(AnalogO, LOG_LEVEL_INF);

AnalogOut::AnalogOut(const struct device* dev_dac, const uint8_t channel_id, const uint8_t resolution) :
	m_dac_dev(dev_dac),
	m_dac_ch_cfg({channel_id, resolution})
{
}

void AnalogOut::Init() {
	if (!device_is_ready(m_dac_dev)) {
		LOG_ERR("DAC device %s is not ready\n", m_dac_dev->name);
		return;
	}
	int ret = dac_channel_setup(m_dac_dev, &m_dac_ch_cfg);
	if (ret != 0) {
		LOG_ERR("Setting up of DAC channel failed with code %d\n", ret);
		return;
	}
}

void AnalogOut::Write(float value) {
	uint32_t raw = static_cast<uint32_t>(value * (1U<<m_dac_ch_cfg.resolution));
	int ret = dac_write_value(m_dac_dev, m_dac_ch_cfg.channel_id, raw);
	if (ret != 0) {
		LOG_ERR("dac_write_value() failed with code %d\n", ret);
		return;
	}
}
