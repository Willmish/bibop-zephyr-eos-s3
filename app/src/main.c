/*
 * Copyright (c) 2023 Jakub Duchniewicz
 * Copyright (c) 2023 Szymon Duchniewicz
 * Copyright (c) 2023 Avanade Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>
#include <stdio.h>

#include "lis2dh.h"

#include "bibop_display.h"
#include "bibop_max30102.h"
#include "model_functions.hpp"
#include "bp_processing.hpp"


/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
/* The devicetree node indentrifier for the "button0" alias */
#define BUTTON0_NODE DT_ALIAS(sw0)
#define GPIO_NODE DT_NODELABEL(gpio)
#if !DT_NODE_HAS_STATUS(GPIO_NODE, okay)
#error "Unsupported board: gpio devicetree label is not defined"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON0_NODE, gpios);
const struct device *const sensor_max = DEVICE_DT_GET_ANY(maxim_max30101);
const struct device *dev_display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
int init_main();

int main(void)
{
    struct bibop_display_conf display_conf;
    if(!init_main())
        return 0;
    if (!bdisplay_init(dev_display, &display_conf)) {
        return 0;
    }
    /*
    bdisplay_loop(dev_display, &display_conf);
    */

    struct sensor_value ir;
    struct sensor_value red;
    char print_buf[64];
    /*
     * NOTE: SENSOR_CHAN_IR on our board IS CONNECTED TO THE RED LED
     * and SENSOR_CHAN_RED is connected TO THE IR LED. So data is actually coming from the other place
     */
    loop_model();
    while(1) {
        bibop_get_mapped_values(sensor_max, &ir, &red);
        if(ir.val2 == -128)
        {
            snprintk(print_buf, sizeof(print_buf), "Place your finger on the sensor");
        }
        else {
            snprintk(print_buf, sizeof(print_buf), "IR value:   %d", ir.val2);
        }
        bdisplay_writetext(dev_display, &display_conf, print_buf);
        k_sleep(K_MSEC(5)); // 5 ms intervals (200Hz)
    }
    return 0;
}

int init_main() {
	int ret;
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

    if(!gpio_is_ready_dt(&button)) {
        printk("Button device not ready.\n");
        return 0;
    }

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}
    if (sensor_max == NULL) {
        printk("No Max30101/2 device found\n");
        return 0;
    }
    if(!device_is_ready(sensor_max)) {
        printk("Device %s is not ready\n", sensor_max->name);
        return 0;
    }

    setup_model();
    preprocess_data();
    if(!device_is_ready(dev_display)) {
        printk("Device %s is not ready\n", dev_display->name);
        return 0;
    }
    return 1;
}
