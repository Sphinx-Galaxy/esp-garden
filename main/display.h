/*
 * Author: Mattis Jaksch
 * Created: 03.03.2022
 *
 * ESP-WROVER-KIT LCD (320x240px)
 */

#pragma once

#include "affont.h"

#include "esp_lcd_panel_io.h"

#define LCD_HOST       SPI2_HOST

#define LCD_PIXEL_CLOCK_HZ (10 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL  0
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL
#define PIN_NUM_DATA0          23  /*!< for 1-line SPI, this also refered as MOSI */
#define PIN_NUM_PCLK           19
#define PIN_NUM_CS             22
#define PIN_NUM_DC             21
#define PIN_NUM_RST            18
#define PIN_NUM_BK_LIGHT       5

// The pixel number in horizontal and vertical
#define LCD_H_RES              320
#define LCD_V_RES              240
// Bit number used to represent command and parameter
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

#if CONFIG_LCD_SPI_8_LINE_MODE
#define PIN_NUM_DATA1    7
#define PIN_NUM_DATA2    8
#define PIN_NUM_DATA3    9
#define PIN_NUM_DATA4    10
#define PIN_NUM_DATA5    11
#define PIN_NUM_DATA6    12
#define PIN_NUM_DATA7    13
#endif // CONFIG_LCD_SPI_8_LINE_MODE

#define LCD_H_RES		320
#define LCD_V_RES		240
#define LCD_LINE_HEIGHT	16
#define LCD_BUFFER_SIZE	1024

#define LETTER_HEIGHT_MARGIN 2
#define LETTER_WIDTH_MARGIN 1

static struct Display {
	char buffer[LCD_BUFFER_SIZE];
	uint16_t buffer_start;
	uint16_t buffer_stop;
	uint16_t buffer_pos;
	uint16_t x_pos;
	uint16_t y_pos;
	esp_lcd_panel_handle_t panel_handle;
};

struct Display* display_init();
void display_destroy(struct Display* dp);

void display_clear(const struct Display* dp);
void display_write(struct Display* dp, const char* msg);
