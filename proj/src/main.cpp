#include "main.h"

#include <stdlib.h>
#include <stdio.h>

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <sys/printk.h>
#include <console/console.h>

#include <logging/log.h>
#include <shell/shell.h>

//LOG_MODULE_DECLARE(main);
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define LED0_NODE DT_NODELABEL(led_0)
#define LED1_NODE DT_NODELABEL(led_1)
#define LED2_NODE DT_NODELABEL(led_2)
#define LED3_NODE DT_NODELABEL(led_3)
#define IN0_NODE	DT_NODELABEL(logic_in_0)
#define IN1_NODE	DT_NODELABEL(logic_in_1)
#define UART_NODE DT_LABEL(DT_ALIAS(intermdule))
#define ADC_NODE		DT_PHANDLE(DT_PATH(zephyr_user), io_channels)

long blinkInterval = 100;
volatile bool enableRegulation = false;

class Button {
	const struct gpio_dt_spec m_button;
	const gpio_flags_t m_extra_flags;

public:
	Button(const struct gpio_dt_spec button, const gpio_flags_t extra_flags = GPIO_INPUT):
		m_button(button),
		m_extra_flags(extra_flags)
		{}
		
		void Init() {

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

	uint32_t GetState() {
		if (!device_is_ready(m_button.port)) return 0;
		return gpio_pin_get_dt(&m_button);
	}

	void SetState(uint32_t state) {
		if (device_is_ready(m_button.port))
			gpio_pin_set(m_button.port, m_button.pin, state);
	}
};

Button* in0;
Button* in1;
Button* led0;
Button* led1;
Button* led2;
Button* led3;











static int cmd_io_set(const struct shell *shell, size_t argc, char **argv)
{
	if(argc < 3) {
		shell_print(shell, "Usage: io set <io> <state>");
		return -1;
	}

	Button* output = NULL;
	if(strcmp("led0", argv[1]) == 0) {
		output = led0;
	}else if(strcmp("led1", argv[1]) == 0) {
		output = led1;
	}else if(strcmp("led2", argv[1]) == 0) {
		output = led2;
	}else if(strcmp("led3", argv[1]) == 0) {
		output = led3;
	}else{
		shell_print(shell, "Unknown Output: '%s'", argv[1]);
	}

	if(output != NULL) {
		auto value = atoi(argv[2]);
		output->SetState(value);
	}

	return 0;
}

static int cmd_io_get(const struct shell *shell, size_t argc, char **argv)
{
	if(argc < 2) {
		shell_print(shell, "Usage: io get <io>");
		return -1;
	}

	Button* input = NULL;
	if(strcmp("in0", argv[1]) == 0) {
		input = in0;
	}else if(strcmp("in1", argv[1]) == 0) {
		input = in1;
	}else{
		shell_print(shell, "Unknown Input: '%s'", argv[1]);
	}

	if(input != NULL) {
		shell_print(shell, "%d", input->GetState());
	}

	return 0;
}

static int cmd_io_regulate(const struct shell *shell, size_t argc, char **argv)
{
	if(argc < 1) {
		shell_print(shell, "Usage: regulate 0|1");
		return -1;
	}

	enableRegulation = (argv[1][0] =='1');
	shell_print(shell, "Regulation is %d", enableRegulation);
	

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_io,
	SHELL_CMD(set, NULL, "Set", cmd_io_set),
	SHELL_CMD(get, NULL, "Get", cmd_io_get),
	SHELL_CMD(regulate, NULL, "Regulation 1|0", cmd_io_regulate),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);
SHELL_CMD_REGISTER(io, &sub_io, "IO commands", NULL);






K_TIMER_DEFINE(timer_blink, NULL, NULL);
void blink0(void)
{
	LOG_INF("[blink0] Run");
	for (size_t loopIter = 0;;loopIter++) {
		k_timer_status_sync(&timer_blink);
		LOG_DBG("[blink0] %d", loopIter);
		if(enableRegulation){
			LOG_INF("[blink0] States: %d, %d", in0->GetState(), in1->GetState());
			led2->SetState(in0->GetState());
			led3->SetState(in1->GetState());
		}
	}
}




/*
struct FrameData {
	struct Buffer {
		uint8_t buffer[10];
		size_t idx;
	} in, frame;
	int64_t lastDataTime;
	int64_t frameDuration;
};

K_SEM_DEFINE(dataPackageSemaphore, 0, 1);
static void uart_fifo_callback(const struct device *dev, void *user_data)
{
	struct FrameData* buffer = (struct FrameData*)user_data;

	if (!uart_irq_update(dev)) {
		LOG_WRN("UART IRQ Fail");
		return;
	}

	if (uart_irq_tx_ready(dev)) {
		uart_irq_tx_disable(dev);
	}

	if (uart_irq_rx_ready(dev)) {
		uint8_t recvData;
		uart_fifo_read(dev, &recvData, 1);
		int64_t now = k_uptime_get();
		if((buffer->lastDataTime + buffer->frameDuration) < now){
			buffer->in.idx = 0;
			for(size_t idx = 0; idx < sizeof(buffer->in.buffer)-1; idx++) {
				buffer->in.buffer[idx] = 0;
			}
		}
		

		if(buffer->in.idx == 0) 
			buffer->lastDataTime = now;

		if(buffer->in.idx < sizeof(buffer->in.buffer)-1) {
			buffer->in.buffer[buffer->in.idx++] = recvData;
		} else {
			for(size_t idx = 0; idx < sizeof(buffer->in.buffer); idx++) {
				if(idx < buffer->in.idx)
					buffer->frame.buffer[idx] = buffer->in.buffer[idx];
				else
					buffer->frame.buffer[idx] = 0;
			}
			buffer->frame.idx = buffer->in.idx;
			buffer->in.idx = 0;
			k_sem_give(&dataPackageSemaphore);
		}
	}
}

struct FrameData frameData;
void dataParser(void)
{
	LOG_INF("[dataParser] Init");
	const struct device *uart_dev = device_get_binding(UART_NODE);
	if(!device_is_ready(uart_dev)) {
		LOG_ERR("Can't setup " UART_NODE "");
		return;
	}

	frameData.frameDuration = 10;
	uart_irq_callback_user_data_set(uart_dev, uart_fifo_callback, &frameData);
	uart_irq_rx_enable(uart_dev);

	LOG_INF("[dataParser] Run");
	for (size_t loopIter = 0;;loopIter++) {
		if (k_sem_take(&dataPackageSemaphore, K_MSEC(10000)) != 0) {
			LOG_INF("[dataParser] Timeout");
			LOG_HEXDUMP_DBG(frameData.in.buffer, sizeof(frameData.in.buffer), "Buffer");
    } else {
			printk("[dataParser] '%s'\n", frameData.frame.buffer);
			LOG_HEXDUMP_DBG(frameData.frame.buffer, sizeof(frameData.frame.buffer), "Buffer");
		}
	}
}
*/


#include <drivers/adc.h>
#define ADC_NUM_CHANNELS	DT_PROP_LEN(DT_PATH(zephyr_user), io_channels)

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif
static uint8_t channel_ids[ADC_NUM_CHANNELS] = {
	DT_IO_CHANNELS_INPUT_BY_IDX(DT_PATH(zephyr_user), 0),
	DT_IO_CHANNELS_INPUT_BY_IDX(DT_PATH(zephyr_user), 1)
};

static int16_t sample_buffer[1];

struct adc_channel_cfg channel_cfg = {
	.gain = ADC_GAIN_1,
	.reference = ADC_REF_INTERNAL,
	.acquisition_time = ADC_ACQ_TIME_DEFAULT,
	.channel_id = 0,
	.differential = 0
};

struct adc_sequence sequence = {
	.channels    = 0,
	.buffer      = sample_buffer,
	.buffer_size = sizeof(sample_buffer),
	.resolution  = 12,
};



#include <drivers/dac.h>


#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)
#if (DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac)  && \
	DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac_channel_id) && \
	DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac_resolution))
