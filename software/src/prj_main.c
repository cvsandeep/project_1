/*
 * prj_main.c
 *
 *  Created on: Apr 9, 2019
 *      Author: venksand
 */

#include "hw_interface.h"
#include "functional_interface.h"

uint64_t 	timestamp = 0L;
volatile u8 duty_cycle[3] = {1,2,3};

/************************** MAIN PROGRAM ************************************/
int main(void)
{
    init_platform();

	uint32_t sts;
	u8 hue, sat, val;
	bool detect;


	sts = do_init();
	if (XST_SUCCESS != sts)
	{
		exit(1);
	}

	//microblaze_enable_interrupts();

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
	NX4IO_setLEDs(0x00);
	OLEDrgb_SetFontColor(&pmodOLEDrgb_inst ,OLEDrgb_BuildHSV(255,255,255));  // blue font
	while ( IsExit() )
	{
		//state = ENC_getState(&pmodENC_inst);
		//hue += ENC_getRotation(state, laststate);
		hue = GetHue();
		sat = GetSat();
		val = GetVal();
		UpdateRGBled(hue, sat, val);
		UpdateDispaly(hue, sat, val);
		//UpdatePmodDispaly();
		//xil_printf("Hue:%d  Sat:%d val:%d\n", hue, sat, val);
		detect = GetDetectType(); // 0 - Sw Detect; 1- HW Detect

		if(detect)
		{
			//Hw Detect
			microblaze_disable_interrupts();
			//UpdateHWDutyCycle();
			duty_cycle[0] = XGpio_DiscreteRead(&GPIOInstR, GPIO_R_INPUT_LOW_CHANNEL);
		    duty_cycle[0] = (duty_cycle[0] * 100)/(duty_cycle[0] + XGpio_DiscreteRead(&GPIOInstR, GPIO_R_INPUT_HIGH_CHANNEL));

		    duty_cycle[1] = XGpio_DiscreteRead(&GPIOInstR, GPIO_G_INPUT_LOW_CHANNEL);
		    duty_cycle[1] = (duty_cycle[1] * 100)/(duty_cycle[1] + XGpio_DiscreteRead(&GPIOInstR, GPIO_G_INPUT_HIGH_CHANNEL));

		    duty_cycle[2] = XGpio_DiscreteRead(&GPIOInstR, GPIO_B_INPUT_LOW_CHANNEL);
		    duty_cycle[2] = (duty_cycle[2] * 100)/(duty_cycle[2] + XGpio_DiscreteRead(&GPIOInstR, GPIO_B_INPUT_HIGH_CHANNEL));


		} else
		{
			//SW Detect
			microblaze_enable_interrupts();
			//UpdateSWDutyCycle();
		}
		//NX4IO_setLEDs(XGpio_DiscreteRead(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL));

		DisplayDutycycle(duty_cycle[0], duty_cycle[1], duty_cycle[2]);
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

#define rising_detect 1
#define falling_detect 2
#define duty_calc 3

volatile bool signal[3];
volatile bool old_signal[3];
volatile u8 type = 0;
volatile u8 high_level[3];
volatile u8 low_level[3];

void FIT_Handler(void)
{
	// Read the GPIO port to read back the generated PWM signal for RGB led's
	gpio_in = XGpio_DiscreteRead(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL);

	// Take value PWM for each color Red, Green Blue and then shift this bit to significant bit
	signal[0] = (gpio_in & 0x04) >> 2;
	signal[1] = (gpio_in & 0x01) >> 0;
	signal[2] = (gpio_in & 0x02) >> 1;

	for (u8 color=0; color < 3; color++)
	{
		if(!old_signal[color] && signal[color]) //detect_rising_edge(signal);
		{
			duty_cycle[color] = (high_level[color]*100)/(high_level[color]+low_level[color]);
			high_level[color] = 1;
		}
		else if(old_signal[color] && !signal[color]) //detect_failing_edge(signal);
		{
			low_level[color] = 0;
		}
		else if(old_signal[color] && signal[color])
		{
			high_level[color]++;
		}
		else if(!old_signal[color] && !signal[color])
		{
			low_level[color]++;
		}
		old_signal[color] = signal[color];
		//xil_printf("signal%d: %d \n", color, signal[color]);

	}
}
