module avg_0(
input wire clk,
input wire [7:0] i_data,
output reg [7:0] o_data 
);
 reg integer coef1;
 reg integer coef2;
 reg integer coef3;
 reg integer prod1;
 reg integer prod2;
 reg integer prod3;
 reg [7:0] sum2;
 reg [7:0] sum3;
 reg [7:0] tap0;
 reg [7:0] tap1;
 reg [7:0] tap2;
 reg [7:0] tap3;
always @(posedge clk or i_data) begin: first_function

tap0 <= i_data
end // first_function

always @(posedge clk or i_data) begin: second_function

tap1 <= tap0;
tap2 <= tap1;
tap3 <= tap2
end // second_function

always @(posedge clk or i_data) begin: third_function
 reg [7:0] tmpTap1;
 reg [7:0] tmpTap2;
 reg [7:0] tmpTap3;
 reg integer result2;
 reg integer result3;
coef1 = 2;
coef2 = 2;
coef3 = 2;
tmpTap1 = tap1;
tmpTap2 = tap2;
tmpTap3 = tap3;
prod1 = (tmpTap1) * (coef1);
prod2 = (tmpTap2) * (coef2);
prod3 = (tmpTap3) * (coef3);
result2 = (prod1) + (prod2);
result3 = ((prod1) + (prod2)) + (prod3);
sum2 <= result2;
sum3 <= result3;
o_data = sum3
end // third_function
endmodule // avg_0
module driver_0(
output reg filter_clk,
output reg [7:0] filter_input 
);


always @(*) begin: filter_clk_driver

filter_clk = !(filter_clk);
#4
end // filter_clk_driver
endmodule // driver_0
module monitor_0(
input wire [7:0] filter_input,
input wire [7:0] filter_output 
);


always @(filter_input or filter_output) begin: prc_monitor


end // prc_monitor
endmodule // monitor_0