/** 
 * this is the top module, 
 * WARNING: this is specialized to fp_t<E, F>
 */
module find_emax #(
  parameter FP = 0,
  parameter DIM = 0,

  /* --- derived parameters --- */
  parameter FP_F = 0,
  parameter FP_E = 0
) (
  /*------ ports ------*/
  input logic clk,
  input logic reset,

  // sc_rvd_in
  // sc_stream_in s_fp
  input  logic[FP-1:0]       s_fp_data,
  input  logic               s_fp_valid,
  output logic               s_fp_ready,

  // sc_rvd_out
  // sc_stream_out m_fp
  output  logic[FP-1:0]       m_fp_data,
  output  logic               m_fp_valid,
  input   logic               m_fp_ready,

  // sc_rvd_out
  // sc_stream_out m_ex
  output  logic[FP_E-1:0]     m_ex_data, // for this part, we need derived parameters
  output  logic               m_ex_valid,
  input   logic               m_ex_ready
);
  /* flattened signals */
  /* slicing logic */
  logic[FP_F-1:0]         s_fp_data_frac;
  logic[FP_E-1:0]         s_fp_data_expo;
  logic                   s_fp_data_sign;
  assign {s_fp_data_sign, s_fp_data_expo, s_fp_data_frac} = s_fp_data;
  /* or, for every reference, we replace it with s_fp_data[...:...] */

  /* registers */
  // These might not work as expected though...
  logic[2*DIM-1:0]                       count; // sc_uint<2*DIM>
  // logic[$bits(s_fp.data.expo)-1:0]    emax;   // expo_t -> sc_uint<11>
  logic[FP_E-1:0]                           emax;
  logic                                  emax_v; // bool

  /* channels */
  logic                                     c_sync; // bool
  // sc_stream<T>                           c_fp
  // sc_stream#(type(s_fp.data))            c_fp; // sc_stream<FP>
  logic[FP_F-1:0]                           c_fp_data_frac;
  logic[FP_E-1:0]                           c_fp_data_expo;
  logic                                     c_fp_data_sign;
  logic                                     c_fp_valid;
  logic                                     c_fp_ready;

  // sc_stream<T>                              c_ex;
  // sc_stream#(type(s_fp.data.expo))          c_ex;
  logic[FP_E-1:0]                              c_ex_data;
  logic                                     c_ex_valid;
  logic                                     c_ex_ready;

  /* modules */
  // was sfifo_cc in the code, but aliased to rvfifo_cc with using
  rvfifo_cc#(FP, 2 * DIM + 1, `RLEVEL) u_que_fp(
    .clk(clk),
    .reset(reset),
    // s_port
    // concatenating logic
    .s_port_data ({c_fp_data_sign, c_fp_data_expo, c_fp_data_frac}),
    .s_port_valid(c_fp_valid),
    .s_port_ready(c_fp_ready),
    
    // m_port
    // slicing logic
    .m_port_data (m_fp_data),
    .m_port_valid(m_fp_valid),
    .m_port_ready(m_fp_ready)
  );

  sreg_fwd#(FP_E) u_reg_ex(
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
      logic[FP_F-1:0] fp_frac;
      logic[FP_E-1:0] fp_expo;
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

