/*
 * prj_main.c
 *
 *  Created on: Apr 9, 2019
 *      Author: Sandeep
 */

#include "functional_interface.h"
/**
 * Volatile variables for using in interrupt handler for software pwm detection
 */
volatile u8 duty_cycle[3] = { 1, 2, 3 };
volatile bool signal[3];
volatile bool old_signal[3];
volatile u32 high_level[3];
volatile u32 low_level[3];

/**
 * ************************ MAIN PROGRAM for the Project***********************************
 */
int main(void) {
	init_platform();

	uint32_t sts;
	u16 hue;
	u8 sat, val;
	bool detect;

	sts = do_init();
	if (XST_SUCCESS != sts) {
		exit(1);
	}

	xil_printf("Starting Main Application\n");
	while (IsExit()) {
		hue = GetHue();
		sat = GetSat();
		val = GetVal();
		UpdateRGBled(hue, sat, val, 0);
		UpdateDispaly(hue, sat, val);
		detect = GetDetectType(); // 0 - Sw Detect; 1- HW Detect
		if (detect) {
			//Hw Detect
			microblaze_disable_interrupts();
			duty_cycle[0] = calc_duty(
					XGpio_DiscreteRead(&GPIOInstR, GPIO_R_INPUT_HIGH_CHANNEL),
					XGpio_DiscreteRead(&GPIOInstR, GPIO_R_INPUT_LOW_CHANNEL));
			duty_cycle[1] = calc_duty(
					XGpio_DiscreteRead(&GPIOInstG, GPIO_G_INPUT_HIGH_CHANNEL),
					XGpio_DiscreteRead(&GPIOInstG, GPIO_G_INPUT_LOW_CHANNEL));
			duty_cycle[2] = calc_duty(
					XGpio_DiscreteRead(&GPIOInstB, GPIO_B_INPUT_HIGH_CHANNEL),
					XGpio_DiscreteRead(&GPIOInstB, GPIO_B_INPUT_LOW_CHANNEL));

		} else {
			//SW Detect
			microblaze_enable_interrupts();
		}

		DisplayDutycycle(duty_cycle[0], duty_cycle[1], duty_cycle[2]);
	}

	// Announce that we're done and clear the LED's
	xil_printf("\nThat's All Folks!\n\n");
	NX4IO_setLEDs(0x00);
	NX4IO_RGBLED_setChnlEn(RGB1, false, false, false);
	NX4IO_RGBLED_setChnlEn(RGB2, false, false, false);
	OLEDrgb_Clear (&pmodOLEDrgb_inst);

	OLEDrgb_PutStringXY(4, 2, "BYE BYE");
	usleep(5000 * 1000);
	// clear the displays and power down the pmodOLEDrbg
	NX410_SSEG_setAllDigits(SSEGHI, CC_BLANK, CC_B, CC_LCY, CC_E, DP_NONE);
	NX410_SSEG_setAllDigits(SSEGLO, CC_B, CC_LCY, CC_E, CC_BLANK, DP_NONE);
	OLEDrgb_Clear(&pmodOLEDrgb_inst);
	OLEDrgb_end(&pmodOLEDrgb_inst);

	// cleanup and exit
	cleanup_platform();
	exit(0);
}

/**************************** INTERRUPT HANDLERS ******************************/

/****************************************************************************/
/**
 * Fixed interval timer interrupt handler
 *
 * Reads the GPIO port which reads back the hardware generated PWM wave for the RGB Leds
 *
 * Calculates the duty cycle at every rising edge
 * Counts low and high signals depending on previous signals
 *****************************************************************************/
void FIT_Handler(void) {
	// Read the GPIO port to read back the generated PWM signal for RGB led's
	gpio_in = XGpio_DiscreteRead(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL);

	// Take value PWM for each color Red, Green Blue and then shift this bit to significant bit
	signal[0] = (gpio_in & 0x04) >> 2;
	signal[1] = (gpio_in & 0x01) >> 0;
	signal[2] = (gpio_in & 0x02) >> 1;

	for (u8 color = 0; color < 3; color++) {
		if (!old_signal[color] && signal[color]) //detect_rising_edge(signal);
				{
			duty_cycle[color] = calc_duty(high_level[color], low_level[color]);
			high_level[color] = 1;
		} else if (old_signal[color] && !signal[color]) //detect_failing_edge(signal);
				{
			low_level[color] = 0;
		} else if (old_signal[color] && signal[color]) {
			high_level[color]++;
			if (high_level[color] == 10000)
				duty_cycle[color] = 99;

		} else if (!old_signal[color] && !signal[color]) {
			low_level[color]++;
			if (low_level[color] == 10000)
				duty_cycle[color] = 0;
		}
		old_signal[color] = signal[color];
	}
}
