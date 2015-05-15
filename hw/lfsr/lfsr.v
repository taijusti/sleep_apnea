`timescale 1ns/1ps

module lfsr16(
input rst_b, /*rst_b is necessary to prevet locking up*/
input clk, /*clock signal*/
output reg [15:0] LFSR = 16'hABCD /*random number output*/
);

wire feedback = LFSR[15];

always@(posedge clk or negedge rst_b)
    begin
        if(!rst_b)
            LFSR <=16'h0000;
        else begin
            LFSR[0] <= feedback;
            LFSR[1] <= LFSR[0];
            LFSR[2] <= LFSR[1];
            LFSR[3] <= LFSR[2];
            LFSR[4] <= LFSR[3] ~^ feedback;
            LFSR[5] <= LFSR[4];
            LFSR[6] <= LFSR[5];
            LFSR[7] <= LFSR[6];
            LFSR[8] <= LFSR[7];
            LFSR[9] <= LFSR[8];
            LFSR[10] <= LFSR[9];
            LFSR[11] <= LFSR[10];
            LFSR[12] <= LFSR[11];
            LFSR[13] <= LFSR[12] ~^ feedback;
            LFSR[14] <= LFSR[13];
            LFSR[15] <= LFSR[14] ~^ feedback;
        end
    end
endmodule