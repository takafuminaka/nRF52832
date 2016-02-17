/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
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
#include "nrf_drv_i2s.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "boards.h"
#include "app_uart.h"

#include "project.h"
#include "ws2812b_driver.h"
#include	"flashing_random.h"
#include	"running_rainbow.h"
#include	"running_rainbowv.h"


#define LED_MASK_OK         BSP_LED_0_MASK
#define LED_MASK_ERROR      BSP_LED_1_MASK

#define UART_TX_BUF_SIZE    256
#define UART_RX_BUF_SIZE    1

#define I2S_BUFFER_SIZE     (NUM_LEDS * BUF_SIZE_PER_LED * 2 / 4)
//static uint32_t m_buffer_rx[I2S_BUFFER_SIZE/2];
static uint32_t m_buffer_tx[I2S_BUFFER_SIZE/2];
static uint8_t *tx_buffer;

// Delay time between consecutive I2S transfers performed in the main loop
// (in milliseconds).
#define PAUSE_TIME          500
// Number of blocks of data (resulting from one call to data handler, thus
// having the size equal to half size of the buffer) to be contained in each
// transfer.
#define BLOCKS_TO_TRANSFER  1

static volatile uint8_t  m_blocks_transferred     = 0;
static volatile uint8_t  m_blocks_bufferred     = 0;
static          uint8_t  m_zero_samples_to_ignore = 0;
static          uint16_t m_sample_value_to_send;
static          uint16_t m_sample_value_expected;
static          bool     m_error_encountered;


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


//static void prepare_tx_data(uint32_t * p_buffer, uint16_t number_of_words)
//{
//    // These variables will be both zero only at the very beginning of each
//    // transfer, so we use them as the indication that the re-initialization
//    // should be performed.
//    if (m_blocks_transferred == 0 && m_zero_samples_to_ignore == 0)
//    {
//        // Number of initial samples (actually pairs of L/R samples) with zero
//        // values that should be ignored - see the comment in 'check_samples'.
//        m_zero_samples_to_ignore = 2;
//        m_sample_value_to_send   = 0xCAFE;
//        m_sample_value_expected  = 0xCAFE;
//        m_error_encountered      = false;
//    }

//    // [each data word contains two 16-bit samples]
//    uint16_t i;
//    for (i = 0; i < number_of_words; ++i)
//    {
//        uint16_t sample_l = m_sample_value_to_send - 1;
//        uint16_t sample_r = m_sample_value_to_send + 1;
//        ++m_sample_value_to_send;

//        ((uint16_t *)p_buffer)[2*i]     = sample_l;
//        ((uint16_t *)p_buffer)[2*i + 1] = sample_r;
//    }
//}


//static bool check_samples(uint32_t const * p_buffer, uint16_t number_of_words)
//{
//    printf("%3u: ", m_blocks_transferred);

//    // [each data word contains two 16-bit samples]
//    uint16_t i;
//    for (i = 0; i < number_of_words; ++i)
//    {
//        uint16_t actual_sample_l   = ((uint16_t const *)p_buffer)[2*i];
//        uint16_t actual_sample_r   = ((uint16_t const *)p_buffer)[2*i + 1];

//        // Normally a couple of initial samples sent by the I2S peripheral
//        // will have zero values, because it starts to output the clock
//        // before the actual data is fetched by EasyDMA. As we are dealing
//        // with streaming the initial zero samples can be simply ignored.
//        if (m_zero_samples_to_ignore > 0 &&
//            actual_sample_l == 0 &&
//            actual_sample_r == 0)
//        {
//            --m_zero_samples_to_ignore;
//        }
//        else
//        {
//            m_zero_samples_to_ignore = 0;

//            uint16_t expected_sample_l = m_sample_value_expected - 1;
//            uint16_t expected_sample_r = m_sample_value_expected + 1;
//            ++m_sample_value_expected;

//            if (actual_sample_l != expected_sample_l ||
//                actual_sample_r != expected_sample_r)
//            {
//                printf("%04x/%04x, expected: %04x/%04x\r\n",
//                    actual_sample_l, actual_sample_r,
//                    expected_sample_l, expected_sample_r);
//                return false;
//            }
//        }
//    }

//    printf("OK\r\n");
//    return true;
//}


//static void check_rx_data(uint32_t const * p_buffer, uint16_t number_of_words)
//{
//    ++m_blocks_transferred;

//    if (!m_error_encountered)
//    {
//        m_error_encountered = true; // !check_samples(p_buffer, number_of_words);
//    }

////    if (m_error_encountered)
////    {
////        LEDS_OFF(LED_MASK_OK);
////        LEDS_INVERT(LED_MASK_ERROR);
////    }
////    else
////    {
////        LEDS_OFF(LED_MASK_ERROR);
////        LEDS_INVERT(LED_MASK_OK);
////    }
//}


