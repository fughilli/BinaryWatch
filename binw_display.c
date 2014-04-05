/*
 * binw_display.c
 *
 *  Created on: Dec 16, 2013
 *      Author: Kevin
 *
 *      (c) Kevin Balke 2013. All rights reserved.
 */

#include "binw_display.h"
#include "binw_matrix.h"
#include "binw_clock.h"
#include "font4x4.h"

/*
 * EXAMPLES OF DISPLAY_NUMERIC:
 * 	15:12:23
 * 	03:12:23 PM
 * EXAMPLES OF DISPLAY_TEXT:
 * 	This is some text.
 * 	I like writing things on tiny screens.
 */

// VARIABLES
// Index of the current LED being operated upon in the matrix
uint8_t current_led;

// Screen buffer to store the rendered image of the scrolling text in
// before flipping to the matrix register
MATRIX_REGISTER_T matrix_text_buffer;

// Timing variables
uint16_t text_scroll_delay;
uint16_t text_scroll_count;

uint16_t display_show_delay;
uint16_t display_show_count;

uint16_t display_flash_delay;
uint16_t display_flash_count;

// Cursor position of the last line in the string to be printed
uint16_t text_cursor_len;

// Display in military (24H) time?
bool military_time;

// Pointer to string to be printed
uint8_t * printstr;

// Buffer to store the numeric time string in
uint8_t text_time_buffer[16];

// Current bitmap line in the string of bitmap characters to be printed
uint16_t text_scroll_cursor;

// Is the display currently displaying text?
bool displaying_text;

// Is the flash on tick?
bool display_flash;

bool display_tick_flag;

bool display_scroll_loop;

// State enums
PRINT_STATE print_state;
DISPLAY_MODE display_mode;
DISPLAY_SETTING_TIME_MODE display_setting_time_mode;

// FUNCTION PROTOTYPES
void matrix_draw_text(MATRIX_REGISTER_T * matrix_reg, uint8_t * ch,
		uint16_t cursor);
void matrix_draw_text_wrapping(MATRIX_REGISTER_T * matrix_reg, uint8_t * ch,
		uint16_t str_len, uint16_t cursor);
void matrix_draw_dec_coded_binary(MATRIX_REGISTER_T * matrix_reg,
		CLOCK_STATE * clock_st);
void matrix_draw_binary(MATRIX_REGISTER_T * matrix_reg, CLOCK_STATE * clock_st);
void matrix_draw_seconds_binary(MATRIX_REGISTER_T * matrix_reg,
		CLOCK_STATE * clock_st);
void display_print_tick(void);
void display_sprint_time_numeric(uint8_t * buf, CLOCK_STATE * clock_st);

/**
 * DRAWS A TEXT BUFFER TO THE DISPLAY AT THE GIVEN CURSOR POSITION
 */
void matrix_draw_text(MATRIX_REGISTER_T * matrix_reg, uint8_t * ch,
		uint16_t cursor)
{
	// Advance the text pointer to the first char that is on the screen
	uint16_t cur_char = cursor / CHAR_SIZE_WIDTH;
	ch += cur_char;
	cursor -= cur_char * CHAR_SIZE_WIDTH;

	// Fill the display buffer
	uint8_t i;
	for (i = 0; i < MATRIX_COLS; i++)
	{
		// Shift the display buffer left a column
		*matrix_reg <<= CHAR_SIZE_HEIGHT;
		// Fill the rightmost column with the appropriate column from the font table
		*matrix_reg |= (CHAR_TO_FONT_BITS(*(ch + (cursor + i)/CHAR_SIZE_WIDTH))>>
		((((i + cursor)%CHAR_SIZE_WIDTH)*CHAR_SIZE_HEIGHT))) & (0xFF >> 8-CHAR_SIZE_HEIGHT);
	}
}

