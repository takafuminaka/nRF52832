/* Copyright (c) 2015 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
* @defgroup spi_ws2812b_main main.c
* @{
* @ingroup spi_ws2812b
*
* @brief WS2812B drive by SPI Master sample main file.
*
* This file contains the source code for a sample application using SPI.
*
*/

#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "nrf_drv_spi.h"
#include "bsp.h"
#include "app_timer.h"
#include "nordic_common.h"
#include  "stdlib.h"

#include "ws2812b_driver.h"
#include "project.h"

#include	"flashing_random.h"
#include	"running_rainbow.h"
#include	"running_rainbowv.h"


const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;

typedef struct
{
	void (*function_init)();
	void (*function_update)(rgb_led_t * led_array_out, uint32_t rap_time);
	uint16_t wait_ms;   // wait time (ms)
	int32_t demo_period;	// demo time (ms)
	uint16_t process_time;	// process time for each step (ms)
} demo_list_t;

const static demo_list_t demo_list[] = {
	{ &running_rainbowv_init, &running_rainbowv, 20 ,30000,8},
	{ &running_rainbow_init, &running_rainbow, 20 ,30000,8},
	{ &flashing_random_init, &flashing_random, 20,30000,8},
};

const static int8_t size_of_list = sizeof(demo_list)/sizeof(demo_list[0]);

static ws2812b_driver_spi_t spi[NUM_SPI_BUS] = {
	{
		.spi = NRF_DRV_SPI_INSTANCE(0)
	},
#if ( NUM_SPI_BUS > 1 ) 
	{
		.spi = NRF_DRV_SPI_INSTANCE(1)
	},
#endif
#if ( NUM_SPI_BUS > 2 ) 
	{
		.spi = NRF_DRV_SPI_INSTANCE(2)
	},
#endif
};


/** @brief Function for main application entry.
 */
int main(void)
{
		spi_buffer_t spi_buffer[NUM_SPI_BUS];
	
		rgb_led_t led_array[NUM_LEDS];
	
		uint32_t current_limit;
		float dim;
	
		// Configure on-board LED-pins as outputs.
		LEDS_CONFIGURE(LEDS_MASK);

		// Initialize spi I/F
		for(uint8_t i=0;i<NUM_SPI_BUS;i++) {
			ws2812b_driver_spi_init(i, &spi[i]);
		}

		for(uint8_t i=0;i<NUM_SPI_BUS;i++) {
				alloc_spi_buffer(&spi_buffer[i], NUM_LEDS);
		}

		LEDS_ON(1 << leds_list[0]);
		LEDS_ON(1 << leds_list[1]);
		LEDS_ON(1 << leds_list[2]);
		LEDS_ON(1 << leds_list[3]);
	
		for(;;)
		{
			LEDS_INVERT(1 << leds_list[2]);

			for(int8_t idemo=0;idemo<size_of_list;idemo++)
			{
				LEDS_INVERT(1 << leds_list[1]);
				
				demo_list[idemo].function_init();
				
				int32_t rest = demo_list[idemo].demo_period;
				int32_t rap  = 0;
				int32_t step = demo_list[idemo].wait_ms + demo_list[idemo].process_time;
				
				while( rest > 0 )
				{
					LEDS_INVERT(1 << leds_list[0]);


					// animate and set up led_array_work 
					demo_list[idemo].function_update(led_array,rap);
					
					// dim LEDs until current limit 
					current_limit = CURRENT_LIMIT;
					ws2812b_driver_current_cap(led_array, NUM_LEDS, current_limit);

					// fade in/out effect
					if ( (demo_list[idemo].demo_period - rest) < FADE_IN_MS )
					{
						dim = (float)0.01+((float)0.99 * ((demo_list[idemo].demo_period - rest)/(float)FADE_IN_MS));
					}
					else if ( rest < FADE_IN_MS) 
					{
						dim = (float)0.01+((float)0.99 * (rest/(float)FADE_IN_MS));
					}
					if ( dim > (float)1.0 ) {
						dim = 1.0;
					}
					
					ws2812b_driver_dim(led_array, NUM_LEDS, dim);
					
					// LED update
					ws2812b_driver_xfer(led_array, spi_buffer[0], spi[0]);
					
					// delay (LED will be updated this period)
					nrf_delay_ms(demo_list[idemo].wait_ms);

					//
					rest -= step;
					rap += step;
				}

				// blank 3sec. between demos
				set_blank(led_array,NUM_LEDS);
				ws2812b_driver_xfer(led_array, spi_buffer[0], spi[0]);

				// delay (LED will be updated this period)
				nrf_delay_ms(3000);
			} // idemo
		} // end-less loop
}

/** @} */

