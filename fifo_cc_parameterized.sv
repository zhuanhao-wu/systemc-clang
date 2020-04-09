module fifo_cc # (
  parameter T    = 64, // derived from T, data width
  parameter IW   = 0,
  parameter FWFT = 1,
  parameter RLEV = 0,
  parameter FLEV = 0
) (
  input clk,
  input srst,
  // sc_rvd_in
  // sc_stream_in s_port
  input  logic[T-1:0]       din,
  input  logic              wr_en,
  output logic              full,

  // sc_rvd_out
  // sc_stream_out m_port
  output  logic[T-1:0]       dout,
  input   logic              rd_en,
  output  logic              empty
);
  localparam MAX_DEPTH = 1 << IW;

  logic[T-1:0] data[MAX_DEPTH];

  logic[IW-1:0] rd_idx;
  logic[IW-1:0] wr_idx;

  logic wr_en_i;
  logic rd_en_i;
  logic full_i;
  logic empty_i;

  always @(*) begin: mc_proc
    if(FWFT) begin
      dout = data[rd_idx];
    end

    wr_en_i = wr_en && !full_i;
    rd_en_i = rd_en && !empty_i;

    full = full_i == FLEV;
    empty = empty_i == FLEV;
  end

  always @(posedge clk) begin: ms_proc
    logic[IW-1:0] wr_inc;
    logic[IW-1:0] rd_inc;
    integer unsigned i = 0;
    wr_inc = (wr_idx + 1) % MAX_DEPTH;
    rd_inc = (rd_idx + 1) % MAX_DEPTH;
    if(srst == RLEV) begin
      if(!FWFT) begin
        dout <= 0;
      end
      for(i=0; i < MAX_DEPTH; i++) begin
        data[i] = 0; // T() is not defined
      end
      rd_idx = 0;
      wr_idx = 0;
      full_i = 0;
      empty_i = 1;
    end else begin
      if(!FWFT) begin
        if(rd_en_i) begin
          dout <= data[rd_idx];
        end
      end
      if(wr_en_i) begin
        data[wr_idx] = din;
        wr_idx = wr_inc;
        if(!rd_en_i) begin
          if(wr_inc == rd_idx) full_i = 1;
          empty_i = 0;
        end
      end
      if(rd_en_i) begin
        rd_idx = rd_inc;
        if(!wr_en_i) begin
          full_i = 0;
          if(rd_inc == wr_idx) empty_i = 1;
        end
      end
    end
  end

endmodule

