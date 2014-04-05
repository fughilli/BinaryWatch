/*
 * binw_buttons.h
 *
 *  Created on: Nov 15, 2013
 *      Author: Kevin
 *
 *      (c) Kevin Balke 2013. All rights reserved.
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
