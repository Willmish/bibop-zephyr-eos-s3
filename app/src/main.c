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

#define STACKSIZE 1024
#define PRIORITY 5
#define THREAD_DELAY_START 1000

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

static struct bibop_display_conf display_conf;

int init_main();
void sensor_task(void *, void *, void *);
void inference_task(void *, void *, void *);
void display_task(void *, void *, void *);

/* Define threads */
K_THREAD_DEFINE(sensor_task_id, STACKSIZE, sensor_task,
                NULL, NULL, NULL,
                PRIORITY, 0, THREAD_DELAY_START);
K_THREAD_DEFINE(inference_task_id, STACKSIZE, inference_task,
                NULL, NULL, NULL,
                PRIORITY, 0, THREAD_DELAY_START);
K_THREAD_DEFINE(display_task_id, STACKSIZE, display_task,
                NULL, NULL, NULL,
                PRIORITY, 0, THREAD_DELAY_START);

#define SENSOR_TASK_MS 5

/* Define queues for data */
K_FIFO_DEFINE(processing_fifo);
K_FIFO_DEFINE(display_fifo);

#define SENSOR_DATA_FIFO_SIZE 200

static int8_t processing_fifo_buffer[SENSOR_DATA_FIFO_SIZE];
static int16_t processing_fifo_idx;

static Features extracted_features;
static Inferred inferred_data;

int main(void)
{
    if(!init_main())
        return 0;
    if (!bdisplay_init(dev_display, &display_conf)) {
        return 0;
    }
    /*
    bdisplay_loop(dev_display, &display_conf);
    */

    //struct sensor_value red;
    //char print_buf[64];
    /*
     * NOTE: SENSOR_CHAN_IR on our board IS CONNECTED TO THE RED LED
     * and SENSOR_CHAN_RED is connected TO THE IR LED. So data is actually coming from the other place
     */
    while(1) {
        //if (ir.val2 == -128) {
        //    snprintk(print_buf, sizeof(print_buf), "Place your finger on the sensor");
        //}
        //else {
        //    snprintk(print_buf, sizeof(print_buf), "IR value:   %d", ir.val2);
        //}
        //bdisplay_writetext(dev_display, &display_conf, print_buf);
        //k_sleep(K_MSEC(5)); // 5 ms intervals (200Hz)
        k_sleep(K_MSEC(5)); // 5 ms intervals (200Hz)
    }
    return 0;
}

int init_main() {
	int ret;
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

    if (!gpio_is_ready_dt(&button)) {
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
    if (!device_is_ready(sensor_max)) {
        printk("Device %s is not ready\n", sensor_max->name);
        return 0;
    }

    setup_model();
    if (!device_is_ready(dev_display)) {
        printk("Device %s is not ready\n", dev_display->name);
        return 0;
    }
    return 1;
}

// TODO: add buffering of data and proper management of that
// add preprocessing of data and lastly displaying + buffering of outpu (semaphores)
// TODO: check if synchronisation is necessary, use simple queue for now
// adjust scaling of the data and ensure that proper data is printed on the screen
// as of now just display BP on the screen and maybe some other value (HR?)
// solder it and just have a small box powered by USB
// TODO: v3 add waking up with the user button
void sensor_task(void *p1, void *p2, void *p3)
{
    // IR is RED due to bug in the HW
    struct sensor_value ir, red;
    while (1) {
        //printk("hello from sensor_task\n");
        bibop_get_mapped_values(sensor_max, &ir, &red);

        processing_fifo_buffer[processing_fifo_idx++] = (int8_t)red.val2;

        if (processing_fifo_idx == SENSOR_DATA_FIFO_SIZE)
        {
            k_fifo_put(&processing_fifo, &processing_fifo_buffer);
            processing_fifo_idx = 0u;
        }

        k_sleep(K_MSEC(SENSOR_TASK_MS));
    }
}

void inference_task(void *p1, void *p2, void *p3)
{
    while (1) {
        printk("hello from inference_task\n");
        int8_t *processing_buffer = k_fifo_get(&processing_fifo, K_FOREVER);
        // TODO: right now nothing is done with this data - FIX IT
        for (int i = 0; i < 32; ++i)
            printf("%d\n", processing_buffer[i]);

        // TODO: for now preprocess_data expects floats - rescale int32_t to floats?
        extracted_features = preprocess_data(processing_buffer);

        // TODO: for now don't run the model
        // loop_model(extracted_features) and adjust the code
        inferred_data = loop_model(&extracted_features);
        k_fifo_put(&display_fifo, &inferred_data);

        k_sleep(K_MSEC(1000));
    }
}

void display_task(void *p1, void *p2, void *p3)
{
    char buf[32];
    while (1) {
        // TODO: display live data, for now just wait until re-drawing with new
        Inferred *inferred = (Inferred *) k_fifo_get(&display_fifo, K_FOREVER);
        printk("hello from display_task\n");
        sprintf(buf, "SBP: %.1f\nDBP: %.1f\n", inferred->sbp, inferred->dbp);

        //bdisplay_loop(dev_display, &display_conf);
        bdisplay_writetext(dev_display, &display_conf, buf);
        k_sleep(K_MSEC(1000));
    }
}
