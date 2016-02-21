/* Copyright (c) 2015 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#define NUM_LEDS			(240)		// Number of LEDs (LED総数）
#define MAX_INTENSE		(64)		// Max intense of random LEDs for "flashing_random" (flashing_random用のランダム色LEDの最大輝度)
#define MAX_INTENSE2	(255)		// Max intense of "shooting start" running bright LEDs for all demos.
#define MAX_INTENSE3	(64)		// Max intense of "rainbow LEDs" for "running_rainbow" and "running_rainbowv" demos. 
#define MIN_INTENSE		(1)			// Minimum intense of randaom LEDs for "flashing_random" (flashing_random用のランダム色LEDの最小輝度)
#define	ROW_SIZE			(17)		// Count of LEDs for each line (1巻き分のLEDの数)
#define CURRENT_LIMIT	(1500) 	// Current limit of LEDs (mA)	(総電流制限値)
#define	FADE_IN_MS		(6000)	// Fade in/out period (フェードイン・アウトの秒数）
