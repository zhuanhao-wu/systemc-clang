module sreg_fwd # (
  parameter T_WIDTH = 0
)(
  input logic clk,
  input logic reset,
  // sc_rvd_in
  // sc_stream_in s_port
  input  logic[T_WIDTH-1:0] s_port_data,
  input  logic              s_port_valid,
  output logic              s_port_ready,

  // sc_rvd_out
  // sc_stream_out m_port
  output  logic[T_WIDTH-1:0] m_port_data,
  output  logic              m_port_valid,
  input   logic              m_port_ready
);
  localparam RLEV = 0;
  logic c_valid;
  always @(*) begin : mc_proc
  ┊ s_port_ready = m_port_ready || !c_valid; // ready()
  ┊ m_port_valid = c_valid; // write()
  end
  always @(posedge clk) begin: ms_proc
  ┊ if(reset == RLEV) begin
  ┊ ┊ m_port_data <= 0; // data_w(T())
  ┊ ┊ c_valid <= 0;
  ┊ end else begin
  ┊ ┊ if(m_port_ready || !c_valid) begin
  ┊ ┊ ┊ m_port_data <= s_port_data;
  ┊ ┊ ┊ c_valid <= s_port_valid;
  ┊ ┊ end
  ┊ end
  end
endmodule
