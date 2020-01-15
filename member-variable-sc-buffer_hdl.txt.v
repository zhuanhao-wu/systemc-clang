module FIR_0(
input wire clk,
input wire real sample,
output reg real result 
);
 reg real i;
 reg [31:0] myint;
 reg integer mysignal;
always @(negedge clk) begin: behavior
 reg real sum;
 reg integer j;
sum = (c/* definition not found */ [ 0 ]) * (sample);
for(j = 1;(j) <= (ORDER/* definition not found */);j=j+1) begin
  sum = (sum) + ((c/* definition not found */ [ j ]) * (i [ (j) - (1) ]));
end;
result <= sum;
i [ 0 ] <= sample;
for(j = 1;(j) < (ORDER/* definition not found */);j=j+1) begin
  i [ j ] <= i [ (j) - (1) ];
end
end // behavior
endmodule // FIR_0