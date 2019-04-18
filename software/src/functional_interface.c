/**
 *
 * @file ece544ip_test.c
 *
 * @author Roy Kravitz (roy.kravitz@pdx.edu)
 * @author Srivatsa Yogendra (srivatsa@pdx.edu)
 * @copyright Portland State University, 2016-2018
 *
 * This file implements a test program for the Nexys4IO and Digilent Pmod peripherals
 * used in ECE 544. The peripherals provides access to the Nexys4 pushbuttons
 * and slide switches, the LEDs, the RGB LEDs, and the Seven Segment display
 * on the Digilent Nexys4 DDR board and the PmodOLEDrgb (94 x 64 RGB graphics display) 
 * and the PmodENC (rotary encoder + slide switch + pushbutton).
 *
 * The test is basic but covers all of the API functions:
 *	o initialize the Nexys4IO, Pmod drivers and all the other peripherals
 *	o Set the LED's to different values
 *	o Check that the duty cycles can be set for both RGB LEDs
 *	o Write character codes to the digits of the seven segment display banks
 *	o Check that all of the switches and pushbuttons on the Nexys4 can be read
 *	o Performs a basic test on the rotary encoder and pmodOLEDrgb
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a	rhk	02-Jul-2016		First release of test program.  Builds on the ece544 peripheral test used
 *							to check the functionality of Nexys4IO and PMod544IOR2
 * 2.00a sy  14-Oct-2016		Modified the code to include different initialize function for other peripherals 
 *							connected to the system.
 * 3.00	rk	05-Apr-2018		Modified for Digilent PmodENC and PmodOLEDrgb.  Replaced MB_Sleep() w/ usleep.
 * </pre>
 *
 * @note
 * The minimal hardware configuration for this test is a Microblaze-based system with at least 32KB of memory,
 * an instance of Nexys4IO, an instance of the pmodOLEDrgb AXI slave peripheral, and instance of the pmodENC AXI
 * slave peripheral, an instance of AXI GPIO, an instance of AXI timer and an instance of the AXI UARTLite 
 * (used for xil_printf() console output)
 *
 * @note
 * The driver code and test application(s) for the pmodOLDrgb and pmodENC are
 * based on code provided by Digilent, Inc.
 ******************************************************************************/

#include "functional_interface.h"

/* ------------------------------------------------------------ */
/*** HSV to RGB Converter
 **
 **   Parameters:
 **      hue - Hue of color
 **      sat - Saturation of color
 **      val - Value of color
 **
 **   Description:
 **      Converts an HSV value into a 565 RGB color and updates the RGB LED
 */
