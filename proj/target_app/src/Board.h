#pragma once

#include "AnalogIO.h"
#include "LogicIO.h"

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

	void Init();
	void init_io_logic();
	void init_io_adc();
	void init_io_dac();

	Button* GetLogicOut(const char* name);
	Button* GetLogicIn(const char* name);
	AnalogOut* GetAnalogOut(const char* name);
	AnalogIn* GetAnalogIn(const char* name);
};

extern Board BOARD;
