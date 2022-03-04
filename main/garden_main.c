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
  		sprintf(res, "TESAT%d\n", i);
  		display_write(dp, res);
  	}
  	
  	display_destroy(dp);
}
