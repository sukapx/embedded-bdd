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

struct DeviceState {
	enum State {
		TESTFRAMEWORK,  // Bare testing
		DEMO,						// toggle GPIOs do Analog read / write
		CONTROLLOOP,		// Active controlloop
		HW_MOCK					// Mock Hardware for Tests
	};
};

volatile DeviceState::State device_state = DeviceState::DEMO;

class IOLogic {
public:
	virtual uint32_t Read() const;
	virtual void Write(uint32_t state);
};

class Button : public IOLogic
{
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

	virtual uint32_t Read() const
	{
		if (!device_is_ready(m_button.port)) return 0;
		return gpio_pin_get_dt(&m_button);
	}

	virtual void Write(uint32_t state) {
		if (device_is_ready(m_button.port))
			gpio_pin_set(m_button.port, m_button.pin, state);
	}
};

#include <drivers/adc.h>
#define ADC_NUM_CHANNELS	DT_PROP_LEN(DT_PATH(zephyr_user), io_channels)

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif



class AnalogIn
{
	const struct device* m_dev_adc;
	const uint8_t m_channel_id;

public:
	AnalogIn(const struct device *dev_adc, const uint8_t channel_id) :
		m_dev_adc(dev_adc),
		m_channel_id(channel_id)
	{
	}

	void Init() {
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

	float Read() {
		int16_t sample_buffer[1];
		struct adc_sequence sequence= {
			.channels    = BIT(m_channel_id),
			.buffer      = sample_buffer,
			.buffer_size = sizeof(sample_buffer),
			.resolution  = 12,
		};
		int ret = adc_read(m_dev_adc, &sequence);
		return static_cast<float>(sample_buffer[0]) / (1U<<sequence.resolution);
	}
};




#include <drivers/dac.h>


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

class AnalogOut
{
const struct device* m_dac_dev;
const struct dac_channel_cfg m_dac_ch_cfg;

public:
	AnalogOut(const struct device* dev_dac, const uint8_t channel_id, const uint8_t resolution) :
		m_dac_dev(dev_dac),
		m_dac_ch_cfg({channel_id, resolution})
	{
	}

	void Init() {
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

	void Write(float value) {
		uint32_t raw = static_cast<uint32_t>(value * (1U<<m_dac_ch_cfg.resolution));
		int ret = dac_write_value(m_dac_dev, m_dac_ch_cfg.channel_id, raw);
		if (ret != 0) {
			LOG_ERR("dac_write_value() failed with code %d\n", ret);
			return;
		}
	}
};





/*
Pinning:

| Control | Testboard |
| aIn1		| aOut0     |
| aOut0		| aIn1      |
| led2		| in0       |
| led3		| in1       |
| in0			| led2      |
| in1			| led3      |

*/


class Board {
public:
	Button* in0;
	Button* in1;
	Button* led0;
	Button* led1;
	Button* led2;
	Button* led3;
	AnalogIn* aIn0;
	AnalogIn* aIn1;
	AnalogOut* aOut0;

	void Init() {
		init_io_logic();
		init_io_adc();
		init_io_dac();
	}

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

	void init_io_adc() {
		aIn0 = new AnalogIn(DEVICE_DT_GET(ADC_NODE), DT_IO_CHANNELS_INPUT_BY_IDX(DT_PATH(zephyr_user), 0));
		aIn1 = new AnalogIn(DEVICE_DT_GET(ADC_NODE), DT_IO_CHANNELS_INPUT_BY_IDX(DT_PATH(zephyr_user), 1));

		aIn0->Init();
		aIn1->Init();
	}

	void init_io_dac() {
		static const struct device *dac_dev = DEVICE_DT_GET(DAC_NODE);
		aOut0 = new AnalogOut(dac_dev, DT_PROP(ZEPHYR_USER_NODE, dac_channel_id), DT_PROP(ZEPHYR_USER_NODE, dac_resolution));

		aOut0->Init();
	}


