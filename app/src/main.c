/*
 * Copyright (c) 2023 Jakub Duchniewicz
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

//#include "eoss3_hal_i2c.h"
//#include "eoss3_hal_gpio.h"
#include "lis2dh.h"

//#include "app_version.h"
#include "model_functions.hpp"


/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
/* The devicetree node indentrifier for the "button0" alias */
#define BUTTON0_NODE DT_ALIAS(sw0)
#define GPIO_NODE DT_NODELABEL(gpio)
#define I2C_NODE DT_NODELABEL(i2c0)
#if !DT_NODE_HAS_STATUS(GPIO_NODE, okay)
#error "Unsupported board: gpio devicetree label is not defined"
#endif
#if !DT_NODE_HAS_STATUS(I2C_NODE, okay)
#error "Unsupported board: I2C0 devicetree label is not defined"
#endif
/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static void fetch_and_display(const struct device *sensor)
{
	static unsigned int count;
	struct sensor_value accel[3];
	struct sensor_value temperature;
	const char *overrun = "";
	int rc = sensor_sample_fetch(sensor);

	++count;
	if (rc == -EBADMSG) {
		/* Sample overrun.  Ignore in polled mode. */
		if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
			overrun = "[OVERRUN] ";
		}
		rc = 0;
	}
	if (rc == 0) {
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_ACCEL_XYZ,
					accel);
	}
	if (rc < 0) {
		printf("ERROR: Update failed: %d\n", rc);
	} else {
		printf("#%u @ %u ms: %sx %f , y %f , z %f",
		       count, k_uptime_get_32(), overrun,
		       sensor_value_to_double(&accel[0]),
		       sensor_value_to_double(&accel[1]),
		       sensor_value_to_double(&accel[2]));
	}

	if (IS_ENABLED(CONFIG_LIS2DH_MEASURE_TEMPERATURE)) {
		if (rc == 0) {
			rc = sensor_channel_get(sensor, SENSOR_CHAN_DIE_TEMP, &temperature);
			if (rc < 0) {
				printf("\nERROR: Unable to read temperature:%d\n", rc);
			} else {
				printf(", t %f\n", sensor_value_to_double(&temperature));
			}
		}

	} else {
		printf("\n");
	}
}

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON0_NODE, gpios);
//const struct device *i2c_dev = DEVICE_DT_GET(I2C_NODE);
//const struct device *const sensor = DEVICE_DT_GET_ANY(st_lis2dh);
//const struct device *const sensor_max = DEVICE_DT_GET_ANY(maxim_max30101);

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

    if(!gpio_is_ready_dt(&button)) {
        printk("Button device not ready.\n");
        return 0;
    }

    /*
    if (i2c_dev == NULL || !device_is_ready(i2c_dev)) {
        printk("I2C device not ready.\n");
        return 0;
    }
    */

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

    //if (sensor_max == NULL) {
    //    printk("No Max30101/2 device found\n");
    //    return 0;
    //}
    //if(!device_is_ready(sensor_max)) {
    //    printk("Device %s is not ready\n", sensor_max->name);
    //    return 0;
    //}

    setup_model();

    struct sensor_value green;
    while(1) {
        //sensor_sample_fetch(sensor_max);
        //sensor_channel_get(sensor_max, SENSOR_CHAN_GREEN, &green);
        loop_model();

        /* print green LED data */
        printf("GREEN=%d\n", green.val1);
        k_sleep(K_MSEC(20));
    }

    /*
    if (sensor == NULL) {
        printk("No Lis2dh device found\n");
        return 0;
    }
    if(!device_is_ready(sensor)) {
        printk("Device %s is not ready\n", sensor->name);
        return 0;
    }
    printk("Polling at 2Hz\n");
    while(true) {
        fetch_and_display(sensor);
        k_sleep(K_MSEC(500));
    }
    */
    /*
    printk("Initialising I2C..\n");
    uint8_t who_am_i = 0;

    ret = i2c_reg_read_byte(i2c_dev, 0x57, 0xff, &who_am_i);
    if (ret == 0) {
    printk("MAX30102 WHO AM I: 0x%x\n", who_am_i);
    }
    */

    /*ret = gpio_pin_configure_dt(&button, GPIO_INPUT); // TODO: THIS MESSES UP THE ACTUAL PINMUX????
	if (ret < 0) {
		return 0;
	}
    printk("Reading button..\n");
    while (1) {
	//	If we have an LED, match its state to the button's.
			int val = gpio_pin_get_dt(&button);
            printk("%d", val);
            //uint8_t val2;
            //HAL_GPIO_Read(0, &val2);
            //printk("%d", val2);

			if (val >= 0) {
				gpio_pin_set_dt(&led, val);
			}
			k_msleep(SLEEP_TIME_MS);
		}
    	*/
    return 0;
}
