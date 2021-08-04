#pragma once


struct DeviceState {
	enum State {
		TESTFRAMEWORK,  // Bare testing
		DEMO,						// toggle GPIOs do Analog read / write
		CONTROLLOOP,		// Active controlloop
		HW_MOCK					// Mock Hardware for Tests
	};
};

extern volatile long blinkInterval;
extern volatile DeviceState::State device_state;