#define DAC_NODE DT_PHANDLE(ZEPHYR_USER_NODE, dac)
#define DAC_CHANNEL_ID DT_PROP(ZEPHYR_USER_NODE, dac_channel_id)
#define DAC_RESOLUTION DT_PROP(ZEPHYR_USER_NODE, dac_resolution)
#else
#error "Unsupported board: see README and check /zephyr,user node"
#define DAC_NODE DT_INVALID_NODE
#define DAC_CHANNEL_ID 0
#define DAC_RESOLUTION 0
#endif

static const struct dac_channel_cfg dac_ch_cfg = {
	.channel_id  = DAC_CHANNEL_ID,
	.resolution  = DAC_RESOLUTION
};





void init_io_logic() {
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

const struct device *dev_adc = DEVICE_DT_GET(ADC_NODE);
void init_io_adc() {
	if (!device_is_ready(dev_adc)) {
		LOG_ERR("ADC device not found\n");
		return;
	}

	for (uint8_t i = 0; i < ADC_NUM_CHANNELS; i++) {
		channel_cfg.channel_id = channel_ids[i];

		LOG_INF("ADC setup %d to %d\n", i, channel_cfg.channel_id);
		adc_channel_setup(dev_adc, &channel_cfg);
	}
}

static const struct device *dac_dev = DEVICE_DT_GET(DAC_NODE);
void init_io_dac() {
	if (!device_is_ready(dac_dev)) {
		LOG_ERR("DAC device %s is not ready\n", dac_dev->name);
		return;
	}
	int ret = dac_channel_setup(dac_dev, &dac_ch_cfg);
	if (ret != 0) {
		LOG_ERR("Setting up of DAC channel failed with code %d\n", ret);
		return;
	}
}


void main(void)
{
	int ret = 0;
	LOG_INF("[BOARD]: %s", CONFIG_BOARD);

	init_io_logic();
	init_io_adc();
	init_io_dac();

	k_timer_start(&timer_blink, K_MSEC(500), K_MSEC(500));

	int32_t adc_vref = adc_ref_internal(dev_adc);
	LOG_INF("[main] Run");
	for (size_t loopIter = 0;;loopIter++) {
		LOG_DBG("loopIter %d", loopIter);

		uint32_t dac_value = (loopIter<<2)%(1U<<DAC_RESOLUTION);
		ret = dac_write_value(dac_dev, 1, dac_value);
		if (ret != 0) {
			LOG_ERR("dac_write_value() failed with code %d\n", ret);
			return;
		}


		if(loopIter%20 == 0) {
			LOG_INF("DAC: %d", dac_value);
			for(size_t adcIdx = 0; adcIdx < ADC_NUM_CHANNELS; adcIdx++){
				sequence.channels = BIT(channel_ids[adcIdx]);
				ret = adc_read(dev_adc, &sequence);
				if (ret != 0) {
					LOG_ERR("ADC reading failed with error %d.\n", ret);
					return;
				}
				LOG_INF("ADC%d: %d   Ref: %d", adcIdx, sample_buffer[0], adc_vref);
			}
		}
		k_msleep(blinkInterval);
	}
}

#define STACKSIZE 1024
#define PRIORITY 7

K_THREAD_DEFINE(blink0_id, STACKSIZE, blink0, NULL, NULL, NULL,
		PRIORITY, 0, 0);
//K_THREAD_DEFINE(dataParser_id, STACKSIZE, dataParser, NULL, NULL, NULL,
//		PRIORITY, 0, 0);
