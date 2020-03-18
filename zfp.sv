/* a working version of the zfp_encode module */
`timescale 1ns/1ps
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
    s_port_ready = m_port_ready || !c_valid; // ready()
    m_port_valid = c_valid; // write()
  end
  always @(posedge clk) begin: ms_proc
    if(reset == RLEV) begin
      m_port_data <= 0; // data_w(T())
      c_valid <= 0;
    end else begin
      if(m_port_ready || !c_valid) begin
        m_port_data <= s_port_data;
        c_valid <= s_port_valid;
      end
    end
  end
endmodule

module fifo_cc # (
  parameter depth = 2,
  parameter E  = 0,
  parameter F  = 0,
  parameter IW = 16,
  parameter FWFT = 1,
  parameter RLEV = 0,
  parameter FLEV = 0
) (
  input clk,
  input srst,
  // sc_rvd_in
  // sc_stream_in s_port
  input  logic[F-1:0]       din_frac,
  input  logic[E-1:0]       din_expo,
  input  logic              din_sign,
  input  logic              wr_en,
  output logic              full,

  // sc_rvd_out
  // sc_stream_out m_port
  output  logic[F-1:0]      dout_frac,
  output  logic[E-1:0]      dout_expo,
  output  logic             dout_sign,
  input   logic              rd_en,
  output  logic              empty
);
  logic[F-1:0] data_frac[depth];
  logic[E-1:0] data_expo[depth];
  logic        data_sign[depth];

  logic[IW-1:0] rd_idx;
  logic[IW-1:0] wr_idx;

  logic wr_en_i;
  logic rd_en_i;
  logic full_i;
  logic empty_i;

  always @(*) begin: mc_proc
    if(FWFT) begin
      dout_frac = data_frac[rd_idx];
      dout_expo = data_expo[rd_idx];
      dout_sign = data_sign[rd_idx];
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
    wr_inc = (wr_idx + 1) % depth;
    rd_inc = (rd_idx + 1) % depth;
    if(srst == RLEV) begin
      if(!FWFT) begin
        dout_frac <= 0;
        dout_expo <= 0;
        dout_sign <= 0;
      end
      for(i=0; i < depth; i++) begin
        data_frac[i] = 0; // T() is not defined
        data_expo[i] = 0; // T() is not defined
        data_sign[i] = 0; // T() is not defined
      end
      rd_idx = 0;
      wr_idx = 0;
      full_i = 0;
      empty_i = 1;
    end else begin
      if(!FWFT) begin
        if(rd_en_i) begin
          dout_frac <= data_frac[rd_idx];
          dout_expo <= data_expo[rd_idx];
          dout_sign <= data_sign[rd_idx];
        end
      end
      if(wr_en_i) begin
        data_frac[wr_idx] = din_frac;
        data_expo[wr_idx] = din_expo;
        data_sign[wr_idx] = din_sign;
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
  input  logic        s_port_valid,
  output logic        s_port_ready,

  // sc_rvd_out
  // sc_stream_out m_port
  output  logic[F-1:0] m_port_data_frac,
  output  logic[E-1:0] m_port_data_expo,
  output  logic        m_port_data_sign,
  output  logic        m_port_valid,
  input   logic        m_port_ready
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

/** 
 * this is the top module, 
 * WARNING: this is specialized to fp_t<E, F>
 * An alternative is to flatten fp_t into multiple fields, which turns
 * s_fp_data into:
 * s_fp_data_frac
 * s_fp_data_expo
 * s_fp_data_sign
 */
module find_emax #(
  parameter E = 0,
  parameter F = 0,
  parameter DIM = 1
) (
  /*------ ports ------*/
  input logic clk,
  input logic reset,

  // sc_rvd_in
  // sc_stream_in s_fp
  (* mark_debug = "true" *) input  logic[F-1:0]  s_fp_data_frac,
  (* mark_debug = "true" *) input  logic[E-1:0]  s_fp_data_expo,
  (* mark_debug = "true" *) input  logic         s_fp_data_sign,
  (* mark_debug = "true" *) input  logic         s_fp_valid,
  (* mark_debug = "true" *) output logic         s_fp_ready,

  // sc_rvd_out
  // sc_stream_out m_fp
  (* mark_debug = "true" *) output  logic[F-1:0] m_fp_data_frac,
  (* mark_debug = "true" *) output  logic[E-1:0] m_fp_data_expo,
  (* mark_debug = "true" *) output  logic        m_fp_data_sign,
  (* mark_debug = "true" *) output  logic        m_fp_valid,
  (* mark_debug = "true" *) input   logic        m_fp_ready,

  // sc_rvd_out
  // sc_stream_out m_ex
  (* mark_debug = "true" *) output  logic[E-1:0] m_ex_data,
  (* mark_debug = "true" *) output  logic        m_ex_valid,
  (* mark_debug = "true" *) input   logic        m_ex_ready
);
  /* registers */
  // These might not work as expected though...
  logic[2*DIM-1:0]                       count; // sc_uint<2*DIM>
  // logic[$bits(s_fp.data.expo)-1:0]    emax;   // expo_t -> sc_uint<11>
  logic[E-1:0]                           emax;
  logic                                  emax_v; // bool

  /* channels */
  logic                                     c_sync; // bool
  // sc_stream<T>                           c_fp
  // sc_stream#(type(s_fp.data))            c_fp; // sc_stream<FP>
  logic[F-1:0]                              c_fp_data_frac;
  logic[E-1:0]                              c_fp_data_expo;
  logic                                     c_fp_data_sign;
  logic                                     c_fp_valid;
  logic                                     c_fp_ready;

  // sc_stream<T>                              c_ex;
  // sc_stream#(type(s_fp.data.expo))          c_ex;
  logic[E-1:0]                              c_ex_data;
  logic                                     c_ex_valid;
  logic                                     c_ex_ready;

  /* modules */
  rvfifo_cc#(fpblk_sz(DIM), E, F) u_que_fp(
    .clk(clk),
    .reset(reset),
    // s_port
    .s_port_data_frac (c_fp_data_frac),
    .s_port_data_expo (c_fp_data_expo),
    .s_port_data_sign (c_fp_data_sign),
    .s_port_valid(c_fp_valid),
    .s_port_ready(c_fp_ready),
    
    // m_port
    .m_port_data_frac  (m_fp_data_frac ),
    .m_port_data_expo  (m_fp_data_expo ),
    .m_port_data_sign  (m_fp_data_sign ),
    .m_port_valid(m_fp_valid),
    .m_port_ready(m_fp_ready)
  );
  sreg_fwd#(E) u_reg_ex(
    .clk(clk),
    .reset(reset),
    // s_port
    .s_port_data (c_ex_data ),
    .s_port_valid(c_ex_valid),
    .s_port_ready(c_ex_ready),
    // m_port
    .m_port_data (m_ex_data ),
    .m_port_valid(m_ex_valid),
    .m_port_ready(m_ex_ready)
  );
  
  // this c_ex here is only a channel to another module

  always @(*) begin: mc_proc
    // TODO: confirm whether this is the case
    s_fp_ready <= c_sync;

    c_fp_data_frac  <= s_fp_data_frac ;
    c_fp_data_expo  <= s_fp_data_expo ;
    c_fp_data_sign  <= s_fp_data_sign ;
    c_fp_valid <= c_sync;

    c_ex_data <= emax;
    c_ex_valid <= emax_v;

    c_sync <= s_fp_valid && c_fp_ready && (!emax_v || c_ex_ready);
  end // mc_proc

  always @(posedge clk) begin: ms_proc

    $display("%t rst: %b s_fp_valid: %b data: %x", $time, reset, s_fp_valid, s_fp_data_frac);
    if(reset == 0) begin
      count  = fpblk_sz(DIM) - 1;
      emax   = 0;
      emax_v = 0;
    end else begin
      bit last;
      logic[F-1:0] fp_frac;
      logic[E-1:0] fp_expo;
      logic        fp_sign;
      last = (count == 0);
      fp_frac = s_fp_data_frac;
      fp_expo = s_fp_data_expo;
      fp_sign = s_fp_data_sign;
      if(c_sync) begin
        if(last) begin
          count = fpblk_sz(DIM) - 1;
        end else begin
          count = count - 1;
        end
      end
      if(emax_v && c_ex_ready) begin
        if(s_fp_valid) emax = fp_expo; // fp.expo
        else emax = 0;
      end else if(s_fp_valid && fp_expo > emax) begin // fp.expo
        emax = fp_expo;
      end
      if(emax_v && c_ex_ready) emax_v = 0;
      else if(c_sync && last) emax_v = 1;
    end
  end // ms_proc

  function int fpblk_sz(int dim);
    fpblk_sz = 1 << 2 * dim;
  endfunction
endmodule

module constants(
  output logic[31:0] block_emax,

  input  logic[3:0]  block_idx,
  output logic[63:0] block
);
  logic[63:0] block_arr[0:15];
  assign block_emax = 32'h3f7;
  assign block = block_arr[block_idx];
  initial begin
    block_arr[ 0] = 64'hbf7c3a7bb8495ca9;
    block_arr[ 1] = 64'hbf79f9d9058ffdaf;
    block_arr[ 2] = 64'hbf77c7abd0b61999;
    block_arr[ 3] = 64'hbf75a42c806bd1da;
    block_arr[ 4] = 64'hbf738f8f740b8ea8;
    block_arr[ 5] = 64'hbf718a050399fef8;
    block_arr[ 6] = 64'hbf6f2772ff8c30fe;
    block_arr[ 7] = 64'hbf6b59aa63d22f68;
    block_arr[ 8] = 64'hbf67aaf8b80cff9e;
    block_arr[ 9] = 64'hbf641b9e71983592;
    block_arr[10] = 64'hbf60abd3f723f2b7;
    block_arr[11] = 64'hbf5ab7934169cc04;
    block_arr[12] = 64'hbf54574f6f4897d3;
    block_arr[13] = 64'hbf4c6e39da7fb99b;
    block_arr[14] = 64'hbf40ae5826a893d1;
    block_arr[15] = 64'hbf25bce8e19d48e1;
  end
endmodule

module tb_driver#(
  parameter E = 11,
  parameter F = 52,
  parameter DIM = 2
)(
  input logic clk,
  input logic reset,
  // typedef fp_t<E, F> real_t
  // typedefa real_t flit_t
  // sc_stream_out<flit_t> m_fp
  output  logic[F-1:0]    m_fp_data_frac,
  output  logic[E-1:0]    m_fp_data_expo,
  output  logic           m_fp_data_sign,
  output  logic               m_fp_valid,
  input   logic               m_fp_ready,

  // sc_stream_in<si_t> s_int
  // parsed as signed integer
  input  logic[E+F+1-1:0]   s_int_data,
  input  logic              s_int_valid,
  output logic              s_int_ready
);
  // these should be replaced by preprocessor in C++, used here for readability

  localparam BLOCK_SIZE = fpblk_sz(DIM);
  logic[31:0] ij;
  logic[E+F+1-1:0] flit;
  logic[31:0] block_idx;
  assign block_idx = ij % BLOCK_SIZE;
  constants c(
    .block_idx(block_idx),
    .block(flit)
  );
  // not sure how to translate ct_recv, so hand written here
  // assign m_fp_valid = ij < (2 * BLOCK_SIZE);
  assign {m_fp_data_sign, m_fp_data_expo, m_fp_data_frac} = flit;
  always @(posedge clk) begin: ct_send
    // $display("%x %x %x %x %x: %x", ij, BLOCK_SIZE, DIM, fpblk_sz(DIM), block_idx, flit);
    if(!reset) begin
      ij <= 0;
      m_fp_valid <= 0;
    end else begin
      if(ij < 2 * BLOCK_SIZE) begin
        m_fp_valid <= 1;
        if(m_fp_valid && m_fp_ready && ij != 2 * BLOCK_SIZE - 1) begin
          ij <= ij + 1;
        end
        if(ij == 2 * BLOCK_SIZE - 1) begin
          m_fp_valid <= 0;
        end
      end
    end
  end

  // this block does nothing now but receive data
  always @(posedge clk) begin: ct_recv
    logic[31:0] ij;
    if(!reset) begin
      ij <= 0;
    end else begin
      if(ij < 2 * BLOCK_SIZE) begin
        if(s_int_valid && s_int_ready) begin
          ij <= ij + 1;
        end
      end
    end
  end
  assign s_int_ready = ct_recv.ij < 2 * BLOCK_SIZE;

  function int fpblk_sz(int dim);
    return 1 << (2 * dim);
  endfunction
endmodule

module fwd_cast #(
  parameter E = 0,
  parameter F = 0,
  parameter DIM=0
) (
  input logic clk,
  input logic reset,

  // sc_stream_in<expo_t> s_ex
  input  logic[E-1:0]            s_ex_data,
  input  logic                   s_ex_valid,
  output logic                   s_ex_ready,

  // sc_stream_in<FP> s_fp
  input  logic[F-1:0]        s_fp_data_frac,
  input  logic[E-1:0]        s_fp_data_expo,
  input  logic               s_fp_data_sign,
  input  logic               s_fp_valid,
  output logic               s_fp_ready,

  // sc_stream_out<si_t> m_int
  (* mark_debug = "true" *) output  logic[E+F+1-1:0]    m_int_data,
  (* mark_debug = "true" *) output  logic               m_int_valid,
  (* mark_debug = "true" *) input   logic               m_int_ready
);
  logic[2*DIM-1:0] count; // sc_signal<sc_uint<2*DIM>> count;

  logic c_sync;
  // sc_steram<si_t> c_int
  logic [E+F+1-1:0] c_int_data;
  logic             c_int_valid;
  logic             c_int_ready;

  sreg_fwd #(E+F+1) u_reg_int(
    .clk(clk),
    .reset(reset),

    .s_port_data (c_int_data),
    .s_port_valid(c_int_valid),
    .s_port_ready(c_int_ready),

    .m_port_data (m_int_data),
    .m_port_valid(m_int_valid),
    .m_port_ready(m_int_ready)
  );
  always @(*) begin: mc_proc
    logic[E-1:0] emax;
    logic[F-1:0] fp_frac;
    logic[E-1:0] fp_expo;
    logic        fp_sign;
    logic[E+F+1-1:0] si;

    c_sync = s_ex_valid && s_fp_valid && c_int_ready;


    s_ex_ready = c_sync && count == 0;
    s_fp_ready = c_sync;

    emax = s_ex_data;
    fp_frac = s_fp_data_frac;
    fp_expo = s_fp_data_expo;
    fp_sign = s_fp_data_sign;
    if(fp_sign) begin // fp.sign
      si = - (({3'h1, fp_frac /* fp.frac */, {(E-2){1'b0}}}) >> (emax - fp_expo)); /* fp.expo */
    end else begin
      si = ({3'h1, fp_frac, {(E-2){1'b0}}}) >> (emax - fp_expo);
    end
    c_int_data = si;
    c_int_valid = c_sync;
  end
  always @(posedge clk) begin: ms_proc
    if(reset == 0) begin
      count = fpblk_sz(DIM) - 1;
    end else begin
      if(c_sync) begin
        if(count == 0) begin
          count <= fpblk_sz(DIM) - 1;
        end else begin
          count <= count - 1;
        end
      end else begin
      end
    end
  end

  function fpblk_sz(int dim);
    fpblk_sz = 1 << 2 * dim;
  endfunction
endmodule

module zfp_encode#(
  parameter E=0,
  parameter F=0,
  parameter DIM=0
)(
  input logic clk,
  input logic reset,

  // sc_stream_in<FP> s_fp
  input  logic[F-1:0]        s_fp_data_frac,
  input  logic[E-1:0]        s_fp_data_expo,
  input  logic               s_fp_data_sign,
  input  logic               s_fp_valid,
  output logic               s_fp_ready,

  // sc_stream_out<si_t> m_int
  output  logic[E+F+1-1:0]    m_int_data,
  output  logic               m_int_valid,
  input   logic               m_int_ready
);

  logic [F-1:0]         c_fp_data_frac;
  logic [E-1:0]         c_fp_data_expo;
  logic                 c_fp_data_sign;
  logic                c_fp_valid;
  logic                c_fp_ready;

  logic [E-1:0] c_ex_data;
  logic                    c_ex_valid;
  logic                    c_ex_ready;

  logic [E+F+1-1:0] c_int_data;
  logic             c_int_valid;
  logic             c_int_ready;

  (* dont_touch = "true" *) find_emax #(
      E, F, DIM
  ) u_find_emax(
    .clk(clk),
    .reset(reset),

    .s_fp_data_frac (s_fp_data_frac),
    .s_fp_data_expo (s_fp_data_expo),
    .s_fp_data_sign (s_fp_data_sign),
    .s_fp_valid(s_fp_valid),
    .s_fp_ready(s_fp_ready),

    .m_fp_data_frac (c_fp_data_frac),
    .m_fp_data_expo (c_fp_data_expo),
    .m_fp_data_sign (c_fp_data_sign),
    .m_fp_valid(c_fp_valid),
    .m_fp_ready(c_fp_ready),

    .m_ex_data (c_ex_data),
    .m_ex_valid(c_ex_valid),
    .m_ex_ready(c_ex_ready)
  );

  (* dont_touch = "true" *) fwd_cast#(E, F, DIM) 
  u_fwd_cast (
    .clk(clk),
    .reset(reset),

    .s_fp_data_frac (c_fp_data_frac),
    .s_fp_data_expo (c_fp_data_expo),
    .s_fp_data_sign (c_fp_data_sign),
    .s_fp_valid(c_fp_valid),
    .s_fp_ready(c_fp_ready),

    .m_int_data (c_int_data),
    .m_int_valid(c_int_valid),
    .m_int_ready(c_int_ready),

    .s_ex_data (c_ex_data),
    .s_ex_valid(c_ex_valid),
    .s_ex_ready(c_ex_ready)
  );

  always @(*) begin: mc_io
    c_int_ready = m_int_ready;
    m_int_data = c_int_data;
    m_int_valid = c_int_valid;
  end

endmodule

module pulse #(
  parameter DELAY=0,
  parameter CYCLES=0,
  parameter PLEV=0
) (
  input logic clk,
  input logic rst,
  output logic sig
);
  logic[31:0] counter = 0;
  always @(posedge clk) begin: ct_pules
    if(rst == 0) begin
        counter <= 0;
        sig <= !PLEV;
    end else begin
        if(counter < DELAY + CYCLES) begin
          counter <= counter + 1;
        end
        if(counter < DELAY) begin
          sig <= !PLEV;
        end else if(counter < DELAY + CYCLES) begin
          sig <= PLEV;
        end else begin
          sig <= !PLEV;
        end
    end
  end
endmodule

module top(
  input logic clk,
  input logic rst,
  (* mark_debug = "true" *) output logic reset_out
);
  localparam E = 11;
  localparam F = 52;
  localparam DIM = 2;
  logic reset;


  logic[F-1:0] c_driver_fp_data_frac;
  logic[E-1:0] c_driver_fp_data_expo;
  logic        c_driver_fp_data_sign;
  logic               c_driver_fp_valid;
  logic               c_driver_fp_ready;

  logic[E+F+1-1:0]    c_dut_int_data;
  logic               c_dut_int_valid;
  logic               c_dut_int_ready;
  
  assign reset_out = reset;

  pulse#(0, 2, 0) u_pulse(
    .clk(clk),
    .rst(rst),
    .sig(reset)
  );

  (* dont_touch = "true" *) tb_driver#(E, F, DIM) u_tb_driver(
    .clk(clk),
    .reset(reset),
    .m_fp_data_frac (c_driver_fp_data_frac ),
    .m_fp_data_expo (c_driver_fp_data_expo ),
    .m_fp_data_sign (c_driver_fp_data_sign ),
    .m_fp_valid(c_driver_fp_valid),
    .m_fp_ready(c_driver_fp_ready),

    .s_int_data (c_dut_int_data ),
    .s_int_valid(c_dut_int_valid),
    .s_int_ready(c_dut_int_ready)
  );

  (* dont_touch = "true" *) zfp_encode#(E, F, DIM) u_dut(
    .clk(clk),
    .reset(reset),
    .s_fp_data_frac (c_driver_fp_data_frac ),
    .s_fp_data_expo (c_driver_fp_data_expo ),
    .s_fp_data_sign (c_driver_fp_data_sign ),
    .s_fp_valid(c_driver_fp_valid),
    .s_fp_ready(c_driver_fp_ready),

    .m_int_data (c_dut_int_data ),
    .m_int_valid(c_dut_int_valid),
    .m_int_ready(c_dut_int_ready)
  );

endmodule

/* top design for Vivado */
module top_wrapper(
    input logic clk_p,
    input logic clk_n
);
  logic clk50, ce;
  clk_wiz_0 clkgen(
    .clk_in1_p(clk_p),
    .clk_in1_n(clk_n),
    .clk_out50(clk50)
  );
  vio_0 vio(.clk(clk50), .probe_out0(ce));
  top tp(
    .clk(clk50),
    .rst(ce)
  );
endmodule

module tb;
  logic clk;
  initial begin
    clk = 1;
    forever begin
      #5 clk = !clk;
    end
  end
  initial begin
    // finish after 1100 ns
    # 1100 $finish;
  end
  initial begin
    $dumpfile("tb.vcd");
    $dumpvars(0,dut);
  end
  top dut(
    .clk(clk),
    .rst(1)
  );
endmodule
