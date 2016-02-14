/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "ws2812b_driver.h"

static volatile bool i2s_transfer_completed;
static uint32_t *i2s_transfer_tx_source;

// static ws2812b_driver_i2s_t  * ws2812b_driver_i2s;

void alloc_i2s_buffer(i2s_buffer_t * i2s_buffer, uint16_t num_leds)
{
	i2s_buffer->buff = malloc(num_leds * BUF_SIZE_PER_LED);
	i2s_buffer->length = num_leds * BUF_SIZE_PER_LED;
	i2s_buffer->tx_buff = malloc(num_leds * BUF_SIZE_PER_LED * 2);
}


void set_blank(rgb_led_t * rgb_led, uint16_t num_leds)
{
	rgb_led_t * p = rgb_led;
	for(uint16_t i=0;i<num_leds;i++)
	{
		p->green = 0;
		p->red   = 0;
		p->blue  = 0;
		p++;
	}
}

void set_buff(rgb_led_t * rgb_led, i2s_buffer_t i2s_buffer)
{
	uint8_t* p_i2s = i2s_buffer.buff;
	rgb_led_t* p_led = rgb_led; 
	for(uint16_t i_led=0;i_led<(i2s_buffer.length/BUF_SIZE_PER_LED);i_led++)
	{
		uint32_t rgb_data = (p_led->green << 16) | (p_led->red << 8 ) | p_led->blue;
		for(uint8_t i_rgb=0;i_rgb<BUF_SIZE_PER_LED;i_rgb++)
		{
			switch(rgb_data & 0x00c00000 )
			{
				case ( 0x00400000 ):
					*p_i2s = (uint8_t)(( PATTERN_0 << 4 ) | PATTERN_1);
					break;
				case ( 0x00800000 ):
					*p_i2s = (uint8_t)(( PATTERN_1 << 4 ) | PATTERN_0);
					break;
				case ( 0x00c00000 ):
					*p_i2s = (uint8_t)(( PATTERN_1 << 4 ) | PATTERN_1);
					break;
				default:
					*p_i2s = (uint8_t)(( PATTERN_0 << 4 ) | PATTERN_0);
			}
			p_i2s++;
			rgb_data <<= (24 / BUF_SIZE_PER_LED);
		}
		p_led++;
	}
}

void ws2812b_driver_i2s_init(ws2812b_driver_i2s_t * i2s)
{
	nrf_drv_i2s_data_handler_t handler;
	ret_code_t err_code;

	printf("ws2812b_driver_i2s_init called\r\n");

	nrf_drv_i2s_config_t i2s_config = NRF_DRV_I2S_DEFAULT_CONFIG;
	i2s_config.irq_priority = APP_IRQ_PRIORITY_HIGH,
  i2s_config.mck_setup = NRF_I2S_MCK_32MDIV10;
  i2s_config.ratio     = NRF_I2S_RATIO_32X;

	i2s_config.sdout_pin = I2S_CONFIG_SDOUT_PIN;
	handler = i2s_event_handler;

	i2s->transfer_completed = &i2s_transfer_completed;
//	ws2812b_driver_i2s = i2s;
	err_code = nrf_drv_i2s_init(&i2s_config, handler);
	printf("  nrf_drv_i2s_init returns with %d\r\n",err_code);
}	

void i2s_event_handler(uint32_t const * p_data_received,
                         uint32_t       * p_data_to_send,
                         uint16_t         number_of_words)
{
    printf(" i2s_event_handler\r\n");
		if (p_data_to_send != NULL)
		{
			i2s_transfer_completed = 1;
		}
		else
		{
			p_data_to_send = i2s_transfer_tx_source;
		}
}

void ws2812b_driver_xfer(rgb_led_t * led_array, i2s_buffer_t i2s_buffer, ws2812b_driver_i2s_t i2s_base)
{
		ret_code_t err_code;
		i2s_base.i2s_buffer = i2s_buffer;

		printf("  set_buff\r\n");
		set_buff(led_array,i2s_base.i2s_buffer);
	
		i2s_base.transfer_completed = 0;
		i2s_transfer_tx_source = (uint32_t *)i2s_base.i2s_buffer.buff;
	
		printf("  nrf_drv_i2s_start\r\n");
//		err_code = nrf_drv_i2s_start(NULL, i2s_base.i2s_buffer.tx_buff , i2s_base.i2s_buffer.length*2/4, 0);
		printf("  nrf_drv_i2s_start return with status %d\r\n",err_code);

//    while (i2s_base.transfer_completed == 0)
//    {
//		}
    nrf_drv_i2s_stop();
}

void ws2812b_driver_current_cap(rgb_led_t * led_array, uint16_t num_leds, uint32_t limit)
{
	uint32_t sum0 = ws2812b_driver_calc_current(led_array, num_leds);
	if ( sum0 > limit ) {
		// fact = (limit - num_leds) / (sum0 - num_leds);
		int32_t factn = limit - num_leds;
		if ( factn < 0 )
		{
			factn = 1;
		}
		int32_t factd = sum0 - num_leds;
		if ( factd < 0 )
		{
			factd = 1;
		}

		rgb_led_t * p = led_array;	
		rgb_led_t dnext;
		for(uint16_t i=0;i<num_leds;i++)
		{
			dnext.green = p->green * factn / factd;
			dnext.red   = p->red   * factn / factd;
			dnext.blue  = p->blue  * factn / factd;

			if ( dnext.green == 0 && p->green > 0 )
			{
				dnext.green = 1;
			}
			if ( dnext.red   == 0 && p->red   > 0 ) 
			{
				dnext.red   = 1;
			}
			if ( dnext.blue  == 0 && p->blue  > 0 )
			{
				dnext.blue  = 1;
			}
			*p = dnext;
			p++;
		} // i
 	}	
}

uint32_t ws2812b_driver_calc_current(rgb_led_t * led_array, uint16_t num_leds)
{
	uint32_t sum = 0;
	rgb_led_t * p = led_array;
	for(uint16_t i=0;i<num_leds;i++)
	{
		sum += p->green + p->red + p->blue;
		p++;
	}
	
	return(num_leds + (sum*45)/(255*3)); // mA
}

void ws2812b_driver_dim(rgb_led_t * led_array, uint16_t num_leds, float dim )
{
		rgb_led_t * p = led_array;	
		for(uint16_t i=0;i<num_leds;i++)
		{
			p->green *= dim;
			p->red   *= dim;
			p->blue  *= dim;

			p++;
		} // i

}
