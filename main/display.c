#include "display.h"

#include <stdio.h>

#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

void copy_msg_to_buffer(struct Display* dp, const char* msg);
struct Letter* select_letter(uint8_t input);
uint16_t get_letter_pixline(uint16_t* buffer, const uint16_t x_pos, const uint16_t y_pos, const struct Letter* letter);
void draw_letter_pixline(uint16_t* buffer, const uint16_t x_pos, const uint16_t y_pos, const struct Letter* letter);

void set_buffer_start(struct Display* dp);

bool is_end_of_buffer(const struct Display* dp);
bool is_end_of_line(const struct Display* dp);

void empty_buffer(uint16_t* buffer, const uint16_t start, const uint16_t stop);

struct Display* display_init() 
{
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_NUM_BK_LIGHT
    };
    // Initialize the GPIO of backlight
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_PCLK,
        .mosi_io_num = PIN_NUM_DATA0,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * 2 + 8
    };
#if CONFIG_LCD_SPI_8_LINE_MODE
    buscfg.data1_io_num = PIN_NUM_DATA1;
    buscfg.data2_io_num = PIN_NUM_DATA2;
    buscfg.data3_io_num = PIN_NUM_DATA3;
    buscfg.data4_io_num = PIN_NUM_DATA4;
    buscfg.data5_io_num = PIN_NUM_DATA5;
    buscfg.data6_io_num = PIN_NUM_DATA6;
    buscfg.data7_io_num = PIN_NUM_DATA7;
    buscfg.flags = SPICOMMON_BUSFLAG_OCTAL;
#endif
    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
#if CONFIG_LCD_SPI_8_LINE_MODE
    io_config.spi_mode = 3;  // using mode 3 to simulate Intel 8080 timing
    io_config.flags.octal_mode = 1;
#endif
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = 16,
    };
    // Initialize the LCD configuration
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    // Turn off backlight to avoid unpredictable display on the LCD screen while initializing
    // the LCD panel driver. (Different LCD screens may need different levels)
    ESP_ERROR_CHECK(gpio_set_level(PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_OFF_LEVEL));

    // Reset the display
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

    // Initialize LCD panel
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    // Swap x and y axis (Different LCD screens may need different options)
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));

    // Turn on backlight (Different LCD screens may need different levels)
    ESP_ERROR_CHECK(gpio_set_level(PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_ON_LEVEL));
    
    
    struct Display* dp = malloc(sizeof(struct Display));
    dp->buffer_start = 0;
    dp->buffer_stop = 0;
    dp->buffer_pos = 0;
    dp->x_pos= 0;
    dp->y_pos = 0;
    dp->panel_handle = panel_handle;
    
    return dp;
}

void display_destroy(struct Display* dp)
{

}

void display_write(struct Display* dp, const char* msg)
{
	copy_msg_to_buffer(dp, msg);
						
	static uint16_t line[2][LCD_H_RES];
	static uint8_t calc_line = 0;
	static uint8_t send_line = 0;
	bool end_of_buffer = false;
	
	dp->y_pos = dp->y_pos%LCD_V_RES;
		
	while(dp->y_pos < LCD_V_RES && !end_of_buffer) {
	
		empty_buffer(line[calc_line], 0, LCD_H_RES);
		
		/* Write line function */
		dp->buffer_pos = dp->buffer_start;
		dp->x_pos = 0;
		while(dp->x_pos < LCD_H_RES
			&& dp->buffer_pos < dp->buffer_stop
			&& dp->buffer[dp->buffer_pos%LCD_BUFFER_SIZE] != '\n'
			&& dp->buffer[dp->buffer_pos%LCD_BUFFER_SIZE] != '\0')
		{
			const struct Letter* letter = select_letter(dp->buffer[dp->buffer_pos%LCD_BUFFER_SIZE]);
			
			if(letter->width + 2 * LETTER_WIDTH_MARGIN + dp->x_pos < LCD_H_RES)
			{
				dp->buffer_pos++;
				dp->x_pos += get_letter_pixline(line[calc_line], dp->x_pos, dp->y_pos, letter);
			}
		}
			
		/**/
		send_line = calc_line;
		calc_line = !calc_line;
		
		esp_lcd_panel_draw_bitmap(dp->panel_handle, 0, dp->y_pos, LCD_H_RES, dp->y_pos + 1, line[send_line]);
		
		dp->y_pos++;
		
		if(dp->y_pos%LCD_LINE_HEIGHT == 0)
		{			
			set_buffer_start(dp);		
			end_of_buffer = is_end_of_buffer(dp);
		}
	}
}