/**
 * DRAWS A TEXT BUFFER TO THE DISPLAY AT THE GIVEN CURSOR POSITION, WRAPPING TO THE BEGINNING
 * IF THE CURSOR POSITION OVERRUNS THE BUFFER
 */
void matrix_draw_text_wrapping(MATRIX_REGISTER_T * matrix_reg, uint8_t * ch,
		uint16_t str_len, uint16_t cursor)
{
	// Compute the string offset
	uint16_t cur_char = cursor / CHAR_SIZE_WIDTH;

	// Make sure that it is within the string
	while (cur_char > str_len)
	{
		cur_char -= str_len;
	}

	// Advance the text pointer to the first char that is on the screen
	ch += cur_char;
	cursor -= cur_char * CHAR_SIZE_WIDTH;

	uint8_t i;
	uint16_t char_offset;
	for (i = 0; i < MATRIX_COLS; i++)
	{
		// Compute the char offset to allow for wrapping
		char_offset = (cursor + i) / CHAR_SIZE_WIDTH;
		// If the offset is beyond the string's length, then wrap it
		if (char_offset > str_len)
		{
			char_offset -= str_len;
		}
		*matrix_reg <<= CHAR_SIZE_HEIGHT;
		*matrix_reg |= (CHAR_TO_FONT_BITS(*(ch + char_offset))>>
		((((i + cursor)%CHAR_SIZE_WIDTH)*CHAR_SIZE_HEIGHT))) & (0xFF >> 8-CHAR_SIZE_HEIGHT);
	}
}

/**
 * PRINTS A STRING TO THE DISPLAY
 */
void display_print(uint8_t * str)
{
	displaying_text = true;
	text_cursor_len = 0;
	printstr = str;
	text_scroll_cursor = 0;
	matrix_text_buffer = 0;

	// Compute the maximum cursur position for the text
	while (*(str + text_cursor_len))
	{
		text_cursor_len++;
	}

	text_cursor_len *= CHAR_SIZE_WIDTH;

	text_cursor_len -= MATRIX_COLS;

	// Set the print state to leading whitespace
	print_state = PRINT_WHITESPACE_START;

	// Clear the matrix register
	matrix_register = 0;
}

/**
 * UPDATES THE DISPLAY BUFFER WHILST DRAWING TEXT
 */
void display_print_tick(void)
{
	text_scroll_cursor++;
	switch (print_state)
	{
	case PRINT_WHITESPACE_START:
	{
		matrix_text_buffer = 0;
		matrix_draw_text(&matrix_text_buffer, printstr, 0);

		matrix_register = matrix_text_buffer
				>> ((MATRIX_COLS - text_scroll_cursor) * MATRIX_ROWS);
		if (text_scroll_cursor == MATRIX_COLS)
		{
			text_scroll_cursor = 0;
			print_state = PRINT_TEXT;
		}
		break;
	}
	case PRINT_TEXT:
	{
		matrix_draw_text(&matrix_register, printstr, text_scroll_cursor);
		if (text_scroll_cursor == text_cursor_len)
		{
			text_scroll_cursor = 0;
			print_state = PRINT_WHITESPACE_END;
		}
		break;
	}
	case PRINT_WHITESPACE_END:
	{
		matrix_register = 0;
		matrix_draw_text(&matrix_register, printstr, text_cursor_len);
		matrix_register <<= text_scroll_cursor * MATRIX_ROWS;
		if (text_scroll_cursor == MATRIX_COLS)
		{
			text_scroll_cursor = 0;
			if (display_scroll_loop)
			{
				print_state = PRINT_WHITESPACE_START;
			}
			else
			{
				print_state = PRINT_IDLE;
				if (display_mode == DISPLAY_NUMERIC)
					display_show_count = display_show_delay;
				displaying_text = false;
			}
		}
		break;
	}
	default:
		text_scroll_cursor = 0;
		break; // Should never get here
	}

	// Flash the corners of the display to indicate being in mode-setting mode
	if (display_setting_time_mode == SETTING_MODE)
	{
		if (display_flash)
		{
			matrix_register |= 0x00900009;
		}
	}
}

