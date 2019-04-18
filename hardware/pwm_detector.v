module pwm_detector #(

	/******************************************************************/
	/* Parameter declarations						                  */
	/******************************************************************/

	// Define some timing parameters

	parameter integer 	CLK_FREQUENCY_HZ = 100000000)

	/******************************************************************/
	/* Port declarations							                  */
	/******************************************************************/

	(				
	input 					clk,			// 100MHz system clock
	input 			 		reset,			// active-high reset signal from Nexys4
	input 					pwm_signal,			// PWM signal from AXI Timer in EMBSYS

	output reg	[31:0]		high_count,		// how long PWM was 'high' --> GPIO input on Microblaze
	output reg	[31:0]		low_count);		// how long PWM was 'low' --> GPIO input on Microblaze

	/******************************************************************/
	/* Local parameters and values		                  	  		  */
	/******************************************************************/

	reg			[31:0]		hcount,lcount;			// 32-bit counter used for high/low count intervals
	reg 					prev_pwm; 		// previous state of PWM; used to detect transitions

	/******************************************************************/
	/* Obtain the counts for high & low intervals	                  */
	/******************************************************************/

	always@(posedge clk) begin

		if (reset) begin					// check for synchronous reset

			lcount <= 32'b0;
			hcount <= 32'b0;					// clear the counter
			high_count <= 32'b0;			// clear the 'high' register
			low_count <= 32'b0;				// clear the 'low' register
			prev_pwm <= 1'b0;				// clear the previous state

		end

		else 
		begin
		    if (prev_pwm && pwm_signal) begin 		// if so, check whether there was a high-to-low transition
				hcount <= hcount + 1; 		// store the 'high' count
			end  
			else if(prev_pwm == 0 && pwm_signal == 0)
			begin
			     lcount <= lcount + 1;
			     if(lcount == 32'h00FFFFF)
			     begin
			         low_count <= 32'b0;
			         high_count <= 32'b0;
			     end
			end
			else if (prev_pwm == 0 && pwm_signal == 1)
			begin
			     high_count <= hcount;
			     low_count <= lcount;
			     hcount <= 1;
			
			end
			else if (prev_pwm == 1 && pwm_signal == 0)
			begin
			     lcount <= 1;
			
			end
		
		     prev_pwm <= pwm_signal;
		
		end

	end

endmodule