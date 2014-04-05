/*
 * binw_matrix.h
 *
 *  Created on: Nov 14, 2013
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

#ifndef BINW_MATRIX_H_
#define BINW_MATRIX_H_

#include <stdint.h>
#include <msp430.h>

#define MATRIX_ROWS 4
#define MATRIX_COLS 6
#define MATRIX_LEDS (MATRIX_ROWS*MATRIX_COLS)

//#define MATRIX_ROW_INVERTED
#define MATRIX_COL_INVERTED

#define MATRIX_IO_MASK 0x3C3F
#define MATRIX_ROW_MASK 0x3C00
#define MATRIX_COL_MASK 0x003F

typedef uint32_t MATRIX_REGISTER_T;

extern MATRIX_REGISTER_T matrix_register;

#define MATRIX_ALL_ON ((((MATRIX_REGISTER_T)1) << MATRIX_LEDS) - 1)
#define MATRIX_ALL_OFF 0

// pins that need to be set to high
extern const uint16_t MATRIX_ROW_PINS[MATRIX_ROWS];

// pins that need to be set to high
extern const uint16_t MATRIX_COL_PINS[MATRIX_COLS];

void matrix_setup(void);
void matrix_update(void);

#endif /* BINW_MATRIX_H_ */
