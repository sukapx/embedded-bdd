/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <shell/shell.h>
#include <console/console.h>

#define LED0_NODE DT_NODELABEL(led_0)
#define LED1_NODE DT_NODELABEL(led_1)

static long blinkInterval = 100;

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


SHELL_SUBCMD_DICT_SET_CREATE(sub_dict_cmds, cmd_util_interval,
	(100, 100), (10, 10), (1000, 1000)
);


SHELL_STATIC_SUBCMD_SET_CREATE(sub_util,
	SHELL_CMD(hexdump, NULL, "Hexdump", cmd_util_hexdump),
	SHELL_CMD(interval, &sub_dict_cmds, "interval", NULL),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);
SHELL_CMD_REGISTER(util, &sub_util, "Util commands", NULL);

void blink0(void)
{
	bool led_is_on = true;
	const struct gpio_dt_spec led1 =
					GPIO_DT_SPEC_GET(LED1_NODE, gpios);
	gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
	while (1) {
		printk("[Thread1]\n");
		for(size_t loops = 0; loops < 100; loops++)
		{
			gpio_pin_set(led1.port, led1.pin, (int)led_is_on);
			led_is_on = !led_is_on;
			k_msleep(blinkInterval);
		}
	}
}


void main(void)
{
	bool led_is_on = true;

	const struct gpio_dt_spec led0 =
					GPIO_DT_SPEC_GET(LED0_NODE, gpios);
	gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

	while (1) {
		printk("[Thread0] %s\n", CONFIG_BOARD);
		for(size_t loops = 0; loops < 100; loops++)
		{
			gpio_pin_set(led0.port, led0.pin, (int)led_is_on);
			led_is_on = !led_is_on;
			k_msleep(100);
		}
	}
}

#define STACKSIZE 1024
#define PRIORITY 7

K_THREAD_DEFINE(blink0_id, STACKSIZE, blink0, NULL, NULL, NULL,
		PRIORITY, 0, 0);