#if 0
void UpdateRGBled(u8 hue, u8 sat, u8 val)
{
	double hh, p, q, t, ff;
	long i;
	u8 R, G, B;
	float S = sat/100.0;
	float V = val/100.0;
	hh = hue;
	if(hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = V * (1.0 - S);
	q = V * (1.0 - (S * ff));
	t = V * (1.0 - (S * (1.0 - ff)));
	switch(i) {
		case 0:
		R = V * 255;
		G = t * 255;
		B = p * 255;
		break;
		case 1:
		R = q * 255;
		G = V * 255;
		B = p * 255;
		break;
		case 2:
		R = p * 255;
		G = V * 255;
		B = t * 255;
		break;

		case 3:
		R = p * 255;
		G = q * 255;
		B = V * 255;
		break;
		case 4:
		R = t * 255;
		G = p * 255;
		B = V * 255;
		break;
		case 5:
		default:
		R = V * 255;
		G = p * 255;
		B = q * 255;
		break;
	}

	xil_printf("LED's R=%d,G=%d,B=%d\n", R, G, B);

	OLEDrgb_PutStringXY(0,7, "R                    ");
	OLEDrgb_PutIntigerXY(1, 7, R , 10);
	OLEDrgb_PutStringXY(4,7, "G");
	OLEDrgb_PutIntigerXY(5, 7, G , 10);
	OLEDrgb_PutStringXY(8,7, "B");
	OLEDrgb_PutIntigerXY(9, 7, B , 10);
	// For RGB1 may be multiply by 8
	NX4IO_RGBLED_setChnlEn(RGB1, true, true, true);
	NX4IO_RGBLED_setDutyCycle(RGB1, R, G, B);
	// For RGB2
	NX4IO_RGBLED_setChnlEn(RGB2, true, true, true);
	NX4IO_RGBLED_setDutyCycle(RGB2, R, G, B);
	OLEDrgb_DrawRectangle(&pmodOLEDrgb_inst ,50,0,95,103, 0 ,true, OLEDrgb_BuildRGB(R, G, B));
	//h = hue; s = sat; v = val;

}
#endif

void UpdateRGBled(u16 hue, u8 sat, u8 val, bool display) {
	static u16 h = 0;
	static u8 s = 0, v = 0;
	u8 region, p, q, t;
	float remain, S = sat / 100.0, V = val / 100.0;
	u8 R, G, B;
	if (h != hue || s != sat || v != val || display) {
		region = hue / 60;
		remain = ((hue / 60.0) - region);
		p = V * (1.0 - S);
		q = V * (1.0 - (S * remain));
		t = V * (1.0 - (S * (1.0 - remain)));
		V = V * 255;
		p = p * 255;
		q = q * 255;
		t = t * 255;
		switch (region) {
		case 0:
			R = val;
			G = t;
			B = p;
			break;
		case 1:
			R = q;
			G = val;
			B = p;
			break;
		case 2:
			R = p;
			G = val;
			B = t;
			break;
		case 3:
			R = p;
			G = q;
			B = val;
			break;
		case 4:
			R = t;
			G = p;
			B = val;
			break;
		default:
			R = val;
			G = p;
			B = q;
			break;
		}

		// For RGB1
		NX4IO_RGBLED_setChnlEn(RGB1, true, true, true);
		NX4IO_RGBLED_setDutyCycle(RGB1, R, G, B);
		// For RGB2
		NX4IO_RGBLED_setChnlEn(RGB2, true, true, true);
		NX4IO_RGBLED_setDutyCycle(RGB2, R, G, B);

		xil_printf("LED's R=%d,G=%d,B=%d\n", R, G, B);
		OLEDrgb_PutStringXY(0, 7, "R                    ");
		OLEDrgb_PutIntigerXY(1, 7, R, 10);
		OLEDrgb_PutStringXY(4, 7, "G");
		OLEDrgb_PutIntigerXY(5, 7, G, 10);
		OLEDrgb_PutStringXY(8, 7, "B");
		OLEDrgb_PutIntigerXY(9, 7, B, 10);

		OLEDrgb_DrawRectangle(&pmodOLEDrgb_inst, 50, 0, 95, 103,
				OLEDrgb_BuildRGB(R, G, B), true, OLEDrgb_BuildRGB(R, G, B));
		OLEDrgb_DrawRectangle(&pmodOLEDrgb_inst, 50, 0, 95, 103,
				OLEDrgb_BuildRGB(R, G, B), true, OLEDrgb_BuildRGB(R, G, B));
		h = hue;
		s = sat;
		v = val;
	}
}

/** u16 GetHue(void)
 *
 * @return The Hue value for the color
 *
 *  Description:
 *        Updates the Hue value value based on the rotatry encoder turn direction
 */
u16 GetHue(void) {
	u32 state = ENC_getState(&pmodENC_inst);
	;
	static u32 prev_state;
	static int Hue = 0;
	Hue += ENC_getRotation(state, prev_state);
	if (Hue > 360)
		Hue = 0;
	if (Hue < 0)
		Hue = 360;
	prev_state = state;
	return Hue;
}

/**u8 GetSat(void)
 *
 * @return the saturation value for the color
 *
 * Description:
 *       Updates the saturation based on left and right buttons
 *       Uses de-bounce delay for the button press
 */
u8 GetSat(void) {
	static u8 Sat = 0;
	if (NX4IO_isPressed (BTNR)) {
		usleep(90000); // Debounce delay
		if (NX4IO_isPressed(BTNR)) {
			if (Sat == 100)
				Sat = 0;
			else
				Sat++;
		}
	}
	if (NX4IO_isPressed (BTNL)) {
		usleep(90000); // Debounce delay
		if (NX4IO_isPressed(BTNL)) {
			if (Sat > 0)
				Sat--;
			if (Sat == 0)
				Sat = 100;
		}
	}
	return Sat;
}

/**u8 GetVal(void)
 *
 * @return Val for the color
 *
 * Description:
 *       Updates the V value based on up and down buttons
 *       Uses de-bounce delay for the button press
 */
u8 GetVal(void) {
	static u8 Val = 0;
	if (NX4IO_isPressed (BTNU)) {
		usleep(90000); // Debounce delay
		if (NX4IO_isPressed(BTNU)) {
			if (Val == 100)
				Val = 0;
			else
				Val++;
		}
	}
	if (NX4IO_isPressed (BTND)) {
		usleep(90000); // Debounce delay
		if (NX4IO_isPressed(BTND)) {
			if (Val > 0)
				Val--;
			if (Val == 0)
				Val = 100;
		}
	}
	return Val;
}

/**bool GetDetectType(void)
 *
 * @return which type of pwm detection is nedded
 *         1 - for HW detect
 *         2 - for SW detect
 * Description:
 *        Based on the zero switch position to detect which type of PWM detection to use
 *        and turn on and off LED0 based on that.
 */
bool GetDetectType(void) {
	u32 leds_data = NX4IO_getLEDS_DATA();
	if ((NX4IO_getSwitches() & 0x001) == 1) {
		NX4IO_setLEDs(leds_data | (1UL << 0));
		//OLEDrgb_PutStringXY(7,7, "HW" );
		return true;
	} else {
		NX4IO_setLEDs(leds_data & ~(1UL << 0));
		//OLEDrgb_PutStringXY(7,7, "SW" );
		return false;
	}
}

/** bool IsExit(void)
 *
 * @return 1 for running; 0 for exit
 *
 * Description:
 *       Checks the buttons and encoder when to exit
 *       Uses de-bounce delay for the button press
 */
bool IsExit(void) {
	u32 state = ENC_getState(&pmodENC_inst);
	if (ENC_buttonPressed(state)) {
		usleep(5000); // Debounce delay
		if (ENC_buttonPressed(state))
			return 0;
	}

	if (NX4IO_isPressed (BTNC)) {
		usleep(5000); // Debounce delay
		if (NX4IO_isPressed(BTNC))
			return 0;
	}

	return 1;
}

/**
 *
 * @param r_duty Red Duty cycle
 * @param g_duty Green Duty cycle
 * @param b_duty Blue Duty cycle
 *
 * Description:
 *       Displays the duty cycles for rgb on the seven segment Display.
 */
void DisplayDutycycle(u8 r_duty, u8 g_duty, u8 b_duty) {
	static u8 Rduty = 1, Gduty = 0, Bduty = 0;
	if (Rduty != r_duty || Gduty != g_duty || Bduty != b_duty) {
		NX410_SSEG_setAllDigits(2, r_duty / 10, r_duty % 10, 0, g_duty / 10, 0);
		NX410_SSEG_setAllDigits(1, g_duty % 10, 0, b_duty / 10, b_duty % 10, 0);
		Rduty = r_duty;
		Gduty = g_duty;
		Bduty = b_duty;
	}
}

/**
 *
 * @param x x axis for the display
 * @param y y axis for the display
 * @param s string to be displayed
 *
 * Description:
 *        Displays the string at xy location on the pmod oled display
 */
void OLEDrgb_PutStringXY(u8 x, u8 y, char* s) {
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, x, y);
	OLEDrgb_PutString(&pmodOLEDrgb_inst, s);
}

