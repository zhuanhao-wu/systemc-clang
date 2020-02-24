interface AXI4(input aclk);
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

  modport raddr();
  modport rdata();

  modport waddr();
  modport wdata();
  modport bresp();

  modport rchannel();
  modport wchannel();
endinterface

module AXI4MasterDataGenerator(

);
endmodule
module AXI4Master(
);
endmodule
