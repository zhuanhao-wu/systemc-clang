typedef struct packed {
  logic[51:0] frac;
  logic[10:0] expo;
  logic       sign;
} fp_t_e_11_f_52;

module fp_t_e_11_f_52_slice (
  input logic[63:0]  in,
  output logic[51:0] frac,
  output logic[10:0] expo,
  output logic       sign
);
  assign {sign, expo, frac} = in;
endmodule

module fp_t_e_11_f_52_concat (
  output logic[63:0]  out,
  input logic[51:0]   frac,
  input logic[10:0]   expo,
  input logic         sign
);
  assign out = {sign, expo, frac};
endmodule

module rvfifo_cc #(
  parameter depth = 2,
  parameter E = 11,
  parameter F = 52
) (
  input logic        clk,
  input logic        reset,

  // not able to merge them into one component
  input logic[63:0]  s_port_data,
  input logic        s_port_valid,
  output logic       s_port_ready,

  output logic[63:0] m_port_data,
  output logic       m_port_valid,
  input logic        m_port_ready
);
  logic[51:0] s_port_data_frac;
  logic[10:0] s_port_data_expo;
  logic       s_port_data_sign;

  logic[51:0] m_port_data_frac;
  logic[10:0] m_port_data_expo;
  logic       m_port_data_sign;
  fp_t_e_11_f_52_concat concat(
    .out(m_port_data),
    .frac(m_port_data_frac),
    .expo(m_port_data_expo),
    .sign(m_port_data_sign)
  );
  fp_t_e_11_f_52_slice slice(
    .in(s_port_data),
    .frac(s_port_data_frac),
    .expo(s_port_data_expo),
    .sign(s_port_data_sign)
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
