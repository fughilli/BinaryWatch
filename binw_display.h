/*
 * binw_display.h
 *
 *  Created on: Dec 16, 2013
 *      Author: Kevin
 *
 *      (c) Kevin Balke 2013. All rights reserved.
 */

#ifndef BINW_DISPLAY_H_
#define BINW_DISPLAY_H_

#include "binw_clock.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	PRINT_WHITESPACE_START,
	PRINT_TEXT,
	PRINT_WHITESPACE_END,
	PRINT_IDLE
} PRINT_STATE;

typedef enum
{
	DISPLAY_OFF,
	DISPLAY_DEC_CODED_BINARY,
	DISPLAY_BINARY,
	DISPLAY_SECONDS_BINARY,
	DISPLAY_NUMERIC
} DISPLAY_MODE;

typedef enum
{
	SETTING_DONE,
	SETTING_SECONDS,
	SETTING_MINUTES,
	SETTING_HOURS,
	SETTING_MODE
} DISPLAY_SETTING_TIME_MODE;

extern PRINT_STATE print_state;
extern DISPLAY_MODE display_mode;
extern DISPLAY_SETTING_TIME_MODE display_setting_time_mode;

#define DISPLAY_DEFAULT_TEXT_SCROLL_DELAY (CLOCK_SECONDARY_FREQ/8) // 1/8th second
#define DISPLAY_DEFAULT_SHOW_DELAY (CLOCK_SECONDARY_FREQ*15) // 15 seconds
#define DISPLAY_DEFAULT_FLASH_DELAY (CLOCK_SECONDARY_FREQ/4) // 1/4th second

#define DISPLAY_TIMEOUT_CALLBACK

#ifdef DISPLAY_TIMEOUT_CALLBACK
extern void display_timeout_callback(void);
#endif /* DISPLAY_TIMEOUT_CALLBACK */

void display_print(uint8_t * str);

void display_draw_tick(void);
void display_clock_tick(void);
void display_tick(void);

void display_setup(void);

void display_set_text_scroll_delay(uint16_t delay);
void display_set_setting_time_flash_delay(uint16_t delay);
void display_set_show_delay(uint16_t delay);

void display_reset_timeout(void);

void display_show(DISPLAY_MODE disp_mode);

void display_set_setting_time_mode(DISPLAY_SETTING_TIME_MODE setting_mode);

void display_set_military_time(bool mil_time);

void display_set_text_scroll_loop(bool loop);

#endif /* BINW_DISPLAY_H_ */
