#include <zephyr/display/cfb.h>
#include <zephyr/device.h>
#include <stdio.h>

struct bibop_display_conf {
    uint16_t rows;
    uint8_t ppt;
    uint8_t font_width;
    uint8_t font_height;
};

int bdisplay_init(const struct device *dev, struct bibop_display_conf *display_conf);
void bdisplay_loop(const struct device *dev, struct bibop_display_conf *display_conf);
void bdisplay_writetext(const struct device *dev, struct bibop_display_conf *display_conf, const char *text);
