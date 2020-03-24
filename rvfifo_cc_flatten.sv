module rvfifo_cc #(
  parameter depth = 2,
  parameter E = 0,
  parameter F = 0
) (
  input clk,
  input reset,
  // sc_rvd_in
  // sc_stream_in s_port
  input  logic[F-1:0] s_port_data_frac,
  input  logic[E-1:0] s_port_data_expo,
  input  logic        s_port_data_sign,
  input  logic              s_port_valid,
  output logic              s_port_ready,

  // sc_rvd_out
  // sc_stream_out m_port
  output  logic[F-1:0] m_port_data_frac,
  output  logic[E-1:0] m_port_data_expo,
  output  logic        m_port_data_sign,
  output  logic              m_port_valid,
  input   logic              m_port_ready
);
  fifo_cc#(depth, E, F, 16, 1, 0, 0) u_fifo(
    .clk(clk),
    .srst(reset),

    .din_frac  (s_port_data_frac),
    .din_expo  (s_port_data_expo),
    .din_sign  (s_port_data_sign),
    .wr_en(s_port_valid),
    .full (s_port_ready),

    .dout_frac (m_port_data_frac),
    .dout_expo (m_port_data_expo),
    .dout_sign (m_port_data_sign),
    .rd_en(m_port_ready),
    .empty(m_port_valid)
  );
  
endmodule
