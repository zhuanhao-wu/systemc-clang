// BYPASS
// sreg<int, BYPASS>
module sreg_bypass(
  // sc_in<bool> clk;
  input clk,

  // sc_in<bool> reset;
  input reset,

  // sc_stream_in<T> s_port;
  input                         s_port_valid,
  output reg                    s_port_ready,
  input  [typename_T_width-1:0] s_port_data,

  // sc_stream_out<T> m_port;
  output reg                       m_port_valid,
  output reg[typename_T_width-1:0] m_port_data,
  input                            m_port_ready
);
  localparam typename_T_width = 32;
  always @(s_port_valid or s_port_data or m_port_ready) begin : mc_io
    s_port_ready <= m_port_ready;
    m_port_data  <= s_port_data;
    m_port_valid <= s_port_valid;
  end // mc_io

endmodule // sreg_bypass

// sreg<int, FWD>
module sreg_fwd(
  // sc_in<bool> clk;
  input clk,

  // sc_in<bool> reset;
  input reset,

  // sc_stream_in<T> s_port;
  input                         s_port_valid,
  output reg                    s_port_ready,
  input  [typename_T_width-1:0] s_port_data,


  // sc_stream_in<T> m_port;
  output reg                       m_port_valid,
  output reg[typename_T_width-1:0] m_port_data,
  input                            m_port_ready
);
  localparam typename_T_width = 32;
  localparam RLEV = 0;
  // sc_signal<bool> c_valid;
  reg c_valid;

  always @(m_port_ready or c_valid) begin : mc_proc
    s_port_ready <= m_port_ready || !c_valid;
    m_port_valid <= c_valid;
  end // mc_proc

  always @(posedge clk) begin : ms_proc
    if(reset == RLEV) begin
      // initialization value
      // TODO: replace T() with a sensible construct
      m_port_data <= T();
      c_valid <= RLEV;
    end else begin
      if(m_port_ready || !c_valid) begin
        m_port_data <= s_port_data;
        c_valid <= s_port_valid;
      end
    end
  end // ms_proc
endmodule // sreg_fwd

// sreg<double, FWD_REV>
module sreg_fwd_rev(
  // sc_in<bool> clk;
  input clk,

  // sc_in<bool> reset;
  input reset,

  // sc_stream_in<T> s_port;
  input                         s_port_valid,
  output reg                    s_port_ready,
  input  [typename_T_width-1:0] s_port_data,


  // sc_stream_in<T> m_port;
  output reg                       m_port_valid,
  output reg[typename_T_width-1:0] m_port_data,
  input                            m_port_ready
);
  localparam typename_T_width = 64;
  localparam IW = 1;
  localparam RLEV = 0;
  localparam unsigned depth = 2;

	// sc_signal<T> data[depth];
  reg[typename_T_width-1:0] data[0:depth-1];
	// sc_signal<sc_uint<IW> > rd_idx;
  reg[IW-1:0] rd_idx;
	// sc_signal<sc_uint<IW> > wr_idx;
  reg[IW-1:0] wr_idx;

	// sc_signal<bool> wr_en_i;
  reg wr_en_i;
	// sc_signal<bool> rd_en_i;
  reg rd_en_i;
	// sc_signal<bool> full_i;
  reg full_i;
	// sc_signal<bool> empty_i;
  reg empty_i;

  always @(s_port_valid or 
    m_port_ready or 
    full_i or 
    empty_i or 
    rd_idx or data) begin : mc_proc
    m_port_data <= data[rd_idx];
    wr_en_i = s_port_valid && !full_i;
    rd_en_i = m_port_ready && !empty_i;

    s_port_ready <= !full_i;
    m_port_valid <= !empty_i;
  end // mc_proc

  always @(posedge clk) begin : ms_proc
    reg[IW-1:0] wr_inc;
    reg[IW-1:0] rd_inc;
    integer i;

    wr_inc = (wr_idx + 1) % depth;
    rd_inc = (rd_idx + 1) % depth;

    if(reset == RLEV) begin
      for(i = 0; i < depth; i = i + 1) begin
        data[i] = T();
      end
      rd_idx = 0;
      wr_idx = 0;
      full_i = 0;
      empty_i = 0;
    end else begin
      if(wr_en_i) begin
        data[wr_idx] = s_port_data;
        wr_idx = wr_inc;
        if(!rd_en_i) begin
          if(wr_inc == rd_idx) begin
            full_i = 1;
          end
          empty_i = 0;
        end
      end
      if(rd_en_i) begin
        rd_idx = rd_inc;
        if(!wr_en_i) begin
          full_i = 0;
          if(rd_inc == wr_idx) begin
            empty_i = 1;
          end
        end
      end
    end
  end // ms_proc

endmodule // sreg_fwd_rev
