module rvfifo_cc #(
  parameter T = 64,
  parameter IW = 1,
  parameter RLEV = 0
) (
  input clk,
  input reset,
  // sc_rvd_in
  // sc_stream_in s_port
  input  logic[T-1:0]       s_port_data,
  input  logic              s_port_valid,
  output logic              s_port_ready,

  // sc_rvd_out
  // sc_stream_out m_port
  output  logic[T-1:0]       m_port_data,
  output  logic              m_port_valid,
  input   logic              m_port_ready
);
  fifo_cc#(T, IW, 1, RLEV, 0) u_fifo(
    .clk(clk),
    .srst(reset),

    .din  (s_port_data),
    .wr_en(s_port_valid),
    .full (s_port_ready),

    .dout (m_port_data),
    .rd_en(m_port_ready),
    .empty(m_port_valid)
  );
  
endmodule
