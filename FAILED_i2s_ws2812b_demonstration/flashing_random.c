/* Copyright (c) 2015 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */


#include	"flashing_random.h"
#include	"project.h"
#include "ws2812b_driver.h"
#include  "stdlib.h"

#define STEP_SRIDE1					(-(ROW_SIZE+1))
#define STEP_SRIDE2					(-(ROW_SIZE-1))
#define DECAY_STEP					(30)
#define	PRAB_FLASH					(1000)


static rgb_led_t led_array_base[NUM_LEDS];				// array for base color
static rgb_led_t led_array_flash1[NUM_LEDS]; // array for flash right-up to left-down
static rgb_led_t led_array_flash2[NUM_LEDS]; // array for flash left-up to right-down
static rgb_led_t led_array_work[NUM_LEDS]; // array for flash left-up to right-down

void flashing_random_init()
{
		// initialize led_array (base color array)
		for(uint16_t i=0;i<NUM_LEDS;i++)
		{
			int c = (i % 7) + 1;
				led_array_base[i].green = (MAX_INTENSE+MIN_INTENSE)/2 * ((c&4)>>2);
				led_array_base[i].red   = (MAX_INTENSE+MIN_INTENSE)/2 * ((c&2)>>1);
				led_array_base[i].blue  = (MAX_INTENSE+MIN_INTENSE)/2 * ((c&1)>>0);
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

void flashing_random(rgb_led_t * led_array_out, uint32_t rap_time)
{
	int16_t nextc;
	
	// update led_array_base
	{
		for(uint16_t i=0;i<NUM_LEDS;i++) {
			nextc = led_array_base[i].green + rand()%3 -1;
			if ( nextc < MIN_INTENSE )
			{
				nextc = MIN_INTENSE;
			}
				
			if ( nextc > MAX_INTENSE )
			{
				nextc = MAX_INTENSE;
			}
			led_array_base[i].green = nextc;
				
			nextc = led_array_base[i].red + rand()%3 -1;
			if ( nextc < MIN_INTENSE )
			{
				nextc = MIN_INTENSE;
			}
				
			if ( nextc > MAX_INTENSE )
			{
				nextc = MAX_INTENSE;
			}
			led_array_base[i].red = nextc;

			nextc = led_array_base[i].blue + rand()%3 -1;
			if ( nextc < MIN_INTENSE )
			{
				nextc = MIN_INTENSE;
			}
				
			if ( nextc > MAX_INTENSE )
			{
				nextc = MAX_INTENSE;
			}
			led_array_base[i].blue = nextc;
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


