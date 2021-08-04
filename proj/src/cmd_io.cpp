#include "main.h"
#include "Board.h"


static int cmd_io_set(const struct shell *shell, size_t argc, char **argv)
{
	if(argc < 3) {
		shell_print(shell, "Usage: io set <io> <state>");
		return -1;
	}

	Button* output = BOARD.GetLogicOut(argv[1]);
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

	Button* input = BOARD.GetLogicIn(argv[1]);
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