void copy_msg_to_buffer(struct Display* dp, const char* msg)
{
	int pos = 0;
	do {
		dp->buffer[(dp->buffer_stop + pos)%LCD_BUFFER_SIZE] = msg[pos];
		pos++;
	} while(msg[pos] != '\0');
	
	dp->buffer_stop += pos;
}

struct Letter* select_letter(uint8_t input)
{	
	for(uint8_t i = 0; i < 18; ++i)
	{
		if(affont[i].ascii_index == input)
		{
			return &affont[i];
		}
	}
	
	return &affont[0];
}

uint16_t get_letter_pixline(uint16_t* buffer, const uint16_t x_pos, const uint16_t y_pos, const struct Letter* letter)
{
	uint16_t total_width = (letter->width + 2 * LETTER_WIDTH_MARGIN);
	
	empty_buffer(buffer, x_pos, x_pos + total_width);
	
	draw_letter_pixline(buffer, x_pos, y_pos%LCD_LINE_HEIGHT, letter);
	
	return total_width;
}

void draw_letter_pixline(uint16_t* buffer, const uint16_t x_pos, const uint16_t y_pos, const struct Letter* letter)
{
	// Only draw within margin
	if(LETTER_HEIGHT_MARGIN <= y_pos && y_pos < (LCD_LINE_HEIGHT - LETTER_HEIGHT_MARGIN))
	{
		for(uint8_t i = 0; i < letter->dot_cnt; ++i)
		{
			// Draw only dots within the current line
			if((y_pos - LETTER_HEIGHT_MARGIN) * letter->width <= letter->dots[i] 
				&& (y_pos - LETTER_HEIGHT_MARGIN + 1) * letter->width > letter->dots[i])
			{
				uint16_t dot_pos = letter->dots[i]%letter->width + LETTER_WIDTH_MARGIN + x_pos;
				buffer[dot_pos] = 0xFFFF;	
			}
		}
	}
}
	
void set_buffer_start(struct Display* dp)
{
	if(is_end_of_line(dp))
	{
		dp->buffer_pos += (dp->buffer[dp->buffer_pos%LCD_BUFFER_SIZE] == '\n' ? 1 : 0); // Skip non-letters
		dp->buffer_start = dp->buffer_pos;
	}
	else
	{
		dp->y_pos -= 16;
	}
}
			
bool is_end_of_buffer(const struct Display* dp)
{		
	return dp->buffer_pos >= dp->buffer_stop
		|| dp->buffer[dp->buffer_pos%LCD_BUFFER_SIZE] == '\0';
}

bool is_end_of_line(const struct Display* dp)
{
	return MAX_LETTER_WIDTH + 2 * LETTER_WIDTH_MARGIN + dp->x_pos >= LCD_H_RES
		|| dp->buffer[dp->buffer_pos%LCD_BUFFER_SIZE] == '\n';
}

void empty_buffer(uint16_t* buffer, const uint16_t start, const uint16_t stop)
{
	for(uint16_t i = start; i < stop; ++i)
	{
		buffer[i] = 0x0;
	}
}

void display_clear(const struct Display* dp)
{
	uint16_t line[LCD_H_RES] = {0};
		
	for(uint16_t y = 0; y < LCD_V_RES; ++y) 
	{
		esp_lcd_panel_draw_bitmap(dp->panel_handle, 0, y, LCD_H_RES, y+1, line);
	}
}
