/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "ws2812b_driver.h"

static volatile bool spi_transfer_completed[NUM_SPI_BUS];

static ws2812b_driver_spi_t  * ws2812b_driver_spi[NUM_SPI_BUS];

void alloc_spi_buffer(spi_buffer_t * spi_buffer, uint16_t num_leds)
{
	spi_buffer->buff = malloc(num_leds * BUF_SIZE_PER_LED);
	spi_buffer->length = num_leds * BUF_SIZE_PER_LED;
	spi_buffer->sector_size = LED_SECTOR_SIZE * BUF_SIZE_PER_LED;
}

void form_spi_sector(spi_buffer_t spi_buffer)
{
	uint8_t* p = spi_buffer.buff-1;
	for(uint16_t i=0;i<spi_buffer.length;i+=spi_buffer.sector_size)
	{
		p += spi_buffer.sector_size;
#ifdef DRIVE_MODE_8M
		if ( *p  == PATTERN_0 ) // if the end of sector bit is 0 
		{
			*p = PATTERN_0_EOS;
		}
		if ( *p  == PATTERN_1 ) // if the end of sector bit is 1 
		{
			*p = PATTERN_1_EOS;
		}
#endif
#ifdef DRIVE_MODE_4M
		if ( ( *p & 0x0f ) == PATTERN_0 ) // if the end of sector bit is 0 
		{
			*p = ( *p & 0xf0 ) | PATTERN_0_EOS;
		}
		if ( ( *p & 0x0f ) == PATTERN_1 ) // if the end of sector bit is 1 
		{
			*p = ( *p & 0xf0 ) | PATTERN_1_EOS;
		}
#endif
	}
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

void set_buff(rgb_led_t * rgb_led, spi_buffer_t spi_buffer)
{
	uint8_t* p_spi = spi_buffer.buff;
	rgb_led_t* p_led = rgb_led; 
	for(uint16_t i_led=0;i_led<(spi_buffer.length/BUF_SIZE_PER_LED);i_led++)
	{
		uint32_t rgb_data = (p_led->green << 16) | (p_led->red << 8 ) | p_led->blue;
		for(uint8_t i_rgb=0;i_rgb<BUF_SIZE_PER_LED;i_rgb++)
		{
#ifdef DRIVE_MODE_8M
			switch(rgb_data & 0x00800000 )
			{
				case ( 0x00000000 ):
					*p_spi = PATTERN_0;
					break;
				case ( 0x00800000 ):
					*p_spi = PATTERN_1;
					break;
			}
#endif
#ifdef DRIVE_MODE_4M
			switch(rgb_data & 0x00c00000 )
			{
				case ( 0x00400000 ):
					*p_spi = ( PATTERN_0 << 4 ) | PATTERN_1;
					break;
				case ( 0x00800000 ):
					*p_spi = ( PATTERN_1 << 4 ) | PATTERN_0;
					break;
				case ( 0x00c00000 ):
					*p_spi = ( PATTERN_1 << 4 ) | PATTERN_1;
					break;
				default:
					*p_spi = ( PATTERN_0 << 4 ) | PATTERN_0;
			}
#endif
			p_spi++;
			rgb_data <<= (24 / BUF_SIZE_PER_LED);
		}
		p_led++;
	}
}

void ws2812b_driver_spi_init(uint8_t id, ws2812b_driver_spi_t * spi)
{
	nrf_drv_spi_handler_t handler;

	nrf_drv_spi_config_t spi_config =
    {
        .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,
        .irq_priority = APP_IRQ_PRIORITY_HIGH,
#ifdef DRIVE_MODE_8M
				.frequency    = NRF_DRV_SPI_FREQ_8M,
#endif
#ifdef DRIVE_MODE_4M
				.frequency    = NRF_DRV_SPI_FREQ_4M,
#endif
			.mode         = NRF_DRV_SPI_MODE_1,
        .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,
				.miso_pin 		= NRF_DRV_SPI_PIN_NOT_USED,
    };

		switch(id)
		{
			case(0):
				spi_config.sck_pin  = SPIM0_SCK_PIN;
				spi_config.mosi_pin = SPIM0_MOSI_PIN;
				handler = spi0_event_handler;
			break;
#if ( NUM_SPI_BUF > 1 )
			case(1):
				spi_config.sck_pin  = SPIM1_SCK_PIN;
				spi_config.mosi_pin = SPIM1_MOSI_PIN;
				handler = spi1_event_handler;
				break;
#if ( NUM_SPI_BUF > 2 )
			case(2):
				spi_config.sck_pin  = SPIM2_SCK_PIN;
				spi_config.mosi_pin = SPIM2_MOSI_PIN;
				handler = spi2_event_handler;
				break;
#endif
#endif

		}
		spi->transfer_completed = &spi_transfer_completed[id];
		ws2812b_driver_spi[id] = spi;
		nrf_drv_spi_init(&(spi->spi), &spi_config, handler);

}	

void spi0_event_handler(nrf_drv_spi_evt_t const * event)
{
  ws2812b_driver_spi_t * spi_base = ws2812b_driver_spi[0];
	
//	if ( spi_base->rest > 0 )
//	{
//		spi_base->xfer_desc.tx_length   = (spi_base->spi_buffer.sector_size>spi_base->rest)?spi_base->rest:spi_base->spi_buffer.sector_size;
//		// *spi_base->transfer_completed = false;
//		nrf_drv_spi_xfer(&spi_base->spi, &spi_base->xfer_desc, 0);
//		spi_base->xfer_desc.p_tx_buffer += spi_base->spi_buffer.sector_size;
//		spi_base->rest -= spi_base->xfer_desc.tx_length;
//  }
//	else
//	{
		*spi_base->transfer_completed = true;
// 	}
}

#if ( NUM_SPI_BUF > 1 )
void spi1_event_handler(nrf_drv_spi_evt_t const * event)
{
		spi_transfer_completed[1] = true;
}
#endif

#if ( NUM_SPI_BUF > 2 )
void spi2_event_handler(nrf_drv_spi_evt_t const * event)
{
		spi_transfer_completed[2] = true;
}
#endif

void ws2812b_driver_xfer(rgb_led_t * led_array, spi_buffer_t spi_buffer, ws2812b_driver_spi_t spi_base)
{
		spi_base.spi_buffer = spi_buffer;
	
		set_buff(led_array,spi_base.spi_buffer);
		form_spi_sector(spi_base.spi_buffer);
	
		spi_base.xfer_desc.p_tx_buffer = spi_base.spi_buffer.buff;
		spi_base.xfer_desc.p_rx_buffer = NULL;
		spi_base.xfer_desc.rx_length   = NULL;
	
		spi_base.rest = spi_base.spi_buffer.length;
	
		// SPI transfer loop
		spi_base.xfer_desc.tx_length   = spi_base.spi_buffer.sector_size;
		while(spi_base.rest > spi_base.xfer_desc.tx_length )
		{
			*spi_base.transfer_completed = false;
			nrf_drv_spi_xfer(&spi_base.spi, &spi_base.xfer_desc, 0);
			spi_base.xfer_desc.p_tx_buffer += spi_base.spi_buffer.sector_size;
			spi_base.rest -= spi_base.xfer_desc.tx_length;

			while (! *spi_base.transfer_completed) {}
		}
		spi_base.xfer_desc.tx_length   = spi_base.rest;
		*spi_base.transfer_completed = false;
		nrf_drv_spi_xfer(&spi_base.spi, &spi_base.xfer_desc, 0);
		spi_base.xfer_desc.p_tx_buffer += spi_base.spi_buffer.sector_size;
		spi_base.rest -= spi_base.xfer_desc.tx_length;
    while (! *spi_base.transfer_completed) {}

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
