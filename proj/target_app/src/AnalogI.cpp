#include "AnalogI.h"
#include <logging/log.h>

LOG_MODULE_REGISTER(AnalogI, LOG_LEVEL_INF);

AnalogIn::AnalogIn(const struct device *dev_adc, const uint8_t channel_id) :
  m_dev_adc(dev_adc),
  m_channel_id(channel_id)
{
}

void AnalogIn::Init() {
  if (!device_is_ready(m_dev_adc)) {
    LOG_ERR("ADC device not found\n");
    return;
  }
  struct adc_channel_cfg channel_cfg = {
    .gain = ADC_GAIN_1,
    .reference = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id = m_channel_id,
    .differential = 0
  };

  LOG_INF("ADC setup channel %d\n", channel_cfg.channel_id);
  adc_channel_setup(m_dev_adc, &channel_cfg);
}

float AnalogIn::Read() {
  int16_t sample_buffer[1];
  struct adc_sequence sequence= {
    .channels    = BIT(m_channel_id),
    .buffer      = sample_buffer,
    .buffer_size = sizeof(sample_buffer),
    .resolution  = 12,
  };
  static_cast<void>(adc_read(m_dev_adc, &sequence));
  return static_cast<float>(sample_buffer[0]) / (1U<<sequence.resolution);
}
