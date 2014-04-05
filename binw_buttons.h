/*
 * binw_buttons.h
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
#ifndef BINW_BUTTONS_H_
#define BINW_BUTTONS_H_

#include <msp430.h>
#include <stdint.h>

#define BUTTONS_LONG_PRESS_TIME ((CLOCK_COUNT_T)2000)
#define BUTTONS_DEBOUNCE_TIME ((CLOCK_COUNT_T)20)

#define DETECT_RELEASE

#define BUTTONS_NUM 2

#define BUTTONS_IO_MASK 0x0300
extern const uint16_t BUTTONS_PINS[BUTTONS_NUM];

typedef uint32_t BUTTON_STATE_T;

extern BUTTON_STATE_T BUTTONS_STATES;

#define BUTTONS_PRESS_FALLING
//#define BUTTONS_PRESS_RISING

extern void (*BUTTONS_CALLBACKS[BUTTONS_NUM][2])(void);

extern void buttons_setup(void);
extern void buttons_tick(void);

#endif /* BINW_BUTTONS_H_ */
