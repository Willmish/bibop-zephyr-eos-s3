#include <zephyr/drivers/sensor.h>

void bibop_get_mapped_values(const struct device *const sensor_max, struct sensor_value *ir, struct sensor_value *red);
int8_t bibop_max30102_remap(uint32_t old_min, uint32_t old_max, uint32_t old_val);