/**
 * UPDATES THE DISPLAY (Each call draws one pixel; MATRIX_ROWS*MATRIX_COLS calls will get one
 * screen refresh. Display buffer updates are performed every screen refresh.)
 */
void display_draw_tick(void)
{
	// If the display is on (otherwise there is no point to doing any of this)
	if (display_mode)
	{
		// Light the LEDs
		matrix_update();
		current_led++;
		// If one screen refresh has been completed
		if (current_led == MATRIX_LEDS)
		{
			current_led = 0;

			/******************************************************************
			 * PERFORM ANY ACTIONS THAT NEED TO OCCUR ONCE PER SCREEN REFRESH *
			 ******************************************************************/

			if (display_tick_flag)
			{
				display_tick_flag = false;

				/****************************************************************
				 * PERFORM ANY ACTIONS THAT NEED TO OCCUR ONCE PER DISPLAY TICK *
				 ****************************************************************/

				// Call display_print_tick() if the display tick delay has been reached
				if (displaying_text)
				{
					text_scroll_count++;
					if (text_scroll_count >= text_scroll_delay)
					{
						text_scroll_count = 0;
						if (print_state != PRINT_IDLE)
							display_print_tick();
					}
				}

				// Change the flash flag if the flash delay has been reached, and if flashing is
				// currently on
				if (display_setting_time_mode)
				{
					display_flash_count++;
					if (display_flash_count >= display_flash_delay)
					{
						display_flash_count = 0;
						display_flash = !display_flash;
						display_clock_tick();
					}
				}

				// Perform the display timeout if necessary (if the delay has been set to 0, the
				// display will never timeout)
				if (display_show_delay)
				{
					display_show_count++;
					if (display_show_count >= display_show_delay)
					{
						if (!displaying_text)
						{
							display_show_count = 0;
							display_show(DISPLAY_OFF);
#ifdef DISPLAY_TIMEOUT_CALLBACK
							display_timeout_callback();
#endif /* DISPLAY_TIMEOUT_CALLBACK */
						}
						else
						{
							display_show_count--;
						}
					}
				}
			}
		}
	}
}

/**
 * SET THE FLAG FOR EVENTS that need to occur less often than every screen refresh
 * but more often than every second
 */
void display_tick(void)
{
	display_tick_flag = true;
}

/**
 * UPDATES THE TIMESTAMPS (Calls should happen once a second.)
 */
void display_clock_tick(void)
{
	switch (display_mode)
	{
	case DISPLAY_DEC_CODED_BINARY:
	{
		matrix_draw_dec_coded_binary(&matrix_register, &clock_state);
		break;
	}
	case DISPLAY_BINARY:
	{
		matrix_draw_binary(&matrix_register, &clock_state);
		break;
	}
	case DISPLAY_SECONDS_BINARY:
	{
		matrix_draw_seconds_binary(&matrix_register, &clock_state);
		break;
	}
	case DISPLAY_NUMERIC:
	{
		display_sprint_time_numeric(text_time_buffer, &clock_state);
		break;
	}
	default:
		break;
	}
}

/**
 * SHOW THE TIME TO THE USER
 */
void display_show(DISPLAY_MODE disp_mode)
{
	display_show_count = 0;

	switch (disp_mode)
	{
	case DISPLAY_NUMERIC:
	{
		if (display_mode != DISPLAY_NUMERIC)
		{
			display_sprint_time_numeric(text_time_buffer, &clock_state);
			display_print(text_time_buffer);
		}
		break;
	}
	case DISPLAY_OFF:
	{
		matrix_register = 0;
		displaying_text = false;
		break;
	}
	default:
	{
		displaying_text = false;
		break;
	}
	}
	display_mode = disp_mode;
}

/**
 * INITIALIZE THE DISPLAY
 */
