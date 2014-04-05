/*
 * binw_clock.c
 *
 *  Created on: Nov 15, 2013
 *      Author: Kevin
 *
 *      (c) Kevin Balke 2013. All rights reserved.
 */

#include "binw_clock.h"
#include <stdbool.h>

uint32_t clock_count_a, clock_count_b;
bool sys_sleeping;

#ifdef RTC
CLOCK_STATE clock_state;
bool clock_paused;
#endif

void clock_setup(void)
{
#ifdef RTC
	clock_state.hours_m = clock_state.hours_tens_m =
			clock_state.hours_ones_m = clock_state.minutes =
					clock_state.seconds = clock_state.minutes_ones =
									clock_state.minutes_tens =
											clock_state.seconds_ones =
													clock_state.seconds_tens =
															0;
	clock_state.hours = 12;
	clock_state.hours_tens = 1;
	clock_state.hours_ones = 2;
	clock_state.pm = false;

	clock_paused = false;
#endif

	clock_count_a = 0;
	clock_count_b = 0;

	// Select the external crystal function of XIN and XOUT
	P2SEL |= 0xC0;
	// Set 12.5 pF capacitors on XIN and XOUT
	BCSCTL3 |= XCAP_3;
	// Clock ACLK at LFXT1CLK / 1
	BCSCTL3 &= ~(LFXT1S_3);
	BCSCTL1 &= ~(DIVA_3 | XTS);

	// Clock TACLK from ACLK / 1
	TACTL &= ~(TASSEL_3 | ID_3 | MC_3);
	TACTL |= TASSEL_1 | ID_0 | MC_1 | TACLR;

	// Setup the timer periods
	TACCR0 = CLOCK_PRIMARY_PERIOD_TICKS - 1;
	TACCTL0 |= CCIE;
#ifdef CLOCK_SECONDARY_PERIOD_TICKS
	TACCR1 = CLOCK_SECONDARY_PERIOD_TICKS - 1;
	TACCTL1 |= CCIE;
#endif

}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void clock_isr(void)
{
	clock_count_a += CLOCK_PRIMARY_PERIOD_TICKS;
	clock_count_b += CLOCK_PRIMARY_PERIOD_TICKS;

#ifdef RTC
	if (!clock_paused)
	{
		clock_inc_seconds();
		if (clock_state.seconds == 0)
		{
			clock_inc_minutes();
			if (clock_state.minutes == 0)
			{
				clock_inc_hours();
			}
		}
	}
#endif

#ifdef CLOCK_USE_EXT_PRIMARY_ISR
	if (!sys_sleeping)
		clock_primary_isr();
#endif
}

#ifdef RTC
void clock_inc_hours(void)
{
	// Adjust the whole-number hours
	clock_state.hours_m++;

	if (clock_state.hours_m == 24)
	{
		clock_state.hours_m = 0;
	}

	if (clock_state.hours_m == 0)
	{
		// 12:00 - 12:59 AM
		clock_state.hours = 12;
		clock_state.pm = false;

	}
	else if (clock_state.hours_m == 12)
	{
		// 12:00 - 12:59 PM
		clock_state.hours = 12;
		clock_state.pm = true;
	}
	else if (clock_state.hours_m > 12)
	{
		// 1:00 - 11:59 PM
		clock_state.hours = clock_state.hours_m - 12;
		clock_state.pm = true;
	}
	else
	{
		// 1:00 - 11:59 AM
		clock_state.hours = clock_state.hours_m;
		clock_state.pm = false;
	}

	// Adjust the tens-ones hours
	// If it is 12:00
	if (clock_state.hours_tens == 1 && clock_state.hours_ones == 2)
	{
		// Make it 1:00
		clock_state.hours_ones = 1;
		clock_state.hours_tens = 0;
	}
	// Otherwise
	else
	{
		// Increment ones place
		clock_state.hours_ones++;
		// If that made it ten
		if (clock_state.hours_ones == 10)
		{
			// Make it zero, and increment the tens place
			clock_state.hours_ones = 0;
			clock_state.hours_tens++;

		}
	}

	clock_state.hours_ones_m++;
	if (clock_state.hours_ones_m == 10)
	{
		clock_state.hours_ones_m = 0;
		clock_state.hours_tens_m++;

	}
	if (clock_state.hours_tens_m == 2 && clock_state.hours_ones_m == 4)
	{
		clock_state.hours_tens_m = 0;
		clock_state.hours_ones_m = 0;
	}
}

void clock_inc_minutes(void)
{
	clock_state.minutes++;
	if (clock_state.minutes == 60)
	{
		clock_state.minutes = 0;
	}

	clock_state.minutes_ones++;
	if (clock_state.minutes_ones == 10)
	{
		clock_state.minutes_ones = 0;
		clock_state.minutes_tens++;
		if (clock_state.minutes_tens == 6)
		{
			clock_state.minutes_tens = 0;
		}
	}
}

void clock_inc_seconds(void)
{
	clock_state.seconds++;
	if (clock_state.seconds == 60)
	{
		clock_state.seconds = 0;
	}

	clock_state.seconds_ones++;
	if (clock_state.seconds_ones == 10)
	{
		clock_state.seconds_ones = 0;
		clock_state.seconds_tens++;
		if (clock_state.seconds_tens == 6)
		{
			clock_state.seconds_tens = 0;
		}
	}
}
#endif /* RTC */

#ifndef CLOCK_SECONDARY_PERIOD_TICKS

#pragma vector = TIMER0_A1_VECTOR
__interrupt void clock_isr_others(void)
{
	; // Do nothing
}

#else

#pragma vector = TIMER0_A1_VECTOR
__interrupt void clock_isr_others(void)
{
	if (TAIV & 0x0002)
	{
		TACCR1 += CLOCK_SECONDARY_PERIOD_TICKS;

		if (TACCR1 > TACCR0)
		{
			TACCR1 -= CLOCK_PRIMARY_PERIOD_TICKS;
		}

#ifdef CLOCK_USE_EXT_SECONDARY_ISR
		clock_secondary_isr();
#endif
	}
}

#endif

#ifdef RTC
void clock_pause(void)
{
	clock_paused = true;
}

void clock_resume(void)
{
	clock_paused = false;
}
#endif /* RTC */

void sys_sleep(void)
{
#ifdef CLOCK_SECONDARY_PERIOD_TICKS
	TACCTL1 &= ~CCIE;

	TACCR1 = 0;
#endif

	sys_sleeping = true;

	_BIS_SR(LPM3_bits + GIE);
}

void sys_wake(void)
{
#ifdef CLOCK_SECONDARY_PERIOD_TICKS
	TACCR1 = CLOCK_SECONDARY_PERIOD_TICKS - 1;

	TACCTL1 |= CCIE;
#endif

	sys_sleeping = false;

	// Doesn't work! Need to use _BIC_SR_IRQ() :(
	_BIC_SR(LPM3_bits);
}
