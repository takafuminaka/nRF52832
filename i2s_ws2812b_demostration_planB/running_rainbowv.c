/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include	"running_rainbowv.h"
#include	"project.h"
#include	"ws2812b_drive.h"
#include	"stdlib.h"

#define STEP_SRIDE1					(-(ROW_SIZE+1))
#define STEP_SRIDE2					(-(ROW_SIZE-1))
#define DECAY_STEP					(30)
#define	PRAB_FLASH					(1000)

static rgb_led_t *led_array_base;				// array for base color
static rgb_led_t *led_array_flash1; // array for flash right-up to left-down
static rgb_led_t *led_array_flash2; // array for flash left-up to right-down
static rgb_led_t *led_array_work; // array for flash left-up to right-down
static uint16_t running_rainbow_num_leds;

const static rgb_led_t color_list[] = {
	{0           , MAX_INTENSE3, 0           }, // Red
	{MAX_INTENSE3, MAX_INTENSE3, 0           }, // Yellow
	{MAX_INTENSE3, 0           , 0           }, // Green
	{MAX_INTENSE3, 0           , MAX_INTENSE3}, // rght green
	{0           , 0           , MAX_INTENSE3}, // Blue
	{0           , MAX_INTENSE3, MAX_INTENSE3}, // Purple
	{0           , MAX_INTENSE3, 0           }, // Red
};

const static uint8_t n_color_list = sizeof(color_list)/sizeof(color_list[0]);

void running_rainbowv_init(uint16_t num_leds)
{
	uint32_t iregion;
  float ratio;
  running_rainbow_num_leds = num_leds;
	
		// allocate buffers
	led_array_base		= malloc(num_leds * sizeof(rgb_led_t));
	led_array_flash1	= malloc(num_leds * sizeof(rgb_led_t));
	led_array_flash2	= malloc(num_leds * sizeof(rgb_led_t));
	led_array_work		= malloc(num_leds * sizeof(rgb_led_t));
	
	// initialize led_array (base color array)
	uint8_t col_size = running_rainbow_num_leds/ROW_SIZE;
	uint16_t v_num_leds = col_size*ROW_SIZE;

	for(uint16_t i0=0;i0<running_rainbow_num_leds;i0++)
	{
		uint16_t irow = i0/ROW_SIZE;
		uint16_t icol = i0%ROW_SIZE;
		uint16_t i= icol*col_size + irow; 
		i = i % v_num_leds;
		iregion = (n_color_list-1)*i/v_num_leds;
		ratio = (i - iregion*(float)v_num_leds/(n_color_list-1))/((float)v_num_leds/(n_color_list-1));
		led_array_base[i0].green = color_list[iregion].green*(float)(1-ratio) + color_list[iregion+1].green * (float)ratio;
		led_array_base[i0].red   = color_list[iregion].red*(float)(1-ratio)   + color_list[iregion+1].red   * (float)ratio;
		led_array_base[i0].blue  = color_list[iregion].blue*(float)(1-ratio)  + color_list[iregion+1].blue  * (float)ratio;
	}

	
	
	// initialize led_array for flash 
	for(uint16_t i=0;i<running_rainbow_num_leds;i++)
	{
			led_array_flash1[i].green = 0;
			led_array_flash1[i].red   = 0;
			led_array_flash1[i].blue  = 0;

			led_array_flash2[i].green = 0;
			led_array_flash2[i].red   = 0;
			led_array_flash2[i].blue  = 0;
	}
}

void running_rainbowv_uninit()
{
		free(led_array_base);
		free(led_array_flash1);
		free(led_array_flash2);
		free(led_array_work);
}


