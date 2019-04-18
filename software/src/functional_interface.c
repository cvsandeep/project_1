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

#include "hw_interface.h"
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
    double      hh, p, q, t, ff;
    long        i;
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
   float remain, S=sat/100.0, V = val/100.0;
   u8 R, G, B;
   if( h != hue || s != sat || v != val || display)
   {
	   region = hue / 60;
	   remain = ( (hue/60.0) - region );
	   p = V * (1.0 - S);
	   q = V * (1.0 - (S * remain));
	   t = V * (1.0 - (S * (1.0 - remain)));
	   V = V*255; p=p*255; q=q*255; t=t*255;
	   switch (region) {
	   case 0:
		  R = val; G = t; B = p;
		  break;
	   case 1:
		  R = q; G = val; B = p;
		  break;
	   case 2:
		  R = p; G = val; B = t;
		  break;
	   case 3:
		  R = p; G = q; B = val;
		  break;
	   case 4:
		  R = t; G = p; B = val;
		  break;
	   default:
		  R = val; G = p; B = q;
		  break;
	   }
	   R = (R*255)/100; G = (G*255)/100; B = (B*255)/100;

	   // For RGB1
		NX4IO_RGBLED_setChnlEn(RGB1, true, true, true);
		NX4IO_RGBLED_setDutyCycle(RGB1, R, G, B);
		// For RGB2
		NX4IO_RGBLED_setChnlEn(RGB2, true, true, true);
		NX4IO_RGBLED_setDutyCycle(RGB2, R, G, B);

		xil_printf("LED's R=%d,G=%d,B=%d\n", R, G, B);
	   OLEDrgb_PutStringXY(0,7, "R                    ");
	   OLEDrgb_PutIntigerXY(1, 7, R , 10);
	   OLEDrgb_PutStringXY(4,7, "G");
	   OLEDrgb_PutIntigerXY(5, 7, G , 10);
	   OLEDrgb_PutStringXY(8,7, "B");
	   OLEDrgb_PutIntigerXY(9, 7, B , 10);

		OLEDrgb_DrawRectangle(&pmodOLEDrgb_inst ,50,0,95,103, OLEDrgb_BuildRGB(R, G, B) ,true, OLEDrgb_BuildRGB(R, G, B));
		OLEDrgb_DrawRectangle(&pmodOLEDrgb_inst ,50,0,95,103, OLEDrgb_BuildRGB(R, G, B) ,true, OLEDrgb_BuildRGB(R, G, B));
		h = hue; s = sat; v = val;
   }
}
//#endif

u16 GetHue(void)
{
	u32 state = ENC_getState(&pmodENC_inst);;
	static u32 prev_state;
	static int Hue = 0;
	Hue += ENC_getRotation(state, prev_state);
	if (Hue > 360) Hue = 0;
	if (Hue < 0) Hue = 360;
	prev_state = state;
	return Hue;
}

u8 GetSat(void)
{
	static u8 Sat = 0;
		if (NX4IO_isPressed(BTNR))
		{
			usleep(90000); // Debounce delay
			if (NX4IO_isPressed(BTNR))
			{
				if (Sat == 100)
					Sat = 0;
				else
					Sat++;
			}
		}
		if (NX4IO_isPressed(BTNL))
		{
			usleep(90000); // Debounce delay
			if (NX4IO_isPressed(BTNL))
			{
				if (Sat > 0)
					Sat--;
				if(Sat == 0 )
					Sat = 100;
			}
		}
	return Sat;
}

u8 GetVal(void)
{
	static u8 Val = 0;
	if (NX4IO_isPressed(BTNU))
	{
		usleep(90000); // Debounce delay
		if (NX4IO_isPressed(BTNU))
		{
			if (Val == 100)
				Val = 0;
			else
				Val++;
		}
	}
	if (NX4IO_isPressed(BTND))
	{
		usleep(90000); // Debounce delay
		if (NX4IO_isPressed(BTND))
		{
			if (Val > 0)
				Val--;
			if (Val  == 0)
				Val = 100;
		}
	}
	return Val;
}

bool GetDetectType(void)
{
	u32 leds_data = NX4IO_getLEDS_DATA();
	if((NX4IO_getSwitches() & 0x001) == 1 )
	{
		NX4IO_setLEDs(leds_data | (1UL << 0));
		//OLEDrgb_PutStringXY(7,7, "HW" );
		return true;
	} else
	{
		NX4IO_setLEDs(leds_data & ~(1UL << 0));
		//OLEDrgb_PutStringXY(7,7, "SW" );
		return false;
	}
}

