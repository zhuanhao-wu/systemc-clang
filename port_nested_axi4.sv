
interface AXI4ReadChannel(input aclk);
  // read address channel
  logic       arready;
  logic       arvalid;
  logic       arlen;
  logic       arsize;
  logic[31:0] araddr;
  // read data
  logic       rready;
  logic       rvalid;
  logic[31:0] rdata;
  logic[31:0] rresp;
endinterface

interface AXI4WriteChannel(input aclk);
  // write address channel
  logic       awready;
  logic       awvalid;
  logic       awlen;
  logic       awsize;
  logic[31:0] awaddr;
  // write data
  logic       wready;
  logic       wvalid;
  logic[31:0] wdata;
  logic[3:0]  wstrb;
  // write resp

endinterface

interface AXI4(input aclk);
  AXI4ReadChannel rchannel(aclk);
  AXI4WriteChannel wchannel(aclk);
endinterface

module AXIMaster();
endmodule

