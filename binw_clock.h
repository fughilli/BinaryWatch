/*
 * binw_clock.h
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

#ifndef BINW_CLOCK_H_
#define BINW_CLOCK_H_

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#define CLOCK_FREQ 32768

#define CLOCK_TICKS_TO_MILLIS(a) (((a)*1000)/CLOCK_FREQ)
#define CLOCK_MILLIS_TO_TICKS(a) (((a)*CLOCK_FREQ)/1000)

#define RTC

#define CLOCK_PRIMARY_PERIOD_TICKS 32768
#define CLOCK_SECONDARY_PERIOD_TICKS 2048

#define CLOCK_PRIMARY_FREQ (CLOCK_FREQ/CLOCK_PRIMARY_PERIOD_TICKS)
#define CLOCK_SECONDARY_FREQ (CLOCK_FREQ/CLOCK_SECONDARY_PERIOD_TICKS)

#define CLOCK_USE_EXT_PRIMARY_ISR
#define CLOCK_USE_EXT_SECONDARY_ISR

typedef uint32_t CLOCK_COUNT_T;

extern CLOCK_COUNT_T clock_count_a, clock_count_b;

#define CLOCK_ELAPSED_TICKS_A()				(clock_count_a + ((CLOCK_COUNT_T)TAR))
#define CLOCK_CLEAR_ELAPSED_TICKS_A()			(clock_count_a = 0)
#define CLOCK_ELAPSED_TICKS_B()				(clock_count_b + ((CLOCK_COUNT_T)TAR))
#define CLOCK_CLEAR_ELAPSED_TICKS_B()			(clock_count_b = 0)

#ifdef RTC
typedef struct
{
	uint8_t hours_m;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;

	uint8_t hours_tens_m;
	uint8_t hours_ones_m;
	uint8_t hours_tens;
	uint8_t hours_ones;
	uint8_t minutes_tens;
	uint8_t minutes_ones;
	uint8_t seconds_tens;
	uint8_t seconds_ones;
	bool pm;
} CLOCK_STATE;

extern CLOCK_STATE clock_state;
extern bool clock_paused;
#endif /* RTC */

extern bool sys_sleeping;

void clock_setup(void);

#ifdef RTC
void clock_inc_hours(void);
void clock_inc_minutes(void);
void clock_inc_seconds(void);

void clock_pause(void);
void clock_resume(void);
#endif /* RTC */

void sys_sleep(void);
void sys_wake(void);

#ifdef CLOCK_USE_EXT_PRIMARY_ISR
extern void clock_primary_isr(void);
#endif /* CLOCK_USE_EXT_PRIMARY_ISR */

#ifdef CLOCK_USE_EXT_SECONDARY_ISR
extern void clock_secondary_isr(void);
#endif /* CLOCK_USE_EXT_SECONDARY_ISR */

#endif /* BINW_CLOCK_H_ */