bool IsExit(void)
{
	u32 state = ENC_getState(&pmodENC_inst);
	if (ENC_buttonPressed(state) )
	{
		usleep(5000); // Debounce delay
		if (ENC_buttonPressed(state) )
		return 0;
	}

	if (NX4IO_isPressed(BTNC))
	{
		usleep(5000); // Debounce delay
		if (NX4IO_isPressed(BTNC))
		return 0;
	}

	return 1;
}

void DisplayDutycycle(u8 r_duty, u8 g_duty, u8 b_duty)
{
	static u8 Rduty = 1, Gduty = 0, Bduty = 0;
	if( Rduty != r_duty || Gduty != g_duty || Bduty != b_duty)
	{
		NX410_SSEG_setAllDigits(2,r_duty/10, r_duty%10, 0, g_duty/10,0);
		NX410_SSEG_setAllDigits(1, g_duty%10, 0, b_duty/10 , b_duty%10, 0);
		Rduty = r_duty; Gduty = g_duty; Bduty = b_duty;
	}
}

void OLEDrgb_PutStringXY(u8 x, u8 y, char* s)
{
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, x, y);
	OLEDrgb_PutString(&pmodOLEDrgb_inst, s);
}

void OLEDrgb_PutIntigerXY(u8 x, u8 y, int32_t num, int32_t radix)
{
	char  buf[16];
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, x, y);
    PMDIO_itoa(num, buf, radix);
	OLEDrgb_PutString(&pmodOLEDrgb_inst, buf);
}

void UpdateDispaly(u16 hue, u8 sat, u8 val)
{
	static u16 h = 0;
	static u8 s = 0, v = 0;

    if( h != hue || s != sat || v != val)
    {
    	OLEDrgb_PutStringXY(0,1, "H:   " );
    	OLEDrgb_PutIntigerXY(2, 1, hue , 10);
    	OLEDrgb_PutStringXY(0,3, "S:   " );
    	OLEDrgb_PutIntigerXY(2, 3, sat , 10);
    	OLEDrgb_PutStringXY(0,5, "V:   " );
    	OLEDrgb_PutIntigerXY(2, 5, val , 10);
    	//OLEDrgb_DrawRectangle(&pmodOLEDrgb_inst ,50,0,95,103, OLEDrgb_BuildHSV(h,s,v) ,true, OLEDrgb_BuildHSV(h,s,v));
    	h = hue; s = sat; v = val;
    }

}

u8 calc_duty(u32 high, u32 low)
{
	static u32 h=1,l=1;
	u32 sum;
	static u8 duty;
	if( h != high || l != low )
	{
		//xil_printf("High count= %d, low count = %d\n", high, low);
		 h = high; l = low;

		sum = (high) + (low);
		duty = (100 * (high)) / sum;
        duty = duty * 2;
		if(duty < 0) duty = 0;
		if(duty > 99) duty = 99;

		return duty;
	}
	return duty;

};

/************************ TEST FUNCTIONS ************************************/

/****************************************************************************/
/**
* Test 1 - Test the LEDS (LD15..LD0)
*
* Checks the functionality of the LEDs API with some constant patterns and
* a sliding patter.  Determine Pass/Fail by observing the LEDs on the Nexys4
*
* @param	*NONE*
*
* @return	*NONE*
*
*****************************************************************************/

void RunTest1(void)
{
	uint16_t ledvalue;

	xil_printf("Starting Test 1...the LED test\n");
	// test the LEDS (LD15..LD0) with some constant patterns
	NX4IO_setLEDs(0x00005555);
	usleep(1000 * 1000);
	NX4IO_setLEDs(0x0000AAAA);
	usleep(1000 * 1000);
	NX4IO_setLEDs(0x0000FF00);
	usleep(1000 * 1000);
	NX4IO_setLEDs(0x000000FF);
	usleep(1000 * 1000);

	// shift a 1 through all of the leds
	ledvalue = 0x0001;
	do
	{
		NX4IO_setLEDs(ledvalue);
		usleep(500 * 1000);
		ledvalue = ledvalue << 1;
	} while (ledvalue != 0);
	return;
}