/**
 *
 * @param x x axis for the display
 * @param y y axis for the display
 * @param num Integer to be displayon the screen
 * @param radix to represent on which Format it need to be displayed
 *
 * Description:
 *        Displays the number at xy location on the pmod oled display
 */
void OLEDrgb_PutIntigerXY(u8 x, u8 y, int32_t num, int32_t radix) {
	char buf[16];
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, x, y);
	PMDIO_itoa(num, buf, radix);
	OLEDrgb_PutString(&pmodOLEDrgb_inst, buf);
}

/**
 *
 * @param hue
 * @param sat
 * @param val
 *
 * Description:
 *        Updates the the values of HSV on the display.
 */
void UpdateDispaly(u16 hue, u8 sat, u8 val) {
	static u16 h = 0;
	static u8 s = 0, v = 0;

	if (h != hue || s != sat || v != val) {
		OLEDrgb_PutStringXY(0, 1, "H:   ");
		OLEDrgb_PutIntigerXY(2, 1, hue, 10);
		OLEDrgb_PutStringXY(0, 3, "S:   ");
		OLEDrgb_PutIntigerXY(2, 3, sat, 10);
		OLEDrgb_PutStringXY(0, 5, "V:   ");
		OLEDrgb_PutIntigerXY(2, 5, val, 10);
		//OLEDrgb_DrawRectangle(&pmodOLEDrgb_inst ,50,0,95,103, OLEDrgb_BuildHSV(h,s,v) ,true, OLEDrgb_BuildHSV(h,s,v));
		h = hue;
		s = sat;
		v = val;
	}

}

/**
 *
 * @param high count of the pwm
 * @param low count of the pwm
 * @return
 *
 * Description:
 *        Caluculates the Duty cycle based on the counts
 */
u8 calc_duty(u32 high, u32 low) {
	static u32 h = 1, l = 1;
	u32 sum;
	static u8 duty;
	if (h != high || l != low) {
		h = high;
		l = low;

		sum = (high) + (low);
		duty = (100 * (high)) / sum;
		duty = duty * 2;
		if (duty < 0)
			duty = 0;
		if (duty > 99)
			duty = 99;

		return duty;
	}
	return duty;

}
