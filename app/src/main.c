/*
 * Copyright (c) 2023 Jakub Duchniewicz
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>
#include <stdio.h>

#include "eoss3_hal_i2c.h"
#include "eoss3_hal_gpio.h"
#include "lis2dh.h"

//#include "app_version.h"


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

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON0_NODE, gpios);

#ifdef CONFIG_LIS2DH_TRIGGER
static void trigger_handler(const struct device *dev,
			    const struct sensor_trigger *trig)
{
	fetch_and_display(dev);
}
#endif

I2C_Config i2c0config =
{
    .eI2CFreq = I2C_400KHZ,
    .eI2CInt = I2C_DISABLE,
    .ucI2Cn = 0
};

int main(void)
{
	int ret;
    HAL_StatusTypeDef hal_status;
    

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
    
    //ret = gpio_pin_configure_dt(&button, GPIO_INPUT); // TODO: THIS MESSES UP THE ACTUAL PINMUX????
	if (ret < 0) {
		return 0;
	}
    //printk("Reading button..\n");
    //while (1) {
	//		/* If we have an LED, match its state to the button's. */
	//		int val = gpio_pin_get_dt(&button);
    //        printk("%d", val);
    //        //uint8_t val2;
    //        //HAL_GPIO_Read(0, &val2);
    //        //printk("%d", val2);

	//		if (val >= 0) {
	//			gpio_pin_set_dt(&led, val);
	//		}
	//		k_msleep(SLEEP_TIME_MS);
	//	}
    printk("Initialising I2C..\n");
    hal_status = HAL_I2C_Init(i2c0config);
    if (hal_status != HAL_OK) {
        printk("Failed to initialise I2C HAL interface! %x\n", hal_status);
        return 0;
    }
    uint8_t rval[2] = {1,1};
    /* check who am i lis2dh */
    hal_status = HAL_I2C_Read(LIS2DH12_I2C_ADDR, LIS2DH12_WHO_AM_I, rval, 1);
    if (hal_status != HAL_OK) {
        printk("Failed to read over I2C HAL interface! %x\n", hal_status);
        return 0;
    }
    printk("LIS2DH12 WHO AM I: 0x%x\n", rval[0]);


	const struct device *const sensor = DEVICE_DT_GET_ANY(st_lis2dh);

	if (sensor == NULL) {
		printk("No device found\n");
		return 0;
	}
	if (!device_is_ready(sensor)) {
		printk("Device %s is not ready\n", sensor->name);
		return 0;
	}

#if CONFIG_LIS2DH_TRIGGER
	{
		struct sensor_trigger trig;
		int rc;

		trig.type = SENSOR_TRIG_DATA_READY;
		trig.chan = SENSOR_CHAN_ACCEL_XYZ;

		if (IS_ENABLED(CONFIG_LIS2DH_ODR_RUNTIME)) {
			struct sensor_value odr = {
				.val1 = 1,
			};

			rc = sensor_attr_set(sensor, trig.chan,
					     SENSOR_ATTR_SAMPLING_FREQUENCY,
					     &odr);
			if (rc != 0) {
				printf("Failed to set odr: %d\n", rc);
				return 0;
			}
			printf("Sampling at %u Hz\n", odr.val1);
		}

		rc = sensor_trigger_set(sensor, &trig, trigger_handler);
		if (rc != 0) {
			printf("Failed to set trigger: %d\n", rc);
			return 0;
		}

		printf("Waiting for triggers\n");
		while (true) {
			k_sleep(K_MSEC(2000));
		}
	}
#else /* CONFIG_LIS2DH_TRIGGER */
	printf("Polling at 0.5 Hz\n"); // Don't wait for trigger right now
	while (true) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return 0;
		}
		fetch_and_display(sensor);
		k_sleep(K_MSEC(2000));
	}
#endif /* CONFIG_LIS2DH_TRIGGER */
}
