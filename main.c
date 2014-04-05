/*
 * main.c
 *
 *  Created on: Nov 15, 2013
 *      Author: Kevin
 *
 *
 *
 * Copyright (C) 2012-2014  Kevin Balke
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#include <msp430.h> 
#include "binw_matrix.h"
#include "font4x4.h"
#include "binw_buttons.h"
#include "binw_clock.h"
#include "binw_display.h"

//#define BUTTON_TEST

void lbutton_short(void);
void lbutton_long(void);
void rbutton_short(void);
void rbutton_long(void);
void clock_primary_isr(void);
void clock_secondary_isr(void);
void display_timeout_callback(void);

const uint16_t MATRIX_ROW_PINS[MATRIX_ROWS] =
{ 0x0400, 0x0800, 0x1000, 0x2000 };

const uint16_t MATRIX_COL_PINS[MATRIX_COLS] =
{ 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020 };

const uint16_t BUTTONS_PINS[BUTTONS_NUM] =
{ 0x0100, 0x0200 };

void (*BUTTONS_CALLBACKS[BUTTONS_NUM][2])(void) =
{
	{	rbutton_short, rbutton_long},
	{	lbutton_short, lbutton_long}
}
;

bool setting_mode;
DISPLAY_MODE selected_mode;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	if (CALBC1_16MHZ == 0xFF || CALDCO_16MHZ == 0xFF) //check if calibration values are empty (incorrect)
	{
		while (1)
		{
			; //trap CPU if calibration is incorrect
		}
	}

	BCSCTL1 = CALBC1_16MHZ; //set CPU to run at 16 MHz
	DCOCTL = CALDCO_16MHZ;

	P1DIR = P2DIR = 0xFF;		// Stop floating inputs

	clock_setup();
	matrix_setup();
	buttons_setup();
	display_setup();

	_BIS_SR(GIE);

#ifndef BUTTON_TEST
	setting_mode = false;
	selected_mode = DISPLAY_DEC_CODED_BINARY;

	display_set_military_time(false);
	display_show(DISPLAY_DEC_CODED_BINARY);
#endif

	while (1)
	{
#ifndef BUTTON_TEST
		display_draw_tick();
#else
		//__delay_cycles(100);
		matrix_update();
#endif
	}

	return 0;
}

#ifndef BUTTON_TEST
void lbutton_short(void)
{
	if (sys_sleeping)
	{
		sys_wake();
		display_show(selected_mode);
	}
	else
	{
		display_reset_timeout();

		if (clock_paused)
		{
			switch (display_setting_time_mode)
			{
			case SETTING_SECONDS:
				display_set_setting_time_mode(SETTING_MINUTES);
				break;
			case SETTING_MINUTES:
				display_set_setting_time_mode(SETTING_HOURS);
				break;
			case SETTING_HOURS:
				display_set_setting_time_mode(SETTING_SECONDS);
				break;
			}
		}

		if (setting_mode)
		{
			switch (display_mode)
			{
			case DISPLAY_NUMERIC:
			{
				selected_mode = DISPLAY_DEC_CODED_BINARY;
				display_show(DISPLAY_DEC_CODED_BINARY);
				break;
			}
			case DISPLAY_DEC_CODED_BINARY:
			{
				selected_mode = DISPLAY_BINARY;
				display_show(DISPLAY_BINARY);
				break;
			}
			case DISPLAY_BINARY:
			{
				selected_mode = DISPLAY_NUMERIC;
				display_show(DISPLAY_NUMERIC);
				break;
			}
			}
		}
	}
}

void lbutton_long(void)
{
	// If the mode is not being set
	if (!setting_mode)
	{
		display_reset_timeout();

		clock_paused = !clock_paused;
		if (clock_paused)
		{
			// No timeout
			display_set_show_delay(0);

			display_set_text_scroll_loop(true);

			display_set_setting_time_mode(SETTING_SECONDS);
		}
		else
		{
			// Default timeout
			display_set_show_delay(DISPLAY_DEFAULT_SHOW_DELAY);

			display_set_text_scroll_loop(false);

			display_set_setting_time_mode(SETTING_DONE);
		}
	}
}

void rbutton_short(void)
{
	if (sys_sleeping)
	{
		sys_wake();
		display_show(selected_mode);
	}
	else
	{
		display_reset_timeout();

		if (clock_paused)
		{
			switch (display_setting_time_mode)
			{
			case SETTING_SECONDS:
				clock_inc_seconds();
				break;
			case SETTING_MINUTES:
				clock_inc_minutes();
				break;
			case SETTING_HOURS:
				clock_inc_hours();
				break;
			}
		}
	}
}

void rbutton_long(void)
{
	// If the time is not being set
	if (!clock_paused)
	{
		display_reset_timeout();

		if (!setting_mode)
		{
			setting_mode = true;

			// No timeout
			display_set_show_delay(0);

			display_set_text_scroll_loop(true);

			// Flash the whole display to indicate changing modes
			display_set_setting_time_mode(SETTING_MODE);
		}
		else
		{
			setting_mode = false;

			// Default timeout
			display_set_show_delay(DISPLAY_DEFAULT_SHOW_DELAY);

			display_set_text_scroll_loop(false);

			// Don't flash the display
			display_set_setting_time_mode(SETTING_DONE);
		}
	}
}

void clock_secondary_isr(void)
{
	buttons_tick();
	display_tick();
}

void clock_primary_isr(void)
{
	display_clock_tick();
}

void display_timeout_callback(void)
{
	sys_sleep();
}
#else
void lbutton_short(void)
{
	matrix_register <<= 1;
}

void lbutton_long(void)
{
	matrix_register = 0xFFFFFFFF;
}

void rbutton_short(void)
{
	matrix_register |= 1;
}

void rbutton_long(void)
{
	matrix_register = 0;
}

void clock_secondary_isr(void)
{
	buttons_tick();
}

void clock_primary_isr(void)
{
	;
}

void display_timeout_callback(void)
{
	;
}
#endif

#pragma vector = unused_interrupts
__interrupt void ISR_TRAP(void)
{
	while (1)
		; // Do nothing
}
