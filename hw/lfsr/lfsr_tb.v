`timescale 1ns/1ps

module lfsr16_tb();
    localparam CLK_PERIOD = 2;
    //reg [15:0] rand_num;
    reg clk;
    reg rst_b;
    wire [15:0] in;
    lfsr16 dut(
        .rst_b(rst_b),
        .clk(clk),
        .LFSR(in)
    );
        
    always #(CLK_PERIOD/2) clk = ~clk;
    
    initial begin
        clk = 1'b1;
        rst_b = 0;

        #2
        rst_b = 1;
         
    end

endmodule