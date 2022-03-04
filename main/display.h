#pragma once

#include "affont.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

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
	uint16_t x_pos;
	uint16_t y_pos;
	esp_lcd_panel_handle_t panel_handle;
};

void display_clear(const struct Display* dp);
void display_write(struct Display* dp, const char* msg);
void update(struct Display* dp);
