#include "display.h"

#include <stdio.h>

void copy_msg_to_buffer(struct Display* dp, const char* msg);
struct Letter* select_letter(uint8_t input);
uint16_t get_letter_pixline(uint16_t* buffer, const uint16_t x_pos, const uint16_t y_pos, const struct Letter* letter);
void draw_letter_pixline(uint16_t* buffer, const uint16_t x_pos, const uint16_t y_pos, const struct Letter* letter);

void set_buffer_start(uint16_t buffer_pos, struct Display* dp);
bool is_end_of_buffer(const struct Display* dp);

void empty_buffer(uint16_t* buffer, const uint16_t start, const uint16_t stop);

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
		uint16_t buffer_pos = dp->buffer_start;
		//printf("Buffer pos at %d (x %d, y %d)\n", buffer_pos, dp->x_pos, dp->y_pos);
		dp->x_pos = 0; // Start at dp->x_pos
		while(dp->x_pos < LCD_H_RES
			&& buffer_pos < dp->buffer_stop
			&& dp->buffer[buffer_pos%LCD_BUFFER_SIZE] != '\n'
			&& dp->buffer[buffer_pos%LCD_BUFFER_SIZE] != '\0')
		{
			const struct Letter* letter = select_letter(dp->buffer[buffer_pos%LCD_BUFFER_SIZE]);
			
			if(dp->x_pos + letter->width + 2*LETTER_WIDTH_MARGIN < LCD_H_RES)
			{
				buffer_pos++;
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
			set_buffer_start(buffer_pos, dp);		
			end_of_buffer = is_end_of_buffer(dp);
		}
	}
}

void copy_msg_to_buffer(struct Display* dp, const char* msg)
{
	int pos = 0;
	do {
		dp->buffer[(pos+dp->buffer_start)%LCD_BUFFER_SIZE] = msg[pos];
		pos++;
	} while(msg[pos] != '\0');
	
	dp->buffer_stop = pos+dp->buffer_start;
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

void set_buffer_start(uint16_t buffer_pos, struct Display* dp)
{
	dp->buffer_start = buffer_pos;
	
	// Skip non-letters
	dp->buffer_start += (dp->buffer[buffer_pos%LCD_BUFFER_SIZE] == '\n' ? 1 : 0);
}
			
bool is_end_of_buffer(const struct Display* dp)
{
	return dp->buffer_start >= dp->buffer_stop
		|| dp->buffer[dp->buffer_start%LCD_BUFFER_SIZE] == '\0';
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
