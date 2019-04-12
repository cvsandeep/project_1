/*
 * prj_main.c
 *
 *  Created on: Apr 9, 2019
 *      Author: venksand
 */

#include "hw_interface.h"
#include "functional_interface.h"

uint64_t 	timestamp = 0L;

/************************** MAIN PROGRAM ************************************/
int main(void)
{
    init_platform();

	uint32_t sts;
	u8 hue, sat, val;
	u8 r_duty = 12, g_duty = 34, b_duty = 56;
	bool detect;


	sts = do_init();
	if (XST_SUCCESS != sts)
	{
		exit(1);
	}

	microblaze_enable_interrupts();

	xil_printf("ECE 544 Getting Started Application\n\r");
	xil_printf("By Roy Kravitz. 05-April-2018\n\n\r");


	// TEST 1 - Test the LD15..LD0 on the Nexys4
	//RunTest1();
	// TEST 2 - Test RGB1 (LD16) and RGB2 (LD17) on the Nexys4
	//RunTest2();
	// TEST 3 - test the seven segment display banks
	RunTest3();
	// TEST 4 - test the rotary encoder (PmodENC) and display (PmodOLEDrgb)
	RunTest4();

	// TEST 5 - test the switches and pushbuttons
	// We will do this in a busy-wait loop
	// pressing BTN_C (the center button will
	// cause the loop to terminate
	timestamp = 0;

	xil_printf("Starting Test 5...the buttons and switch test\n");
	xil_printf("Press the center pushbutton to exit\n");

	// blank the display digits and turn off the decimal points
	NX410_SSEG_setAllDigits(SSEGLO, CC_BLANK, CC_BLANK, CC_BLANK, CC_BLANK, DP_NONE);
	NX410_SSEG_setAllDigits(SSEGHI, CC_BLANK, CC_BLANK, CC_BLANK, CC_BLANK, DP_NONE);
	// loop the test until the user presses the center button
	while (0)
	{
		// Run an iteration of the test
		RunTest5();
		// check whether the center button is pressed.  If it is then
		// exit the loop.
		if (NX4IO_isPressed(BTNC))
		{
			// show the timestamp on the seven segment display and quit the loop
			NX4IO_SSEG_putU32Dec((uint32_t) timestamp, true);
			break;
		}
		else
		{
			// increment the timestamp and delay 100 msecs
			timestamp += 100;
			usleep(100 * 1000);
		}
	}
	xil_printf("Starting Main Application\n");
	laststate = ENC_getState(&pmodENC_inst);
	OLEDrgb_SetFontColor(&pmodOLEDrgb_inst ,OLEDrgb_BuildHSV(255,255,255));  // blue font
	while ( IsExit() )
	{
		//state = ENC_getState(&pmodENC_inst);
		//hue += ENC_getRotation(state, laststate);
		hue = GetHue();
		sat = GetSat();
		val = GetVal();
		UpdateRGBled(hue, sat, val);
		OLEDrgb_PutStringXY(0,1, "Hue:" );
		OLEDrgb_PutIntigerXY(4, 1, hue , 10);
		OLEDrgb_PutStringXY(0,3, "Sat:" );
		OLEDrgb_PutIntigerXY(4, 3, sat , 10);
		OLEDrgb_PutStringXY(0,5, "Val:" );
		OLEDrgb_PutIntigerXY(4, 5, val , 10);
		//UpdatePmodDispaly();
		//xil_printf("Hue:%d  Sat:%d val:%d\n", hue, sat, val);
		detect = GetDetectType(); // 0 - Sw Detect; 1- HW Detect

		if(detect)
		{
			//Hw Detect
			microblaze_disable_interrupts();
			//UpdateLed();
			//UpdateHWDutyCycle();

		} else
		{
			//SW Detect
			microblaze_enable_interrupts();
			//UpdateLed();
			//UpdateSWDutyCycle();
		}

		DisplayDutycycle(r_duty, g_duty, b_duty);
		laststate = state;
		//usleep(1000);
	}

	// announce that we're done
	xil_printf("\nThat's All Folks!\n\n");
	OLEDrgb_Clear(&pmodOLEDrgb_inst);
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 4, 2);
	OLEDrgb_SetFontColor(&pmodOLEDrgb_inst ,OLEDrgb_BuildRGB(0, 0, 255));  // blue font
	OLEDrgb_PutString(&pmodOLEDrgb_inst,"BYE BYE");

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
* @note
* ECE 544 students - When you implement your software solution for pulse width detection in
* Project 1 this could be a reasonable place to do that processing.
 *****************************************************************************/

void FIT_Handler(void)
{
	// Read the GPIO port to read back the generated PWM signal for RGB led's
	gpio_in = XGpio_DiscreteRead(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL);
}
