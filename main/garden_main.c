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
  	
  	display_write(dp, "01234569\n");
  	display_write(dp, "ABCDEFGHI\n");
  	display_write(dp, "JKLMNOPQR\n");
  	display_write(dp, "STUVWXYZ\n");
  	
  	/*char res[32];
  	
  	for(int i = 0; i < 8; ++i)
  	{
  		sprintf(res, "MATTIS%d\n", i);
  		display_write(dp, res);
  	}*/
  	
  	display_destroy(dp);
}
