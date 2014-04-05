/*
 * binw_matrix.h
 *
 *  Created on: Nov 14, 2013
 *      Author: Kevin
 *
 *      (c) Kevin Balke 2013. All rights reserved.
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
