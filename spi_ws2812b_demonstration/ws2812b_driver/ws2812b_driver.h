/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef WS2812B_DRIVER_H__
#define WS2812B_DRIVER_H__

#include "nrf_drv_spi.h"
#include "stdlib.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "bsp.h"

// #define DRIVE_MODE_4M
#define DRIVE_MODE_8M

#ifdef DRIVE_MODE_8M
#define PATTERN_0 (0xe0)			// Bit pattern for data "0"
#define PATTERN_1 (0xfc)      // Bit pattern for data "1"
#define PATTERN_0_EOS (0x0e)  // BIt pattern for data "0" for end of sector
#define PATTERN_1_EOS (0xfc)  // BIt pattern for data "1" for end of sector
#define	LED_SECTOR_SIZE	(10)	// number of LEDs which is sent in one SPIM transfer (<=10)
#define	BUF_SIZE_PER_LED	(24)	// buffer size for each LED (8bit * 8 * 3 )
#endif

#ifdef DRIVE_MODE_4M
#define PATTERN_0 (0x0C)			// Bit pattern for data "0"
#define PATTERN_1 (0x0e)      // Bit pattern for data "1"
#define PATTERN_0_EOS (0x06)  // BIt pattern for data "0" for end of sector
#define PATTERN_1_EOS (0x0e)  // BIt pattern for data "1" for end of sector
#define	LED_SECTOR_SIZE	(21)	// number of LEDs which is sent in one SPIM transfer (<=21)
#define	BUF_SIZE_PER_LED	(12)	// buffer size for each LED (4bit * 8 * 3 )
#endif

#define NUM_SPI_BUS (3)


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
		uint8_t sector_size; // buffer size per one SPI transaction
} spi_buffer_t;

typedef struct
{
		nrf_drv_spi_t spi;
		volatile bool * transfer_completed;
		nrf_drv_spi_xfer_desc_t xfer_desc;
		volatile spi_buffer_t spi_buffer;
		volatile uint32_t rest;
} ws2812b_driver_spi_t;

void alloc_spi_buffer(spi_buffer_t * spi_buffer, uint16_t num_leds);

void sectorize_spi_buffer(spi_buffer_t spi_buffer);

void set_blank(rgb_led_t * rgb_led, uint16_t num_leds);

void set_buff(rgb_led_t* rgb_led, spi_buffer_t spi_buffer);

void form_spi_sector(spi_buffer_t spi_buffer);

void spi_master_init(ws2812b_driver_spi_t const * p_instance);

void ws2812b_driver_spi_init(uint8_t id,ws2812b_driver_spi_t *spi);

void ws2812b_driver_xfer(rgb_led_t * led_array, spi_buffer_t spi_buffer, ws2812b_driver_spi_t spi);

void spi0_event_handler(nrf_drv_spi_evt_t const * event);
void spi1_event_handler(nrf_drv_spi_evt_t const * event);
void spi2_event_handler(nrf_drv_spi_evt_t const * event);

void ws2812b_driver_current_cap(rgb_led_t * led_array, uint16_t num_leds, uint32_t limit);

void ws2812b_driver_dim(rgb_led_t * led_array, uint16_t num_leds, float dim );

uint32_t ws2812b_driver_calc_current(rgb_led_t * led_array, uint16_t num_leds);


#endif // WS2812B_DRIVER_H__
