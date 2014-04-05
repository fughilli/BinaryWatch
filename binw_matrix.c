/*
 * binw_matrix.c
 *
 *  Created on: Nov 14, 2013
 *      Author: Kevin
 *
 *      (c) Kevin Balke 2013. All rights reserved.
 */

#include "binw_matrix.h"

uint8_t led_row_index;
uint8_t led_col_index;
uint8_t prev_led_row_index;
uint8_t prev_led_col_index;

MATRIX_REGISTER_T matrix_register_buffer;
MATRIX_REGISTER_T matrix_register;

void matrix_setup(void)
{
	led_row_index = 0;
	led_col_index = 0;
	prev_led_row_index = 0;
	prev_led_col_index = 0;

	matrix_register_buffer = 0;
	matrix_register = 0;

#ifdef MATRIX_ROW_INVERTED
	P1OUT |= MATRIX_ROW_MASK;
	P2OUT |= (MATRIX_ROW_MASK >> 8);
	P1OUT &= ~MATRIX_COL_MASK;
	P2OUT &= ~(MATRIX_COL_MASK >> 8;
#endif
#ifdef MATRIX_COL_INVERTED
	P1OUT |= MATRIX_COL_MASK;
	P2OUT |= (MATRIX_COL_MASK >> 8);
	P1OUT &= ~MATRIX_ROW_MASK;
	P2OUT &= ~(MATRIX_ROW_MASK >> 8);
#endif
	P1DIR |= MATRIX_IO_MASK;
	P2DIR |= (MATRIX_IO_MASK >> 8);
}

void matrix_update(void)
{
#ifdef MATRIX_ROW_INVERTED
	P1OUT |= (MATRIX_ROW_PINS[prev_led_row_index]);
	P2OUT |= (MATRIX_ROW_PINS[prev_led_row_index] >> 8);
	P1OUT &= ~(MATRIX_COL_MASK);
	P2OUT &= ~((MATRIX_COL_MASK) >> 8);
	if (matrix_register_buffer & 1)
	{
		P1OUT |= (MATRIX_COL_PINS[led_col_index]);
		P2OUT |= ((MATRIX_COL_PINS[led_col_index]) >> 8);
	}
	P1OUT &= ~(MATRIX_ROW_PINS[led_row_index]);
	P2OUT &= ~(MATRIX_ROW_PINS[led_row_index] >> 8);
#endif
#ifdef MATRIX_COL_INVERTED
	P1OUT |= (MATRIX_COL_PINS[prev_led_col_index]);
	P2OUT |= (MATRIX_COL_PINS[prev_led_col_index] >> 8);
	P1OUT &= ~(MATRIX_ROW_MASK);
	P2OUT &= ~((MATRIX_ROW_MASK) >> 8);
	if (matrix_register_buffer & 1)
	{
		P1OUT |= (MATRIX_ROW_PINS[led_row_index]);
		P2OUT |= ((MATRIX_ROW_PINS[led_row_index]) >> 8);
	}
	P1OUT &= ~(MATRIX_COL_PINS[led_col_index]);
	P2OUT &= ~(MATRIX_COL_PINS[led_col_index] >> 8);
#endif

	matrix_register_buffer >>= 1;

	prev_led_row_index = led_row_index;
	prev_led_col_index = led_col_index;

	led_row_index++;
	if (led_row_index == MATRIX_ROWS)
	{
		led_row_index = 0;
		led_col_index++;
		if (led_col_index == MATRIX_COLS)
		{
			led_col_index = 0;
			matrix_register_buffer = matrix_register;
		}
	}
}
