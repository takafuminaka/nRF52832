/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "ws2812b_driver.h"

void alloc_xfer_buffer(xfer_buffer_t * xfer_buffer, uint16_t num_leds)
{
	xfer_buffer->buff = malloc(num_leds * BUF_SIZE_PER_LED);
	xfer_buffer->length = num_leds * BUF_SIZE_PER_LED;
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

void set_buff(rgb_led_t * rgb_led, xfer_buffer_t xfer_buffer)
{
	uint8_t* p_xfer = xfer_buffer.buff;
	rgb_led_t* p_led = rgb_led; 
	int8_t offset = 1;
	
	for(uint16_t i_led=0;i_led<(xfer_buffer.length/BUF_SIZE_PER_LED);i_led++)
	{
		uint32_t rgb_data = (p_led->green << 16) | (p_led->red << 8 ) | p_led->blue;
		for(uint8_t i_rgb=0;i_rgb<BUF_SIZE_PER_LED;i_rgb++)
		{
			switch(rgb_data & 0x00c00000 )
			{
				case ( 0x00400000 ):
					*(p_xfer + offset)  = (uint8_t)(( PATTERN_0 << 4 ) | PATTERN_1);
					break;
				case ( 0x00800000 ):
					*(p_xfer + offset)  = (uint8_t)(( PATTERN_1 << 4 ) | PATTERN_0);
					break;
				case ( 0x00c00000 ):
					*(p_xfer + offset)  = (uint8_t)(( PATTERN_1 << 4 ) | PATTERN_1);
					break;
				default:
					*(p_xfer + offset)  = (uint8_t)(( PATTERN_0 << 4 ) | PATTERN_0);
			}
			p_xfer++;
			offset = -offset;
			rgb_data <<= (24 / BUF_SIZE_PER_LED);
		}
		p_led++;
	}
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
