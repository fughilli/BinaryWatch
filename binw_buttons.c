/*
 * binw_buttons.c
 *
 *  Created on: Nov 15, 2013
 *      Author: Kevin
 *
 *      (c) Kevin Balke 2013. All rights reserved.
 */

#include "binw_buttons.h"
#include "binw_clock.h"

BUTTON_STATE_T BUTTONS_STATES;
BUTTON_STATE_T BUTTONS_LP_STATES;

CLOCK_COUNT_T BUTTONS_START_TIMES[BUTTONS_NUM];

bool buttons_ticking;
bool buttons_interrupting;

void buttons_setup(void)
{
	P1DIR &= ~BUTTONS_IO_MASK;
	P2DIR &= ~(BUTTONS_IO_MASK >> 8);
#ifdef BUTTONS_PRESS_FALLING
	P1IES |= BUTTONS_IO_MASK;
	P2IES |= (BUTTONS_IO_MASK >> 8);
#endif
#ifdef BUTTONS_PRESS_RISING
	P1IES &= ~BUTTONS_IO_MASK;
	P2IES &= ~(BUTTONS_IO_MASK >> 8);
#endif
	P1IE |= BUTTONS_IO_MASK;
	P2IE |= (BUTTONS_IO_MASK >> 8);

	P1IFG = 0;
	P2IFG = 0;

	BUTTONS_STATES = 0;
	BUTTONS_LP_STATES = 0;

	buttons_ticking = false;
	buttons_interrupting = false;

	uint8_t i = 0;
	for (; i < BUTTONS_NUM; i++)
	{
		BUTTONS_START_TIMES[i] = 0;
	}
}

#pragma vector = PORT1_VECTOR, PORT2_VECTOR
__interrupt void port_interrupt(void)
{
	buttons_interrupting = true;

	// Disable I/O interrupts
	P1IE = P2IE = 0;

	if (!buttons_ticking)
	{
		uint8_t buttons_index = 0;
		CLOCK_COUNT_T buttons_elapsed_time = 0;

		// Iterate through all of the registered buttons
		for (; buttons_index < BUTTONS_NUM; buttons_index++)
		{
			// If the interrupt was triggered by the button at interrupt_index
			if ((P1IFG & (BUTTONS_PINS[buttons_index]))
					|| (P2IFG & (BUTTONS_PINS[buttons_index] >> 8)))
			{
				// If the recorded state for the current button is "pressed" (the button is now released)
				if ((BUTTONS_STATES >> buttons_index) & 1)
				{
					buttons_elapsed_time = CLOCK_ELAPSED_TICKS_A()
							- BUTTONS_START_TIMES[buttons_index];

					if (buttons_elapsed_time
							>= CLOCK_MILLIS_TO_TICKS(BUTTONS_DEBOUNCE_TIME))
					{
						if ((BUTTONS_LP_STATES >> buttons_index) & 1)
						{
							BUTTONS_LP_STATES &= ~(((BUTTON_STATE_T) 1)
									<< buttons_index);
						}
						else
						{
							BUTTONS_CALLBACKS[buttons_index][0]();

							_BIC_SR(LPM3_EXIT);
						}
					}

					BUTTONS_STATES &= ~(((BUTTON_STATE_T) 1) << buttons_index);
#ifdef BUTTONS_PRESS_FALLING
					P1IES |= BUTTONS_PINS[buttons_index];
					P2IES |= (BUTTONS_PINS[buttons_index] >> 8);
#endif
#ifdef BUTTONS_PRESS_RISING
					P1IES &= ~BUTTONS_PINS[buttons_index];
					P2IES &= ~(BUTTONS_PINS[buttons_index] >> 8);
#endif
					break;
				}
				// If the recorded state for the current button is "released" (the button is now pressed)
				else
				{
					// If no buttons are recorded as being pressed (elapsed time is not being monitored...)
					if (!BUTTONS_STATES)
					{
						// Clear the elapsed time counter
						CLOCK_CLEAR_ELAPSED_TICKS_A();
						// Record the start time
						BUTTONS_START_TIMES[buttons_index] = CLOCK_ELAPSED_TICKS_A();
					}
					// Buttons are pressed, use the current elapsed time
					else
					{
						BUTTONS_START_TIMES[buttons_index] =
								CLOCK_ELAPSED_TICKS_A();
					}
					BUTTONS_STATES |= (((BUTTON_STATE_T) 1) << buttons_index);
#ifdef BUTTONS_PRESS_FALLING
					P1IES &= ~BUTTONS_PINS[buttons_index];
					P2IES &= ~(BUTTONS_PINS[buttons_index] >> 8);
#endif
#ifdef BUTTONS_PRESS_RISING
					P1IES |= BUTTONS_PINS[buttons_index];
					P2IES |= (BUTTONS_PINS[buttons_index] >> 8);
#endif
					break;
				}
			}
		}
	}

	// Enable I/O interrupts
	P1IE |= BUTTONS_IO_MASK;
	P2IE |= (BUTTONS_IO_MASK >> 8);

	// Clear the interrupt flag
	P1IFG &= ~BUTTONS_IO_MASK;
	P2IFG &= ~(BUTTONS_IO_MASK >> 8);

	buttons_interrupting = false;
}

void buttons_tick(void)
{
	if (!buttons_interrupting)
	{
		buttons_ticking = true;

		uint8_t buttons_index = 0;
		CLOCK_COUNT_T buttons_elapsed_time = 0;

		for (; buttons_index < BUTTONS_NUM; buttons_index++)
		{
			if (((BUTTONS_STATES >> buttons_index) & 1)
					&& !((BUTTONS_LP_STATES >> buttons_index) & 1))
			{
				/*#ifdef BUTTONS_PRESS_FALLING
				 if ((P1IN & (BUTTONS_PINS[buttons_index]))
				 || (P2IN & (BUTTONS_PINS[buttons_index] >> 8)))
				 #endif
				 #ifdef BUTTONS_PRESS_RISING
				 if(!((P1IN & (BUTTONS_PINS[buttons_index]))
				 || (P2IN & (BUTTONS_PINS[buttons_index] >> 8))))
				 #endif
				 {*/
				buttons_elapsed_time = CLOCK_ELAPSED_TICKS_A()
						- BUTTONS_START_TIMES[buttons_index];

				if (buttons_elapsed_time
						>= CLOCK_MILLIS_TO_TICKS(BUTTONS_LONG_PRESS_TIME))
				{
					BUTTONS_CALLBACKS[buttons_index][1]();

					BUTTONS_LP_STATES |=
							(((BUTTON_STATE_T) 1) << buttons_index);
				}
				/*else if (buttons_elapsed_time
				 >= CLOCK_MILLIS_TO_TICKS(BUTTONS_DEBOUNCE_TIME))
				 {
				 BUTTONS_CALLBACKS[buttons_index][0]();
				 }*/

				//}
			}
		}

		buttons_ticking = false;
	}
}
