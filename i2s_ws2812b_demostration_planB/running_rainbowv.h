/* Copyright (c) 2015 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef RUNNING_RAINBOWV_H__
#define RUNNING_RAINBOWV_H__

#include	"project.h"
#include	"ws2812b_drive.h"
#include 	"stdlib.h"


void running_rainbowv_init(uint16_t num_leds);

void running_rainbowv_uninit(void);

void running_rainbowv(rgb_led_t * led_array, uint32_t rap_time);

#endif // RUNNING_RAINBOWV_H__



