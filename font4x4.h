/*
 * font4x4.h
 *
 *  Created on: Nov 15, 2013
 *      Author: Kevin
 *
 *      (c) Kevin Balke 2013. All rights reserved.
 */

#ifndef FONT4X4_H_
#define FONT4X4_H_

#include <stdint.h>

#define CHAR_SIZE_WIDTH (4u)
#define CHAR_SIZE_HEIGHT (4u)

#define NUM_FONT_CHARS (95u)
#define FIRST_CHAR (32u)
#define LAST_CHAR (FIRST_CHAR + NUM_FONT_CHARS)

typedef uint16_t FONT_CHAR_DATA_T;

extern const FONT_CHAR_DATA_T font[NUM_FONT_CHARS];

#define CHAR_TO_FONT_BITS(ch) (font[((ch) - FIRST_CHAR)])

#endif /* FONT4X4_H_ */
