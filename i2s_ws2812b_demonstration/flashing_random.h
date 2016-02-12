/* Copyright (c) 2015 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef FLASHING_RANDOM_H__
#define FLASHING_RANDOM_H__

#include	"project.h"
#include "ws2812b_driver.h"
#include  "stdlib.h"

void flashing_random_init(void);
void flashing_random(rgb_led_t * led_array, uint32_t rap_time);

#endif // FLASHING_RANDOM_H__