	Button* GetLoginOut(const char* name) {
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

	
	Button* GetLoginIn(const char* name) {
		Button* input = NULL;
		if(strcmp("in0", name) == 0) {
			input = in0;
		}else if(strcmp("in1", name) == 0) {
			input = in1;
		}
		return input;
	}

	AnalogOut* GetAnalogOut(const char* name) {
		AnalogOut* aOut = NULL;
		if(strcmp("aOut0", name) == 0) {
			aOut = aOut0;
		}
		return aOut;
	}
	
	AnalogIn* GetAnalogIn(const char* name) {
		AnalogIn* aIn = NULL;
		if(strcmp("aIn0", name) == 0) {
			aIn = aIn0;
		}else if(strcmp("aIn1", name) == 0) {
			aIn = aIn1;
		}
		return aIn;
	}
};

Board BOARD;





static int cmd_io_set(const struct shell *shell, size_t argc, char **argv)
{
	if(argc < 3) {
		shell_print(shell, "Usage: io set <io> <state>");
		return -1;
	}

	Button* output = BOARD.GetLoginOut(argv[1]);
	if(output != NULL) {
		auto value = atoi(argv[2]);
		output->Write(value);
		return 0;
	}

	AnalogOut* aOut = BOARD.GetAnalogOut(argv[1]);
	if(aOut != NULL) {
		auto value = atoi(argv[2]);
		BOARD.aOut0->Write(value * 0.001f);
		return 0;
	}

	shell_print(shell, "Unknown Output: '%s'", argv[1]);
	return -1;
}

static int cmd_io_get(const struct shell *shell, size_t argc, char **argv)
{
	if(argc < 2) {
		shell_print(shell, "Usage: io get <io>");
		return -1;
	}

	Button* input = BOARD.GetLoginIn(argv[1]);
	if(input != NULL) {
		shell_print(shell, "%d", input->Read());
		return 0;
	}

	AnalogIn* aIn = BOARD.GetAnalogIn(argv[1]);
	if(aIn != NULL) {
		shell_print(shell, "%d", static_cast<int>(aIn->Read()*1000));
		return 0;
	}

	shell_print(shell, "Unknown Input: '%s'", argv[1]);
	return -1;
}

static int cmd_io_devicestate(const struct shell *shell, size_t argc, char **argv,
		    void *data)
{
	if(argc < 1) {
		shell_print(shell, "Usage: devicestate DEMO, CONTROLLOOP, TESTFRAMEWORK");
		return -1;
	}

	device_state = (DeviceState::State)(intptr_t)data;
	shell_print(shell, "Regulation is %d", device_state);

	return 0;
}
SHELL_SUBCMD_DICT_SET_CREATE(sub_io_devicestate, cmd_io_devicestate,
	(DEMO, DeviceState::DEMO),
	(CONTROLLOOP, DeviceState::CONTROLLOOP),
	(TESTFRAMEWORK, DeviceState::TESTFRAMEWORK),
	(HW_MOCK, DeviceState::HW_MOCK)
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_io,
	SHELL_CMD(set, NULL, "Set", cmd_io_set),
	SHELL_CMD(get, NULL, "Get", cmd_io_get),
	SHELL_CMD(devicestate, &sub_io_devicestate, "devicestate DEMO, CONTROLLOOP, TESTFRAMEWORK", cmd_io_devicestate),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);
SHELL_CMD_REGISTER(io, &sub_io, "IO commands", NULL);








/**
 * in0 enables heater
 * aIn1 is seen as Temperature
 */
void controlloop(){
	static bool heating = false;

	if(BOARD.in0->Read() != 0) {
		if(BOARD.aIn1->Read() > 0.7F && heating) {
			heating = false;
		}else if(BOARD.aIn1->Read() < 0.3F && !heating) {
			heating = true;
		}

		BOARD.led2->Write(heating);
	}else{
		BOARD.led2->Write(false);
	}

	printk("[controlloop] Enabled: %d   Heating: %d  Temperature: %d\n", BOARD.in0->Read(), heating, static_cast<int>(BOARD.aIn1->Read()*1000));
}

void hw_mock() {
	static int temperature = 0;
	if(BOARD.in0->Read()){
		if(temperature < 950)
			temperature += 25;
	}else{
		if(temperature > 50)
			temperature -= 25;
	}
	BOARD.aOut0->Write(temperature*0.001F);
	printk("[hw_mock] IsHeating: %d Temperature: %d\n", BOARD.in0->Read(), temperature);
}


K_TIMER_DEFINE(timer_controlloop, NULL, NULL);
void controlloop_task(void)
{
	LOG_INF("[controlloop] Run");
	for (size_t loopIter = 0;;loopIter++) {
		k_timer_status_sync(&timer_controlloop);
		LOG_DBG("[controlloop] %d", loopIter);
		if(device_state == DeviceState::CONTROLLOOP){
			controlloop();
		} else if(device_state == DeviceState::HW_MOCK){
			hw_mock();
		}
	}
}





void main(void)
{
	LOG_INF("[BOARD]: %s", CONFIG_BOARD);

	BOARD.Init();

	k_timer_start(&timer_controlloop, K_MSEC(500), K_MSEC(100));

	LOG_INF("[main] Run");
	for (size_t loopIter = 0;;loopIter++) {
		LOG_DBG("loopIter %d", loopIter);
		if(device_state == DeviceState::DEMO){
			float dac_value = (loopIter%1000)*0.001;
			BOARD.aOut0->Write(dac_value);

			if((loopIter%20) == 0) {
				LOG_INF("DAC: %d", static_cast<int>(dac_value*1000));
				LOG_INF("ADC0: %d", static_cast<int>(BOARD.aIn0->Read()*1000));
				LOG_INF("ADC1: %d", static_cast<int>(BOARD.aIn1->Read()*1000));
			}
		}
		BOARD.led0->Write(!BOARD.led0->Read());
		k_msleep(blinkInterval);
	}
}

#define STACKSIZE 1024
#define PRIORITY 7

K_THREAD_DEFINE(controlloop_id, STACKSIZE, controlloop_task, NULL, NULL, NULL,
		PRIORITY, 0, 0);
//K_THREAD_DEFINE(dataParser_id, STACKSIZE, dataParser, NULL, NULL, NULL,
//		PRIORITY, 0, 0);