// This is the I2S data handler - all data exchange related to the I2S transfers
// is done here.
static void data_handler(uint32_t const * p_data_received,
                         uint32_t       * p_data_to_send,
                         uint16_t         number_of_words)
{
    // Non-NULL value in 'p_data_received' indicates that a new portion of
    // data has been received and should be processed.
//		printf("data_handler called\r\n");

    // Non-NULL value in 'p_data_to_send' indicates that the driver needs
    // a new portion of data to send.
    if (p_data_to_send != NULL)
    {
//			printf("data_handler for tx\r\n");
			if ( m_blocks_bufferred == 0 )
			{
				memcpy(p_data_to_send,tx_buffer,number_of_words*4);
				m_blocks_bufferred = 1;
			}
			else
			{
				++m_blocks_transferred;
			}
    }
}


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    #ifdef DEBUG
    app_error_print(id, pc, info);
    #endif

    LEDS_ON(LEDS_MASK);
    while(1);
}


////// Define of demos :start /////
typedef struct
{
	void (*function_init)();
	void (*function_update)(rgb_led_t * led_array_out, uint32_t rap_time);
	uint16_t wait_ms;   // wait time (ms)
	int32_t demo_period;	// demo time (ms)
	uint16_t process_time;	// process time for each step (ms)
} demo_list_t;

const static demo_list_t demo_list[] = {
	{ &running_rainbowv_init, &running_rainbowv, 20 ,30000,10},
	{ &running_rainbow_init, &running_rainbow, 20 ,30000,10},
	{ &flashing_random_init, &flashing_random, 20,30000,10},
};

const static int8_t size_of_list = sizeof(demo_list)/sizeof(demo_list[0]);
////// Define list of demos : end /////


const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;

int main(void)
{
    uint32_t err_code = NRF_SUCCESS;
		xfer_buffer_t xfer_buffer;

		LEDS_CONFIGURE(LED_MASK_OK | LED_MASK_ERROR);

    init_uart();
    printf("\r\n"
           "I2S ws2812b demonstration\r\n");

    nrf_drv_i2s_config_t config = NRF_DRV_I2S_DEFAULT_CONFIG;
    // In Master mode the MCK frequency and the MCK/LRCK ratio should be
    // set properly in order to achieve desired audio sample rate (which
    // is equivalent to the LRCK frequency).
    // For the following settings we'll get the LRCK frequency equal to
    // 15873 Hz (the closest one to 16 kHz that is possible to achieve).
    config.mck_setup = NRF_I2S_MCK_32MDIV10;
    config.ratio     = NRF_I2S_RATIO_32X;
		config.format    = NRF_I2S_FORMAT_ALIGNED;
		config.alignment = NRF_I2S_ALIGN_LEFT;
		

		alloc_xfer_buffer(&xfer_buffer, NUM_LEDS);
	
    err_code = nrf_drv_i2s_init(&config, data_handler);
    APP_ERROR_CHECK(err_code);

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
				demo_list[idemo].function_init();
				
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
					
					set_buff(led_array, xfer_buffer);
          tx_buffer = xfer_buffer.buff;					
					
					// LED update
//					printf(" xfer\r\n",idemo);
					
					
					m_blocks_transferred = 0;
					m_blocks_bufferred = 0;

//					printf("nrf_drv_i2s_start\r\n");
					
					err_code = nrf_drv_i2s_start(NULL, m_buffer_tx,
							I2S_BUFFER_SIZE, 0);
					APP_ERROR_CHECK(err_code);

//					while (m_blocks_transferred < BLOCKS_TO_TRANSFER)
//					{}

          // wait for 
					nrf_delay_us((NUM_LEDS+20) * (24*5/4));

//					printf("success\r\n");
					nrf_drv_i2s_stop();

					// delay (LED will be updated this period)
//					printf(" delay\r\n",idemo);
						nrf_delay_ms(demo_list[idemo].wait_ms);

					//
					rest -= step;
					rap += step;

					if ( (lrap / 1000) != (rap / 1000) ) 
					{
						printf(" %3d",rap / 1000);
						lrap = rap;
					}

				}
        printf("\r\n");
				
				// blank 3sec. between demos
				set_blank(led_array,NUM_LEDS);
				set_buff(led_array, xfer_buffer);
				tx_buffer = xfer_buffer.buff;					
				
				// LED update
//				printf(" xfer %d\r\n",idemo);
				
				
				m_blocks_transferred = 0;
				m_blocks_bufferred = 0;

//				printf("nrf_drv_i2s_start\r\n");
				
				err_code = nrf_drv_i2s_start(NULL, m_buffer_tx,
						I2S_BUFFER_SIZE, 0);
				APP_ERROR_CHECK(err_code);

//				while (m_blocks_transferred < BLOCKS_TO_TRANSFER)
//				{}
				nrf_delay_us((NUM_LEDS+20) * (24*5/4));

//				printf("success\r\n");
				nrf_drv_i2s_stop();



				// delay (LED will be updated this period)
				printf("blank and delay\r\n");
				nrf_delay_ms(3000);
			} // idemo



    }
}

/** @} */
