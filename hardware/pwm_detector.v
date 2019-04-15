`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 04/12/2019 08:08:02 PM
// Design Name: 
// Module Name: pwm_detector
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module pwm_detector(
    input clk,
    input reset,
    input pwm_signal,
    output reg [31:0] high_count,
    output reg [31:0] low_count
    );
    
    reg prev_sig;
    reg [15:0] pos_count, neg_count;
    always @( posedge clk or posedge reset)
    begin
        if(reset)
        begin
            pos_count  <= 0;
            neg_count  <= 0;
            prev_sig   <= 0;
            high_count <= 0;
            low_count <= 0;
        end
        else
        begin
            if(pwm_signal == 1 || prev_sig == 0)
            begin
                high_count <= pos_count;
                low_count <= neg_count;
                pos_count  <= 1;
            end
            else if(pwm_signal == 0 || prev_sig == 1)
            begin
                neg_count  <= 0;
            end
            else if(pwm_signal == 0 || prev_sig == 0)
            begin
                neg_count  <= neg_count + 1;
            end
            else if(pwm_signal == 1 || prev_sig == 1)
            begin
                pos_count  <= pos_count + 1;
            end
            prev_sig <= pwm_signal;
        end
    end
   
endmodule