void display_setup(void)
{
	current_led = 0;
	matrix_text_buffer = 0;

	text_scroll_count = 0;
	text_scroll_delay = DISPLAY_DEFAULT_TEXT_SCROLL_DELAY;

	display_show_count = 0;
	display_show_delay = DISPLAY_DEFAULT_SHOW_DELAY;

	display_flash_count = 0;
	display_flash_delay = DISPLAY_DEFAULT_FLASH_DELAY;

	display_flash = false;

	display_tick_flag = false;

	display_scroll_loop = false;

	military_time = true;
	displaying_text = false;

	display_mode = DISPLAY_OFF;
	print_state = PRINT_IDLE;
}

/**
 * SET THE NUMBER OF DISPLAY TICKS BETWEEN TEXT CURSOR INCREMENTS
 */
void display_set_text_scroll_delay(uint16_t delay)
{
	if (delay > 0)
		text_scroll_delay = delay;
	else
		text_scroll_delay = DISPLAY_DEFAULT_TEXT_SCROLL_DELAY;
}

/**
 * SET THE NUMBER OF DISPLAY TICKS BETWEEN FLASH FLIPS
 */
void display_set_setting_time_flash_delay(uint16_t delay)
{
	if (delay > 0)
		display_flash_delay = delay;
	else
		display_flash_delay = DISPLAY_DEFAULT_FLASH_DELAY;
}

/**
 * SET THE NUMBER OF DISPLAY TICKS UNTIL SCREEN TIMEOUT
 */
void display_set_show_delay(uint16_t delay)
{
	display_show_delay = delay;
}

void display_reset_timeout(void)
{
	display_show_count = 0;
}

#define switch_flash_hours(a,b) (((display_setting_time_mode == SETTING_HOURS) && display_flash)?(b):(a))
#define switch_flash_minutes(a,b) (((display_setting_time_mode == SETTING_MINUTES) && display_flash)?(b):(a))
#define switch_flash_seconds(a,b) (((display_setting_time_mode == SETTING_SECONDS) && display_flash)?(b):(a))

void display_sprint_time_numeric(uint8_t * buf, CLOCK_STATE * clock_st)
{

	if (military_time)
	{
		*buf = switch_flash_hours('0' + clock_st->hours_tens_m, ' ');
		buf++;
		*buf = switch_flash_hours('0' + clock_st->hours_ones_m, ' ');
	}
	else
	{
		*buf = switch_flash_hours('0' + clock_st->hours_tens, ' ');
		buf++;
		*buf = switch_flash_hours('0' + clock_st->hours_ones, ' ');
	}
	buf++;
	*buf = ':';
	buf++;
	*buf = switch_flash_minutes('0' + clock_st->minutes_tens, ' ');
	buf++;
	*buf = switch_flash_minutes('0' + clock_st->minutes_ones, ' ');
	buf++;
	*buf = ':';
	buf++;
	*buf = switch_flash_seconds('0' + clock_st->seconds_tens, ' ');
	buf++;
	*buf = switch_flash_seconds('0' + clock_st->seconds_ones, ' ');
	buf++;
	if (!military_time)
	{
		*buf = ' ';
		buf++;
		*buf = switch_flash_hours((clock_st->pm) ? ('P') : ('A'), ' ');
		buf++;
		*buf = switch_flash_hours('M', ' ');
		buf++;
	}
	*buf = 0;
}

