#include "main.h"
#include <shell/shell.h>

static int cmd_util_hexdump(const struct shell *shell, size_t argc, char **argv)
{
	shell_print(shell, "argc = %d", argc);
	for (size_t cnt = 0; cnt < argc; cnt++) {
		shell_print(shell, "argv[%d]", cnt);
		shell_hexdump(shell, (uint8_t*)argv[cnt], strlen(argv[cnt]));
	}

	return 0;
}

static int cmd_util_interval(const struct shell *shell, size_t argc, char **argv,
		    void *data)
{
	blinkInterval = (intptr_t)data;

	return 0;
}


static int cmd_util_echo(const struct shell *shell, size_t argc, char **argv)
{
	shell_print(shell, "Got: %s", argv[1]);

	return 0;
}

SHELL_SUBCMD_DICT_SET_CREATE(sub_dict_cmds, cmd_util_interval,
	(100, 100), (10, 10), (1000, 1000)
);


SHELL_STATIC_SUBCMD_SET_CREATE(sub_util,
	SHELL_CMD(hexdump, NULL, "Hexdump", cmd_util_hexdump),
	SHELL_CMD(echo, NULL, "Echo", cmd_util_echo),
	SHELL_CMD(interval, &sub_dict_cmds, "interval", NULL),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);
SHELL_CMD_REGISTER(util, &sub_util, "Util commands", NULL);

