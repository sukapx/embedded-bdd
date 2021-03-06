#include "main.h"

#include <stdlib.h>
#include <stdio.h>

#include <zephyr.h>
#include <logging/log.h>
#include <sys/printk.h>
#include <shell/shell.h>

#include "Board.h"
#include "ModuleCom.h"
#include "Settings.h"

//LOG_MODULE_DECLARE(main);
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);


volatile long blinkInterval = 1000;
volatile DeviceState::State device_state = DeviceState::DEMO;




#include <heater.h>
Heater heater;
/**
 * in0 enables heater
 * aIn1 is seen as Temperature
 */
void controlloop(){
	static int iteration = 0;

	heater.SetEnabled(BOARD.in0->Read());
	heater.SetCurrentTemperature(BOARD.aIn1->Read());
	heater.SetConfigTemperatureMin(SETTINGS.Get(Settings::TEMPERATURE_MIN) * 0.001F);
	heater.SetConfigTemperatureMax(SETTINGS.Get(Settings::TEMPERATURE_MAX) * 0.001F);

	heater.Process();

	BOARD.led2->Write(heater.GetHeaterState());

	if((iteration++%10) == 0) {
		FuncFrame frame;
		frame.func = 1;
		frame.subFunc = 0;
		frame.i32 = static_cast<int32_t>(BOARD.aIn1->Read() * 1000.F);
		dataSendFrame(frame);

		if(SETTINGS.Get(Settings::CONTROL_LOG_ENABLE))
		{
			printf("[controlloop] Itr: %d, Enabled: %d, Heating: %d, Temperature: %d.%03d\n",
						iteration,
						BOARD.in0->Read(), heater.GetHeaterState(), 
						static_cast<int32_t>(BOARD.aIn1->Read()),
						static_cast<int32_t>(BOARD.aIn1->Read()*1000)%1000);
		}
	}
}

#include <MockHardware.h>
MockHardware mockHardware;

void hw_mock() {
	static int iteration = 0;

	mockHardware.SetHeating(BOARD.in0->Read());
	mockHardware.Process();
	BOARD.aOut0->Write(mockHardware.GetTemperature());

	if((iteration++%10) == 0) {
		if(SETTINGS.Get(Settings::CONTROL_LOG_ENABLE))
		{
			printf("[hw_mock] Itr: %d, Enabled: %d, Heating: %d, Temperature: %d.%03d\n", 
						iteration,
						BOARD.led2->Read(),
						BOARD.in0->Read(),
						static_cast<int32_t>(mockHardware.GetTemperature()),
						static_cast<int32_t>(mockHardware.GetTemperature()*1000)%1000);
		}
	}
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

	k_timer_start(&timer_controlloop, K_MSEC(500), K_MSEC(20));

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
