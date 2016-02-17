/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */


#include	"running_rainbow.h"
#include	"project.h"
#include "ws2812b_driver.h"
#include  "stdlib.h"

#define STEP_SRIDE1					(-(ROW_SIZE+1))
#define STEP_SRIDE2					(-(ROW_SIZE-1))
#define DECAY_STEP					(0)
#define	PRAB_FLASH					(1000)


static rgb_led_t led_array_base[NUM_LEDS];				// array for base color
static rgb_led_t led_array_flash1[NUM_LEDS]; // array for flash right-up to left-down
static rgb_led_t led_array_flash2[NUM_LEDS]; // array for flash left-up to right-down
static rgb_led_t led_array_work[NUM_LEDS]; // array for flash left-up to right-down
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

void running_rainbow_init()
{
	uint32_t iregion;
  float ratio;

	// initialize led_array (base color array)
	for(uint16_t i=0;i<NUM_LEDS;i++)
	{
		iregion = (n_color_list-1)*i/NUM_LEDS;
		ratio = (i - iregion*(float)NUM_LEDS/(n_color_list-1))/((float)NUM_LEDS/(n_color_list-1));
		led_array_base[i].green = color_list[iregion].green*(1-ratio) + color_list[iregion+1].green * ratio;
		led_array_base[i].red   = color_list[iregion].red*(1-ratio)   + color_list[iregion+1].red   * ratio;
		led_array_base[i].blue  = color_list[iregion].blue*(1-ratio)  + color_list[iregion+1].blue  * ratio;
	}

	// initialize led_array for flash 
	for(uint16_t i=0;i<NUM_LEDS;i++)
	{
			led_array_flash1[i].green = 0;
			led_array_flash1[i].red   = 0;
			led_array_flash1[i].blue  = 0;

			led_array_flash2[i].green = 0;
			led_array_flash2[i].red   = 0;
			led_array_flash2[i].blue  = 0;
	}
}

void running_rainbow(rgb_led_t * led_array_out, uint32_t rap_time)
{
	int16_t nextc;
	
	// update led_array_base
	{
		for(uint16_t i=0;i<NUM_LEDS;i++)
		{
			led_array_work[i] = led_array_base[i];
		}

		int16_t delta=-6;
		for(uint16_t i=0;i<NUM_LEDS;i++) {
			led_array_base[i] = led_array_work[(delta+i+NUM_LEDS)%NUM_LEDS];
		}
	}
// Update led_array_flash1
	{
		for(uint16_t i=0;i<NUM_LEDS;i++)
		{
			led_array_work[i] = led_array_flash1[i];
		}
		for(uint16_t i=0;i<NUM_LEDS;i++)
		{
			if ( rand()%PRAB_FLASH == 0 )
			{
				led_array_flash1[i].green = MAX_INTENSE2;
				led_array_flash1[i].red   = MAX_INTENSE2;
				led_array_flash1[i].blue  = MAX_INTENSE2;
			}
			else if ( i + STEP_SRIDE1 >= NUM_LEDS || i + STEP_SRIDE1 < 0 ) 
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
		for(uint16_t i=0;i<NUM_LEDS;i++)
		{
			led_array_work[i] = led_array_flash2[i];
		}
		for(uint16_t i=0;i<NUM_LEDS;i++)
		{
			if ( rand()%PRAB_FLASH == 0 )
			{
				led_array_flash2[i].green = MAX_INTENSE2;
				led_array_flash2[i].red   = MAX_INTENSE2;
				led_array_flash2[i].blue  = MAX_INTENSE2;
			}
			else if ( i + STEP_SRIDE2 >= NUM_LEDS || i + STEP_SRIDE2 < 0 ) 
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
		for(uint16_t i=0;i<NUM_LEDS;i++)
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
