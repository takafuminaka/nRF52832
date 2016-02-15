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


#define PATTERN_0 ((uint8_t)0xe0)			// Bit pattern for data "0" is "HLLL".
#define PATTERN_1 ((uint8_t)0xfc)      // Bit pattern for data "1" is "HHHL".
#define	BUF_SIZE_PER_LED	(12)	// buffer size for each LED (8bit * 4 * 3 )

typedef struct
{
    uint8_t   green; // Brightness of green (0 to 255)
    uint8_t   red;   // Brightness of red   (0 to 255)
    uint8_t   blue;  // Brightness of blue  (0 to 255)
} rgb_led_t;

typedef struct
{
		uint8_t* buff; // data buffer
		uint16_t length; // buffer size
		uint32_t* tx_buff; // data buffer
		uint32_t* rx_buff; // data buffer
} i2s_buffer_t;

typedef struct
{
		nrf_drv_i2s_config_t i2s;
		volatile bool * transfer_completed;
		volatile i2s_buffer_t i2s_buffer;
		volatile uint32_t rest;
} ws2812b_driver_i2s_t;

void alloc_i2s_buffer(i2s_buffer_t * i2s_buffer, uint16_t num_leds);

void sectorize_i2s_buffer(i2s_buffer_t i2s_buffer);

void set_blank(rgb_led_t * rgb_led, uint16_t num_leds);

void set_buff(rgb_led_t* rgb_led, i2s_buffer_t i2s_buffer);

void form_i2s_sector(i2s_buffer_t i2s_buffer);

void i2s_master_init(ws2812b_driver_i2s_t const * p_instance);

void ws2812b_driver_i2s_init(ws2812b_driver_i2s_t *i2s);

void ws2812b_driver_xfer(rgb_led_t * led_array, i2s_buffer_t i2s_buffer, ws2812b_driver_i2s_t i2s);

void i2s_event_handler(uint32_t const * p_data_received,
                         uint32_t       * p_data_to_send,
                         uint16_t         number_of_words);

void ws2812b_driver_current_cap(rgb_led_t * led_array, uint16_t num_leds, uint32_t limit);

void ws2812b_driver_dim(rgb_led_t * led_array, uint16_t num_leds, float dim );

uint32_t ws2812b_driver_calc_current(rgb_led_t * led_array, uint16_t num_leds);


#endif // WS2812B_DRIVER_H__