void matrix_draw_dec_coded_binary(MATRIX_REGISTER_T * matrix_reg,
		CLOCK_STATE * clock_st)
{
	// Always display military time in this mode
	/*if (military_time)
	 {*/
	*matrix_reg = clock_st->hours_tens_m;
	*matrix_reg <<= MATRIX_ROWS;
	*matrix_reg |= clock_st->hours_ones_m;
	/*}
	 else
	 {
	 *matrix_reg = clock_st->hours_tens;
	 *matrix_reg <<= MATRIX_ROWS;
	 *matrix_reg |= clock_st->hours_ones;
	 }*/
	*matrix_reg <<= MATRIX_ROWS;
	*matrix_reg |= clock_st->minutes_tens;
	*matrix_reg <<= MATRIX_ROWS;
	*matrix_reg |= clock_st->minutes_ones;
	*matrix_reg <<= MATRIX_ROWS;
	*matrix_reg |= clock_st->seconds_tens;
	*matrix_reg <<= MATRIX_ROWS;
	*matrix_reg |= clock_st->seconds_ones;

	if (display_setting_time_mode)
	{
		switch (display_setting_time_mode)
		{
		case SETTING_SECONDS:
		{
			if (display_flash)
			{
				*matrix_reg ^= 0x000000FF;
			}
			else
			{
				*matrix_reg &= 0xFFFFFF00;
			}
			break;
		}
		case SETTING_MINUTES:
		{
			if (display_flash)
			{
				*matrix_reg ^= 0x0000FF00;
			}
			else
			{
				*matrix_reg &= 0xFFFF00FF;
			}
			break;
		}
		case SETTING_HOURS:
		{
			if (display_flash)
			{
				*matrix_reg ^= 0x00FF0000;
			}
			else
			{
				*matrix_reg &= 0xFF00FFFF;
			}
			break;
		}
		case SETTING_MODE:
		{
			if (display_flash)
			{
				*matrix_reg = ~(*matrix_reg);
			}
			else
			{
				*matrix_reg = 0;
			}
			break;
		}
		}
	}
}

void matrix_draw_binary(MATRIX_REGISTER_T * matrix_reg, CLOCK_STATE * clock_st)
{
	*matrix_reg = 0;

	uint8_t i = 0;
	*matrix_reg <<= 1;
	for (i = 0; i < 6; i++)
	{
		*matrix_reg <<= 1;
		*matrix_reg |= ((clock_state.hours_m >> (5 - i)) & 0x01);
		*matrix_reg <<= 1;
		*matrix_reg |= ((clock_state.minutes >> (5 - i)) & 0x01);
		*matrix_reg <<= 1;
		*matrix_reg |= ((clock_state.seconds >> (5 - i)) & 0x01);
		*matrix_reg <<= 1;
	}

	if (display_setting_time_mode)
	{
		switch (display_setting_time_mode)
		{
		case SETTING_SECONDS:
		{
			if (display_flash)
			{
				*matrix_reg ^= 0x00222222;
			}
			else
			{
				*matrix_reg &= 0x00DDDDDD;
			}
			break;
		}
		case SETTING_MINUTES:
		{
			if (display_flash)
			{
				*matrix_reg ^= 0x00444444;
			}
			else
			{
				*matrix_reg &= 0x00AAAAAA;
			}
			break;
		}
		case SETTING_HOURS:
		{
			if (display_flash)
			{
				*matrix_reg ^= 0x00888888;
			}
			else
			{
				*matrix_reg &= 0x00777777;
			}
			break;
		}
		case SETTING_MODE:
		{
			if (display_flash)
			{
				*matrix_reg = ~(*matrix_reg);
				*matrix_reg &= 0x00EEEEEE;
			}
			else
			{
				*matrix_reg = 0;
			}
			break;
		}
		}
	}
}

void matrix_draw_seconds_binary(MATRIX_REGISTER_T * matrix_reg,
		CLOCK_STATE * clock_st)
{
	// TODO: Implement
}

void display_set_setting_time_mode(DISPLAY_SETTING_TIME_MODE setting_mode)
{
	display_setting_time_mode = setting_mode;

	// If setting time is complete, reset the flash state
	if (!display_setting_time_mode)
	{
		display_flash = false;
		display_flash_count = 0;
	}
}

void display_set_military_time(bool mil_time)
{
	military_time = mil_time;
}

void display_set_text_scroll_loop(bool loop)
{
	display_scroll_loop = loop;
}