void running_rainbowv(rgb_led_t * led_array_out, uint32_t rap_time)
{
	int16_t nextc;
	uint16_t num_leds = running_rainbow_num_leds/ROW_SIZE*ROW_SIZE;

	// update led_array_base
	{
		for(uint16_t i=0;i<running_rainbow_num_leds;i++)
		{
			led_array_work[i] = led_array_base[i];
		}
		uint16_t i;
		uint16_t delta = ROW_SIZE*6;
		for(uint16_t i0=0;i0<running_rainbow_num_leds;i0++) {
			if ( i0 > delta )
			{
				i = i0 - delta;
			}
			else
			{
				i = i0 + num_leds - 1 - delta;
			}
			
			led_array_base[i0] = led_array_work[i];
		}
	}
// Update led_array_flash1
	{
		for(uint16_t i=0;i<running_rainbow_num_leds;i++)
		{
			led_array_work[i] = led_array_flash1[i];
		}
		for(uint16_t i=0;i<running_rainbow_num_leds;i++)
		{
			if ( rand()%PRAB_FLASH == 0 )
			{
				led_array_flash1[i].green = MAX_INTENSE2;
				led_array_flash1[i].red   = MAX_INTENSE2;
				led_array_flash1[i].blue  = MAX_INTENSE2;
			}
			else if ( i + STEP_SRIDE1 >= running_rainbow_num_leds || i + STEP_SRIDE1 < 0 ) 
			{
				led_array_flash1[i].green = 0;
				led_array_flash1[i].red   = 0;
				led_array_flash1[i].blue  = 0;
			}
			else
			{
				nextc = led_array_work[i+STEP_SRIDE1].green - DECAY_STEP;
				if ( nextc < 0 )
				{
					nextc = 0;
				}
				led_array_flash1[i].green = nextc;
		
				nextc = led_array_work[i+STEP_SRIDE1].red - DECAY_STEP;
				if ( nextc < 0 )
				{
					nextc = 0;
				}
				led_array_flash1[i].red = nextc;
		
				nextc = led_array_work[i+STEP_SRIDE1].blue - DECAY_STEP;
				if ( nextc < 0 )
				{
					nextc = 0;
				}
				led_array_flash1[i].blue = nextc;
			}	
		}
	}

// Update led_array_flash2
	{
		for(uint16_t i=0;i<running_rainbow_num_leds;i++)
		{
			led_array_work[i] = led_array_flash2[i];
		}
		for(uint16_t i=0;i<running_rainbow_num_leds;i++)
		{
			if ( rand()%PRAB_FLASH == 0 )
			{
				led_array_flash2[i].green = MAX_INTENSE2;
				led_array_flash2[i].red   = MAX_INTENSE2;
				led_array_flash2[i].blue  = MAX_INTENSE2;
			}
			else if ( i + STEP_SRIDE2 >= running_rainbow_num_leds || i + STEP_SRIDE2 < 0 ) 
			{
				led_array_flash2[i].green = 0;
				led_array_flash2[i].red   = 0;
				led_array_flash2[i].blue  = 0;
			}
			else
			{
				nextc = led_array_work[i+STEP_SRIDE2].green - DECAY_STEP;
				if ( nextc < 0 )
				{
					nextc = 0;
				}
				led_array_flash2[i].green = nextc;
			
				nextc = led_array_work[i+STEP_SRIDE2].red - DECAY_STEP;
				if ( nextc < 0 )
				{
					nextc = 0;
				}
				led_array_flash2[i].red = nextc;
			
					nextc = led_array_work[i+STEP_SRIDE2].blue - DECAY_STEP;
				if ( nextc < 0 )
				{
					nextc = 0;
				}
				led_array_flash2[i].blue = nextc;
			}
		}
	}
// Merge led arrays	
	{
		for(uint16_t i=0;i<running_rainbow_num_leds;i++)
		{
			nextc = led_array_base[i].green + led_array_flash1[i].green + led_array_flash2[i].green;
			if ( nextc < MIN_INTENSE )
			{
				nextc = MIN_INTENSE;
			}
				
			if ( nextc > MAX_INTENSE2 )
			{
				nextc = MAX_INTENSE2;
			}
			led_array_out[i].green = nextc;
				
			nextc = led_array_base[i].red + led_array_flash1[i].red + led_array_flash2[i].red;
			if ( nextc < MIN_INTENSE )
			{
				nextc = MIN_INTENSE;
			}
				
			if ( nextc > MAX_INTENSE2 )
			{
				nextc = MAX_INTENSE2;
			}
			led_array_out[i].red = nextc;

			nextc = led_array_base[i].blue + led_array_flash1[i].blue + led_array_flash2[i].blue;
			if ( nextc < MIN_INTENSE )
			{
				nextc = MIN_INTENSE;
			}
				
			if ( nextc > MAX_INTENSE2 )
		 	{
				nextc = MAX_INTENSE2;
			}
			led_array_out[i].blue = nextc;
		
		}	
	}
}

