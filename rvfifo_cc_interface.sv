/* interface implementation of the rvfifo_cc */

typedef struct packed {
  logic[51:0] frac;
  logic[10:0] expo;
  logic       sign;
} fp_t_e_11_f_52;

interface rvfifo_cc_if;
  fp_t_e_11_f_52 data;
  logic          valid;
  logic          ready;
  modport master(
    output data, valid,
    input ready
  );
  modport slave(
    input data, valid,
    output ready
  );
endinterface

module rvfifo_cc #(
  parameter depth = 2,
  parameter E = 11,
  parameter F = 52
) (
  input logic clk,
  input logic reset,
  rvfifo_cc_if.master m_port,
  rvfifo_cc_if.slave s_port
);
  fifo_cc#(depth, E, F, 16, 1, 0, 0) u_fifo(
    .clk(clk),
    .srst(reset),

    .din_frac  (s_port.data.frac),
    .din_expo  (s_port.data.expo),
    .din_sign  (s_port.data.sign),
    .wr_en(s_port.valid),
    .full (s_port.ready),

    .dout_frac (m_port.data.frac),
    .dout_expo (m_port.data.expo),
    .dout_sign (m_port.data.sign),
    .rd_en(m_port.ready),
    .empty(m_port.valid)
  );
endmodule
