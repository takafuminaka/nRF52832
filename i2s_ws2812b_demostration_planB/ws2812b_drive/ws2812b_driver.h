/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef WS2812B_DRIVER_H__
#define WS2812B_DRIVER_H__

#include <stdio.h>
#include "nrf_drv_i2s.h"
#include "stdlib.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "bsp.h"

typedef struct
{
    uint8_t   green; // Brightness of green (0 to 255)
    uint8_t   red;   // Brightness of red   (0 to 255)
    uint8_t   blue;  // Brightness of blue  (0 to 255)
} rgb_led_t;

void ws2812b_drive_set_blank(rgb_led_t * rgb_led, uint16_t num_leds);

void ws2812b_drive_current_cap(rgb_led_t * led_array, uint16_t num_leds, uint32_t limit);

void ws2812b_drive_dim(rgb_led_t * led_array, uint16_t num_leds, float dim );

uint32_t ws2812b_drive_calc_current(rgb_led_t * led_array, uint16_t num_leds);

#endif // WS2812B_DRIVER_H__
