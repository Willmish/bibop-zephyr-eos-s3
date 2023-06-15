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
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON0_NODE, gpios);
const struct device *i2c_dev = DEVICE_DT_GET(I2C_NODE);

//I2C_Config i2c0config =
//{
//    .eI2CFreq = I2C_400KHZ,
//    .eI2CInt = I2C_DISABLE,
//    .ucI2Cn = 0
//};

int main(void)
{
	int ret;
    //HAL_StatusTypeDef hal_status;
    

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

    if(!gpio_is_ready_dt(&button)) {
        printk("Button device not ready.\n");
        return 0;
    }

    if (i2c_dev == NULL || !device_is_ready(i2c_dev)) {
        printk("I2C device not ready.\n");
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
    uint8_t who_am_i = 0;
        
    ret = i2c_reg_read_byte(i2c_dev, LIS2DH12_I2C_ADDR, LIS2DH12_WHO_AM_I, &who_am_i);
    if (ret) {
    printk("LIS2DH12 WHO AM I: 0x%x\n", who_am_i);
    }

    /*hal_status = HAL_I2C_Init(i2c0config);
    if (hal_status != HAL_OK) {
        printk("Failed to initialise I2C HAL interface! %x\n", hal_status);
        return 0;
    }
    uint8_t rval = 0;
    // check who am i lis2dh 
    hal_status = HAL_I2C_Read(LIS2DH12_I2C_ADDR, LIS2DH12_WHO_AM_I, &rval, 1);
    if (hal_status != HAL_OK) {
        printk("Failed to read over I2C HAL interface! %x\n", hal_status);
        return 0;
    }
    printk("LIS2DH12 WHO AM I: 0x%x\n", rval);
    */
    return 0;
}