/****************************************************************************/
/**
* Test 2 - Test the RGB LEDS (LD17..LD16)
*
* Checks the functionality of the RGB LEDs API with a fixed duty cycle.
* Determine Pass/Fail by observing the RGB LEDs on the Nexys4.
*
* @param	*NONE*
*
* @return	*NONE*
*
*****************************************************************************/
void RunTest2(void)
{
	// Test the RGB LEDS (LD17..LD16)
	xil_printf("Starting Test 2...the RGB LED test\n");

	// For RGB1 turn on only the blue LED (e.g. Red and Green duty cycles
	// are set to 0 but enable all three PWM channels
	NX4IO_RGBLED_setChnlEn(RGB1, true, true, true);
	NX4IO_RGBLED_setDutyCycle(RGB1, 0, 0, 16);
	usleep(1500 * 1000);

	// For RGB2, only write a non-zero duty cycle to the green channel
	NX4IO_RGBLED_setChnlEn(RGB2, true, true, true);
	NX4IO_RGBLED_setDutyCycle(RGB2, 0, 32, 0);
	usleep(1500 * 1000);

	// Next make RGB1 red. This time we'll only enable the red PWM channel
	NX4IO_RGBLED_setChnlEn(RGB1, true, false, false);
	NX4IO_RGBLED_setDutyCycle(RGB1, 64, 64, 64);
	usleep(1500 * 1000);

	// Next make RGB2 BRIGHTpurple-ish by only changing the duty cycle
	NX4IO_RGBLED_setDutyCycle(RGB2, 255, 255, 255);
	usleep(1500 * 1000);

	// Finish by turning the both LEDs off
	// We'll do this by disabling all of the channels without changing
	// the duty cycles
	NX4IO_RGBLED_setDutyCycle(RGB1, 0, 0, 0);
	NX4IO_RGBLED_setDutyCycle(RGB2, 0, 0, 0);

	return;
}

/****************************************************************************/
/**
* Test 3 - Test the seven segment display
*
* Checks the seven segment display by displaying DEADBEEF and lighting all of
* the decimal points. Determine Pass/Fail by observing the seven segment
* display on the Nexys4.
*
* @param	*NONE*
*
* @return	*NONE*
*
*****************************************************************************/
void RunTest3(void)
{
	xil_printf("Starting Test 3...The seven segment display test\n");

	NX4IO_SSEG_putU32Hex(0xBEEFDEAD);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT7, true);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT6, true);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT5, true);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT4, true);
	usleep(2500 * 1000);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT7, false);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT6, false);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT5, false);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT4, false);

	NX4IO_SSEG_putU32Hex(0xDEADBEEF);
	NX4IO_SSEG_setDecPt(SSEGLO, DIGIT3, true);
	NX4IO_SSEG_setDecPt(SSEGLO, DIGIT2, true);
	NX4IO_SSEG_setDecPt(SSEGLO, DIGIT1, true);
	NX4IO_SSEG_setDecPt(SSEGLO, DIGIT0, true);
	usleep(2500 * 1000);
	return;
}


