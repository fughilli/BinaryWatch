/*
 * font4x4.h
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
