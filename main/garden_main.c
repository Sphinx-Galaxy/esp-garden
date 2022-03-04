/* LCD tjpgd example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "display.h"

// Using SPI2 in the example, as it aslo supports octal modes on some targets
#define LCD_HOST       SPI2_HOST
// To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many.
// More means more memory use, but less overhead for setting up / finishing transfers. Make sure 240
// is dividable by this.
#define PARALLEL_LINES 16
// The number of frames to show before rotate the graph
#define ROTATE_FRAME   30

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (10 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  0
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_DATA0          23  /*!< for 1-line SPI, this also refered as MOSI */
#define EXAMPLE_PIN_NUM_PCLK           19
#define EXAMPLE_PIN_NUM_CS             22
#define EXAMPLE_PIN_NUM_DC             21
#define EXAMPLE_PIN_NUM_RST            18
#define EXAMPLE_PIN_NUM_BK_LIGHT       5

// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES              320
#define EXAMPLE_LCD_V_RES              240
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

#if CONFIG_EXAMPLE_LCD_SPI_8_LINE_MODE
#define EXAMPLE_PIN_NUM_DATA1    7
#define EXAMPLE_PIN_NUM_DATA2    8
#define EXAMPLE_PIN_NUM_DATA3    9
#define EXAMPLE_PIN_NUM_DATA4    10
#define EXAMPLE_PIN_NUM_DATA5    11
#define EXAMPLE_PIN_NUM_DATA6    12
#define EXAMPLE_PIN_NUM_DATA7    13
#endif // CONFIG_EXAMPLE_LCD_SPI_8_LINE_MODE

void app_main(void)
{
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT
    };
    // Initialize the GPIO of backlight
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    spi_bus_config_t buscfg = {
        .sclk_io_num = EXAMPLE_PIN_NUM_PCLK,
        .mosi_io_num = EXAMPLE_PIN_NUM_DATA0,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = PARALLEL_LINES * EXAMPLE_LCD_H_RES * 2 + 8
    };
#if CONFIG_EXAMPLE_LCD_SPI_8_LINE_MODE
    buscfg.data1_io_num = EXAMPLE_PIN_NUM_DATA1;
    buscfg.data2_io_num = EXAMPLE_PIN_NUM_DATA2;
    buscfg.data3_io_num = EXAMPLE_PIN_NUM_DATA3;
    buscfg.data4_io_num = EXAMPLE_PIN_NUM_DATA4;
    buscfg.data5_io_num = EXAMPLE_PIN_NUM_DATA5;
    buscfg.data6_io_num = EXAMPLE_PIN_NUM_DATA6;
    buscfg.data7_io_num = EXAMPLE_PIN_NUM_DATA7;
    buscfg.flags = SPICOMMON_BUSFLAG_OCTAL;
#endif
    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_PIN_NUM_DC,
        .cs_gpio_num = EXAMPLE_PIN_NUM_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
#if CONFIG_EXAMPLE_LCD_SPI_8_LINE_MODE
    io_config.spi_mode = 3;  // using mode 3 to simulate Intel 8080 timing
    io_config.flags.octal_mode = 1;
#endif
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = 16,
    };
    // Initialize the LCD configuration
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    // Turn off backlight to avoid unpredictable display on the LCD screen while initializing
    // the LCD panel driver. (Different LCD screens may need different levels)
    ESP_ERROR_CHECK(gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL));

    // Reset the display
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

    // Initialize LCD panel
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    // Swap x and y axis (Different LCD screens may need different options)
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));

    // Turn on backlight (Different LCD screens may need different levels)
    ESP_ERROR_CHECK(gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL));
	
	// Display testing
    struct Display dp = {{}, 0, 0, 0, 0, 0, panel_handle};
  	
  	display_clear(&dp);
  	
  	display_write(&dp, "MATTIS");
  	display_write(&dp, "TEST\n");
  	
  	char res[32];
  	
  	for(int i = 0; i < 13; ++i)
  	{
  		sprintf(res, "TESAT%d\n", i);
  		display_write(&dp, res);
  	}
}