/****************************************************************************/
/**
* Test 4 - Test the PmodENC and PmodOLEDrgb
*
* The rotary encoder portion of this test is taken from the Digilent PmodENC driver example
*
* Performs some basic tests on the PmodENC and PmodOLEDrgb.  Includes the following tests
* 	1.	check the rotary encoder by displaying the rotary encoder
* 		count in decimal and hex on the LCD display.  Rotate the knob
* 		to change the values up or down.  The pushbuttons can be used
* 		as follows:
* 			o 	press the rotary encoder pushbutton to exit
* 			o 	press BtnUp to clear the count
*
*	6.	display the string "357#&CFsw" on the LCD display.  These values
* 		were chosen to check that the bit order is correct.  The screen will
* 		clear in about 5 seconds.
* 	7.	display "Looks Good" on the display.  The screen will clear
* 		in about 5 seconds.
*
*
* @param	*NONE*
*
* @return	*NONE*
*
*****************************************************************************/
void RunTest4(void)
{
	u32 state, laststate; //comparing current and previous state to detect edges on GPIO pins.
	u32 ticks = 0, lastticks = 1;

	// Set up the display output
	OLEDrgb_Clear(&pmodOLEDrgb_inst);
	OLEDrgb_SetFontColor(&pmodOLEDrgb_inst,OLEDrgb_BuildRGB(200, 12, 44));
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 0, 1);
	OLEDrgb_PutString(&pmodOLEDrgb_inst,"Enc:");
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 0, 4);
	OLEDrgb_PutString(&pmodOLEDrgb_inst,"Hex:");

	// get the previous state
	laststate = ENC_getState(&pmodENC_inst);
	while(IsExit()) {
		// get the PmodENC state
		state = ENC_getState(&pmodENC_inst);

		// check if the rotary encoder pushbutton or BTNC is pressed
		// exit the loop if either one is pressed.
//		if (ENC_buttonPressed(state) && !ENC_buttonPressed(laststate))//only check on button posedge
	//	{
		//	break;
	//	}

		if (NX4IO_isPressed(BTNC))
		{
			break;
		}

		// check BTNU and clear count if it's pressed
		// update the count if it is not

			// BTNU is not pressed so increment count
		ticks = GetHue();
		//xil_printf("ticks before:%d \n", ticks);
		ticks += ENC_getRotation(state, laststate);
		//xil_printf("ticks after:%d \n", ticks);

		// update the display with the new count if the count has changed
		if (ticks != lastticks) {
			OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 4, 1);
			OLEDrgb_PutString(&pmodOLEDrgb_inst,"         ");
			OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 4, 1);
			PMDIO_putnum(&pmodOLEDrgb_inst, ticks, 10);

			OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 2, 5);
			OLEDrgb_PutString(&pmodOLEDrgb_inst,"         ");
			OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 2, 5);
			PMDIO_puthex(&pmodOLEDrgb_inst, ticks);

			// display the count on the LEDs and seven segment display while we're at it
			NX4IO_setLEDs(ticks);
			NX4IO_SSEG_putU32Dec(ticks, true);
		}

		laststate = state;
		lastticks = ticks;
		usleep(1000);
	} // rotary button has been pressed - exit the loop
	xil_printf("\nPmodENC test completed\n");

	// Write some character to the screen to check the ASCII translation
 	OLEDrgb_Clear(&pmodOLEDrgb_inst);
 	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 0, 7);
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, '3');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, '5');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, '7');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, '#');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, '&');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, 'C');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, 'F');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, 's');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, 'w');
	usleep(2500 * 1000);

	// Write one final string
	OLEDrgb_Clear(&pmodOLEDrgb_inst);
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 0, 4);
	//OLEDrgb_PutString(&pmodOLEDrgb_inst, s);

	return;
}



/****************************************************************************/
/**
* Test 5 - Test the pushbuttons and switches
*
* Copies the slide switch values to LEDs and the pushbuttons to the decimal
* points in SSEGLO.  Also shows shows the value of the switches on SSEGLO.
* Doing this not only tests the putU16HEX() function but also lets the user
* try all of the character codes (they are displayed on DIGIT7. Determine
* Pass/Fail by flipping switches and pressing
* buttons and seeing if the results are reflected in the LEDs and decimal points
*
* @param	*NONE*
*
* @return	*NONE*
*
* @note
* This function does a single iteration. It should be inclosed in a loop
* if you want to repeat the test
*
*****************************************************************************/
void RunTest5()
{
	uint16_t ledvalue;
	uint8_t  btnvalue;
	uint32_t regvalue;
	uint32_t ssegreg;

	// read the switches and write them to the LEDs and SSEGLO
	ledvalue = NX4IO_getSwitches();
	NX4IO_setLEDs(ledvalue);
	NX4IO_SSEG_putU16Hex(SSEGLO, ledvalue);

	// write sw[4:0] as a character code to digit 7 so we can
	// check that all of the characters are displayed correctly
	NX4IO_SSEG_setDigit(SSEGHI, DIGIT7, (ledvalue & 0x1F));

	// read the buttons and write them to the decimal points on SSEGHI
	// use the raw get and put functions for the seven segment display
	// to test them and to keep the functionality simple.  We want to
	// ignore the center button so mask that out while we're at it.
	btnvalue = NX4IO_getBtns() & 0x01F;
	ssegreg = NX4IO_SSEG_getSSEG_DATA(SSEGHI);

	// shift the button value to bits 27:24 of the SSEG_DATA register
	// these are the bits that light the decimal points.
	regvalue = ssegreg & ~NEXYS4IO_SSEG_DECPTS_MASK;
	regvalue  |=  btnvalue << 24;

	// write the SSEG_DATA register with the new decimal point values
	NX4IO_SSEG_setSSEG_DATA(SSEGHI, regvalue);
	return;
}
