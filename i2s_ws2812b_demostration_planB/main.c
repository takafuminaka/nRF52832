/* Copyright (c) 2015 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 * @defgroup i2s_example_main main.c
 * @{
 * @ingroup i2s_example
 *
 * @brief I2S Example Application main file.
 *
 * This file contains the source code for a sample application using I2S.
 */

#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_i2s.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "boards.h"
#include "app_uart.h"

#include "project.h"
#include "ws2812b_drive.h"
#include "i2s_ws2812b_drive.h"

#include	"flashing_random.h"
#include	"running_rainbow.h"
#include	"running_rainbowv.h"

////// Definition of demos :start /////
#define	I2S_STDO_PIN	25

typedef struct
{
	void (*function_init)(uint16_t num_leds);
	void (*function_uninit)(void);
	void (*function_update)(rgb_led_t * led_array_out, uint32_t rap_time);
	uint16_t wait_ms;   // wait time (ms)
	int32_t demo_period;	// demo time (ms)
	uint16_t process_time;	// process time for each step (ms)
} demo_list_t;

const static demo_list_t demo_list[] = {
	{ &running_rainbowv_init, &running_rainbowv_uninit,&running_rainbowv, 20 ,30000,10},
	{ &running_rainbow_init, &running_rainbow_uninit,&running_rainbow, 20 ,30000,10},
	{ &flashing_random_init, &flashing_random_uninit,&flashing_random, 20,30000,10},
};

const static int8_t size_of_list = sizeof(demo_list)/sizeof(demo_list[0]);
////// Definition list of demos : end /////

// start of UART I/F for debug write
#define LED_MASK_OK         BSP_LED_0_MASK
#define LED_MASK_ERROR      BSP_LED_1_MASK

#define UART_TX_BUF_SIZE    256
#define UART_RX_BUF_SIZE    1

static void uart_event_handler(app_uart_evt_t * p_event)
{
    // This function is required by APP_UART_FIFO_INIT, but we don't need to
    // handle any events here.
}

static void init_uart(void)
{
    uint32_t err_code;

    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_ENABLED,
        .use_parity   = false,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud9600
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handler,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    APP_ERROR_CHECK(err_code);
}





void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    #ifdef DEBUG
    app_error_print(id, pc, info);
    #endif

    LEDS_ON(LEDS_MASK);
    while(1);
}




const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;
// end of UART I/F for debug write

int main(void)
{
		LEDS_CONFIGURE(LED_MASK_OK | LED_MASK_ERROR);

    init_uart();
    printf("\r\n"
           "I2S ws2812b demonstration\r\n");

		LEDS_ON(1 << leds_list[0]);
		LEDS_ON(1 << leds_list[1]);
		LEDS_ON(1 << leds_list[2]);
		LEDS_ON(1 << leds_list[3]);

		for (;;)
    {
			rgb_led_t led_array[NUM_LEDS];
			uint32_t current_limit;
			float dim;

			LEDS_INVERT(1 << leds_list[2]);

			for(int8_t idemo=0;idemo<size_of_list;idemo++)
			{
				printf("demo %d start",idemo);
				LEDS_INVERT(1 << leds_list[1]);
				
//				printf(" function_init %d\r\n",idemo);
				demo_list[idemo].function_init(NUM_LEDS);
				
				int32_t rest = demo_list[idemo].demo_period;
				int32_t rap  = 0;
				int32_t step = demo_list[idemo].wait_ms + demo_list[idemo].process_time;
				int32_t lrap = 0;
				
				while( rest > 0 )
				{
					LEDS_INVERT(1 << leds_list[0]);


					// animate and set up led_array_work 
					demo_list[idemo].function_update(led_array,rap);

					// dim LEDs until current limit 
					current_limit = CURRENT_LIMIT;
					ws2812b_drive_current_cap(led_array, NUM_LEDS, current_limit);

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
					
					// multiply dim value to all leds
					ws2812b_drive_dim(led_array, NUM_LEDS, dim);
					i2s_ws2812b_drive_xfer(led_array, NUM_LEDS, I2S_STDO_PIN);
					
					nrf_delay_ms(demo_list[idemo].wait_ms);

					//
					rest -= step;
					rap += step;

					if ( (lrap / 1000) != (rap / 1000) ) 
					{
						printf(" %3d",rap / 1000);
						lrap = rap;
					}
				} // while (rest > 0 )
        printf("\r\n");

				demo_list[idemo].function_uninit();
				
				// blank 3sec. between demos
				ws2812b_drive_set_blank(led_array,NUM_LEDS);
				i2s_ws2812b_drive_xfer(led_array, NUM_LEDS, I2S_STDO_PIN);

				// delay (LED will be updated this period)
				printf("blank and delay\r\n");
				nrf_delay_ms(3000);
			} // idemo
    } //		for (;;)
}


/** @} */
