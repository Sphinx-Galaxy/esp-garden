/*
 * Author: Mattis Jaksch
 * Created: 03.03.2022
 */

#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "display.h"

void app_main(void)
{
	// Display testing
    struct Display* dp = display_init();
  	
  	display_clear(dp);
  	
  	display_write(dp, "MATTIS");
  	display_write(dp, "TEST\n");
  	
  	char res[32];
  	
  	for(int i = 0; i < 32; ++i)
  	{
  		sprintf(res, "TESAT%d", i);
  		display_write(dp, res);
  	}
  	
  	display_destroy(dp);
}
