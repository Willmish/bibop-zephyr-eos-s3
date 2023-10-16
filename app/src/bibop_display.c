#include "bibop_display.h"

int  bdisplay_init(const struct device *dev, struct bibop_display_conf *display_conf) {
    if(display_set_pixel_format(dev, PIXEL_FORMAT_MONO10) != 0) {
        printf("Failed to set required pixel format\n");
        return 0;
    }

    printf("Initialized %s\n", dev->name);

	if (cfb_framebuffer_init(dev)) {
		printf("Framebuffer initialization failed!\n");
		return 0;
	}

	cfb_framebuffer_clear(dev, true);

	display_blanking_off(dev);

	display_conf->rows = cfb_get_display_parameter(dev, CFB_DISPLAY_ROWS);
	display_conf->ppt = cfb_get_display_parameter(dev, CFB_DISPLAY_PPT);

    cfb_framebuffer_set_font(dev, 0);
    printf("font width %d, font height %d\n",
           display_conf->font_width, display_conf->font_height);
    /*
	for (int idx = 0; idx < 42; idx++) {
		if (cfb_get_font_size(dev, idx, &display_conf->font_width, &display_conf->font_height)) {
			break;
		}
		cfb_framebuffer_set_font(dev, idx);
		printf("font width %d, font height %d\n",
		       display_conf->font_width, display_conf->font_height);
	}
    */

	printf("x_res %d, y_res %d, ppt %d, rows %d, cols %d\n",
	       cfb_get_display_parameter(dev, CFB_DISPLAY_WIDTH),
	       cfb_get_display_parameter(dev, CFB_DISPLAY_HEIGH),
	       display_conf->ppt,
	       display_conf->rows,
	       cfb_get_display_parameter(dev, CFB_DISPLAY_COLS));
    return 1;
}

void bdisplay_loop(const struct device *dev, struct bibop_display_conf *display_conf) {
    for (int i = 0; i < display_conf->rows; i++) {
        cfb_framebuffer_clear(dev, false);
        if (cfb_print(dev,
                  "0123456789mMgj!\"§$%&/()=",
                  0, i * display_conf->ppt)) {
            printf("Failed to print a string\n");
            continue;
        }

        cfb_framebuffer_finalize(dev);
        /*
#if defined(CONFIG_ARCH_POSIX)
        k_sleep(K_MSEC(100));
#endif
        */
    }
}

void bdisplay_writetext(const struct device *dev, struct bibop_display_conf *display_conf, const char *text) {
        cfb_framebuffer_clear(dev, false);
        if (cfb_print(dev,
                  text,
                  0, 2 * display_conf->ppt)) {
            printf("Failed to print a string\n");
            return;
        }

        cfb_framebuffer_finalize(dev);
}
