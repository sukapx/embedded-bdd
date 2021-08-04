#include "Board.h"
#include <string.h>

#define LED0_NODE DT_NODELABEL(led_0)
#define LED1_NODE DT_NODELABEL(led_1)
#define LED2_NODE DT_NODELABEL(led_2)
#define LED3_NODE DT_NODELABEL(led_3)
#define IN0_NODE	DT_NODELABEL(logic_in_0)
#define IN1_NODE	DT_NODELABEL(logic_in_1)
#define UART_NODE DT_LABEL(DT_ALIAS(intermdule))
#define ADC_NODE		DT_PHANDLE(DT_PATH(zephyr_user), io_channels)

#define ADC_NUM_CHANNELS	DT_PROP_LEN(DT_PATH(zephyr_user), io_channels)

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)
#if (DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac)  && \
	DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac_channel_id) && \
	DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac_resolution))
#define DAC_NODE DT_PHANDLE(ZEPHYR_USER_NODE, dac)
#else
#error "Unsupported board: see README and check /zephyr,user node"
#define DAC_NODE DT_INVALID_NODE
#define DAC_CHANNEL_ID 0
#define DAC_RESOLUTION 0
#endif


Board BOARD;



void Board::Init() {
  init_io_logic();
  init_io_adc();
  init_io_dac();
}

void Board::init_io_logic() {
  in0 = new Button(GPIO_DT_SPEC_GET(IN0_NODE, gpios));
  in1 = new Button(GPIO_DT_SPEC_GET(IN1_NODE, gpios));

  led0 = new Button(GPIO_DT_SPEC_GET(LED0_NODE, gpios), GPIO_OUTPUT_INACTIVE);
  led1 = new Button(GPIO_DT_SPEC_GET(LED1_NODE, gpios), GPIO_OUTPUT_INACTIVE);
  led2 = new Button(GPIO_DT_SPEC_GET(LED2_NODE, gpios), GPIO_OUTPUT_INACTIVE);
  led3 = new Button(GPIO_DT_SPEC_GET(LED3_NODE, gpios), GPIO_OUTPUT_INACTIVE);

  in0->Init();
  in1->Init();

  led0->Init();
  led1->Init();
  led2->Init();
  led3->Init();
}

void Board::init_io_adc() {
  aIn0 = new AnalogIn(DEVICE_DT_GET(ADC_NODE), DT_IO_CHANNELS_INPUT_BY_IDX(DT_PATH(zephyr_user), 0));
  aIn1 = new AnalogIn(DEVICE_DT_GET(ADC_NODE), DT_IO_CHANNELS_INPUT_BY_IDX(DT_PATH(zephyr_user), 1));

  aIn0->Init();
  aIn1->Init();
}

void Board::init_io_dac() {
  static const struct device *dac_dev = DEVICE_DT_GET(DAC_NODE);
  aOut0 = new AnalogOut(dac_dev, DT_PROP(ZEPHYR_USER_NODE, dac_channel_id), DT_PROP(ZEPHYR_USER_NODE, dac_resolution));

  aOut0->Init();
}


Button* Board::GetLogicOut(const char* name) {
  Button* output = NULL;
  if(strcmp("led0", name) == 0) {
    output = led0;
  }else if(strcmp("led1", name) == 0) {
    output = led1;
  }else if(strcmp("led2", name) == 0) {
    output = led2;
  }else if(strcmp("led3", name) == 0) {
    output = led3;
  }
  return output;
}


Button* Board::GetLogicIn(const char* name) {
  Button* input = NULL;
  if(strcmp("in0", name) == 0) {
    input = in0;
  }else if(strcmp("in1", name) == 0) {
    input = in1;
  }
  return input;
}

AnalogOut* Board::GetAnalogOut(const char* name) {
  AnalogOut* aOut = NULL;
  if(strcmp("aOut0", name) == 0) {
    aOut = aOut0;
  }
  return aOut;
}

AnalogIn* Board::GetAnalogIn(const char* name) {
  AnalogIn* aIn = NULL;
  if(strcmp("aIn0", name) == 0) {
    aIn = aIn0;
  }else if(strcmp("aIn1", name) == 0) {
    aIn = aIn1;
  }
  return aIn;
}

