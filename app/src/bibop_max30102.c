#include "bibop_max30102.h"
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <stdio.h>

#define MAX30102_MAX_IR 115000
#define MAX30102_MIN_IR 95000
#define MAX30102_MAX_RED 105000
#define MAX30102_MIN_RED 95000


int8_t bibop_max30102_remap(uint32_t old_min, uint32_t old_max, uint32_t old_val) {
    int8_t new_min = -128;
    int8_t new_max = 127;
    if (old_val > old_max)
        old_val = old_max;
    else if (old_val < old_min)
        old_val = old_min;
    int8_t new_value = (int8_t)((old_val - old_min) * (new_max - new_min) / (old_max - old_min) + new_min);
    return new_value;
}

void bibop_get_mapped_values(const struct device * const sensor_max, struct sensor_value *ir, struct sensor_value *red) {
        sensor_sample_fetch(sensor_max);
        sensor_channel_get(sensor_max, SENSOR_CHAN_IR, red);
        sensor_channel_get(sensor_max, SENSOR_CHAN_RED, ir);

        /* print green LED data */
        ir->val2 = bibop_max30102_remap(MAX30102_MIN_IR, MAX30102_MAX_IR, ir->val1);
        red->val2 = bibop_max30102_remap(MAX30102_MIN_RED, MAX30102_MAX_RED, red->val1);
        //if (new_ir == -128 && new_red == -128) {
        //    /* Super low ir and red values, probably finger not there */
        //    printf("Place yo finger on the sensor!\n");
        //    return;
        //}
        printf("IR=%d %d\n", ir->val1, ir->val2);
        printf("RED=%d %d\n", red->val1, red->val2);
}
