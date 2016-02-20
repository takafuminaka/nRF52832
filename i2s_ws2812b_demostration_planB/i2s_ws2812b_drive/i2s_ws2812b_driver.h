/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef I2S_WS2812B_DRIVER_H__
#define I2S_WS2812B_DRIVER_H__

#include <stdio.h>
#include "nrf_drv_i2s.h"
#include "stdlib.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "bsp.h"

#include "ws2812b_drive.h"

#define I2S_WS2812B_DRIVE_PATTERN_0 ((uint8_t)0x08)			// Bit pattern for data "0" is "HLLL".
#define I2S_WS2812B_DRIVE_PATTERN_1 ((uint8_t)0x0e)      // Bit pattern for data "1" is "HHHL".
#define	I2S_WS2812B_DRIVE_BUF_SIZE_PER_LED	(12)	// buffer size for each LED (8bit * 4 * 3 )

void i2s_ws2812b_drive_set_buff(rgb_led_t* rgb_led, xfer_buffer_t xfer_buffer);
#endif // I2S_WS2812B_DRIVER_H__
