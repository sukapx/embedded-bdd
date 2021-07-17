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

#define LED0_NODE DT_NODELABEL(led_0)
#define LED1_NODE DT_NODELABEL(led_1)

void blink0(void)
{
	bool led_is_on = true;
	const struct gpio_dt_spec led1 =
					GPIO_DT_SPEC_GET(LED1_NODE, gpios);
	gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
	while (1) {
		printk("[Thread1]\n");
		for(size_t loops = 0; loops < 10; loops++)
		{
			gpio_pin_set(led1.port, led1.pin, (int)led_is_on);
			led_is_on = !led_is_on;
			k_msleep(100);
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
		for(size_t loops = 0; loops < 10; loops++)
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
