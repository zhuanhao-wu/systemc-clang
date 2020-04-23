module pulse_0(
  input logic [0:0] clk,
  output logic [0:0] sig 
);
  logic [1:0] st;
  logic [7:0] count;
  always @(dont_initialize or posedge pos or sensitive or sensitive_neg or sensitive_pos) begin: ms_pulse
    logic [0:0] _XLAT_0;
    _XLAT_0 = 0;
    if((0) && (2)) begin
      st = PRE_S;
      count = (0) - (1);
      sig = !(0);
    end else begin
      if(2) begin
        st = PULSE_S;
        count = (2) - (1);
        sig = 0;
      end else begin
        st = POST_S;
        sig = !(0);
      end
    end
    if((count) == (0)) begin
      st = PULSE_S;
      count = (2) - (1);
      sig = 0;
    end else begin
      _XLAT_0 = 1;
    end
    if((count) == (0)) begin
      st = POST_S;
      sig = !(0);
    end else begin
      _XLAT_0 = 1;
    end
    POST_S;
    if(_XLAT_0) begin
      count = (count) - (1);
    end
  end // ms_pulse
endmodule // pulse_0
module tb_driver_0(
  input logic [0:0] clk,
  input logic [0:0] reset,
  input  logic [255:0] s_port,
  input  logic s_port_valid,
  output logic s_port_ready,
  output logic [0:0] error,
  output logic [63:0] m_port,
  output logic m_port_valid,
  input  logic m_port_ready 
);
  
  logic [0:0] c_sync_recv;
  logic [0:0] c_sync_send;
  logic [31:0] rcount;
  logic [31:0] scount;
  always @(create_method_process or dont_initialize or m_port or posedge pos or rcount or s_port or scount or sensitive or sensitive_neg or sensitive_pos) begin: mc_send
    logic [0:0] _XLAT_0;
    logic [0:0] _XLAT_1;
    logic [63:0] _XLAT_2;
    _XLAT_0 = (scount) < ((2) * (1));
    _XLAT_1 = (m_port_ready) && (_XLAT_0);
    _XLAT_2 = block [ (scount) % (1) ];
    m_port_data <= _XLAT_2;
    m_port_valid <= _XLAT_0;
    c_sync_send = _XLAT_1;
    if(_XLAT_1) begin
      "//# Unimplemented: NONAME";
    end
  end // mc_send
  
  always @(create_method_process or dont_initialize or m_port or posedge pos or rcount or s_port or scount or sensitive or sensitive_neg or sensitive_pos) begin: ms_send
    
    if((reset) == (0)) begin
      scount = 0;
    end else begin
      if(c_sync_send) begin
        scount = (scount) + (1);
      end
    end
  end // ms_send
  
  always @(create_method_process or dont_initialize or m_port or posedge pos or rcount or s_port or scount or sensitive or sensitive_neg or sensitive_pos) begin: mc_recv
    logic [0:0] _XLAT_0;
    logic [255:0] _XLAT_1;
    _XLAT_0 = s_port_valid;
    _XLAT_1 = s_port_data;
    s_port_ready <= 1;
    error = (rcount) != (2);
    c_sync_recv = _XLAT_0;
    if(_XLAT_0) begin
      "//# Unimplemented: NONAME";
    end
  end // mc_recv
  
  always @(create_method_process or dont_initialize or m_port or posedge pos or rcount or s_port or scount or sensitive or sensitive_neg or sensitive_pos) begin: ms_recv
    
    if((reset) == (0)) begin
      rcount = 0;
    end else begin
      if(c_sync_recv) begin
        rcount = (rcount) + (1);
      end
    end
  end // ms_recv
endmodule // tb_driver_0
module zfp_encode_0(
  input logic [0:0] clk,
  input logic [0:0] reset,
  input  logic [63:0] s_fp,
  input  logic s_fp_valid,
  output logic s_fp_ready,
  output logic [255:0] m_enc,
  output logic m_enc_valid,
  input  logic m_enc_ready 
);
  logic [63:0] c_fe_fp,
  logic c_fe_fp_valid,
  logic c_fe_fp_ready
  logic [10:0] c_fe_ex,
  logic c_fe_ex_valid,
  logic c_fe_ex_ready
  find_emax u_find_emax();
  fwd_cast u_fwd_cast();
  logic [10:0] c_fc_block[0:3];
  logic [0:0] c_fc_ready;
  logic [0:0] c_fc_valid;
  always @(c_fc_block or c_fc_valid or m_enc or sensitive or sensitive_neg or sensitive_pos) begin: mc_io
    logic [255:0] _XLAT_0;
    logic [31:0] _XLAT_1;
    for(_XLAT_1 = 0;(_XLAT_1) < (1);_XLAT_1=_XLAT_1+1) begin
        "//# Unimplemented: NONAME" = c_fc_block [ _XLAT_1 ];;
    end
    m_enc_data <= _XLAT_0;
    m_enc_valid <= c_fc_valid;
    c_fc_ready <= m_enc_ready;
  end // mc_io
endmodule // zfp_encode_0
module find_emax_0(
  input logic [0:0] clk,
  input logic [0:0] reset,
  input  logic [63:0] s_fp,
  input  logic s_fp_valid,
  output logic s_fp_ready,
  output logic [63:0] m_fp,
  output logic m_fp_valid,
  input  logic m_fp_ready,
  output logic [10:0] m_ex,
  output logic m_ex_valid,
  input  logic m_ex_ready 
);
  logic [63:0] c_fp,
  logic c_fp_valid,
  logic c_fp_ready
  logic [10:0] c_ex,
  logic c_ex_valid,
  logic c_ex_ready
  rvfifo_cc u_que_fp();
  sreg u_reg_ex();
  logic [0:0] c_sync;
  logic [1:0] count;
  logic [10:0] emax;
  logic [0:0] emax_v;
  always @(c_ex or c_fp or c_sync or create_method_process or dont_initialize or emax or emax_v or posedge pos or ready or s_fp or sensitive or sensitive_neg or sensitive_pos or value_changed_event) begin: mc_proc
    
    s_fp_ready <= c_sync;
    c_fp_data <= s_fp_data;
    c_fp_valid <= c_sync;
    c_ex_data <= emax;
    c_ex_valid <= emax_v;
    c_sync = ((s_fp_valid) && (c_fp_ready)) && ((!(emax_v)) || (c_ex_ready));
  end // mc_proc
  
  always @(c_ex or c_fp or c_sync or create_method_process or dont_initialize or emax or emax_v or posedge pos or ready or s_fp or sensitive or sensitive_neg or sensitive_pos or value_changed_event) begin: ms_proc
    logic [0:0] _XLAT_0;
    logic [63:0] _XLAT_1;
    logic [10:0] _XLAT_2;
    if((reset) == (0)) begin
      count = (1) - (1);
      emax = 0;
      emax_v = 0;
    end else begin
      _XLAT_0 = (count) == (0);
      _XLAT_1 = s_fp_data;
      if(((expo) == (0)) && ((frac) == (0))) begin
        _XLAT_2 = expo;
      end else begin
        _XLAT_2 = (expo) + (1);
      end
      if(c_sync) begin
        if(_XLAT_0) begin
          count = (1) - (1);
        end else begin
          count = (count) - (1);
        end
      end
      if((emax_v) && (c_ex_ready)) begin
        if(s_fp_valid) begin
          emax = _XLAT_2;
        end else begin
          emax = 0;
        end
      end else begin
        if((s_fp_valid) && (_XLAT_2 = emax)) begin
          emax = _XLAT_2;
        end
      end
      if((emax_v) && (c_ex_ready)) begin
        emax_v = 0;
      end else begin
        if((c_sync) && (_XLAT_0)) begin
          emax_v = 1;
        end
      end
    end
  end // ms_proc
endmodule // find_emax_0
module sreg_0(
  input logic [0:0] clk,
  input logic [0:0] reset,
  input  logic [10:0] s_port,
  input  logic s_port_valid,
  output logic s_port_ready,
  output logic [10:0] m_port,
  output logic m_port_valid,
  input  logic m_port_ready 
);
  logic [31:0] IW;
  logic [31:0] depth;
  logic [10:0] data[0:depth-1];
  logic [0:0] empty_i;
  logic [0:0] full_i;
  logic [0:0] rd_en_i;
  logic [0:0] rd_idx;
  logic [0:0] wr_en_i;
  logic [0:0] wr_idx;
  always @(create_method_process or data or dont_initialize or empty_i or full_i or m_port or posedge pos or rd_idx or reset_signal_is or s_port or sensitive or sensitive_neg or sensitive_pos) begin: mc_proc
    
    m_port_data <= data [ rd_idx ];
    wr_en_i = (s_port_valid) && (!(full_i));
    rd_en_i = (m_port_ready) && (!(empty_i));
    s_port_ready <= !(full_i);
    m_port_valid <= !(empty_i);
  end // mc_proc
  
  always @(create_method_process or data or dont_initialize or empty_i or full_i or m_port or posedge pos or rd_idx or reset_signal_is or s_port or sensitive or sensitive_neg or sensitive_pos) begin: ms_proc
    logic [0:0] _XLAT_0;
    logic [0:0] _XLAT_1;
    logic [31:0] _XLAT_2;
    _XLAT_0 = ((wr_idx) + (1)) % (depth);
    _XLAT_1 = ((rd_idx) + (1)) % (depth);
    if((reset) == (0)) begin
      for(_XLAT_2 = 0;(_XLAT_2) < (depth);_XLAT_2=_XLAT_2+1) begin
          data [ _XLAT_2 ] = "//# Unimplemented: CXXTemporaryObjectExpr";;
      end
      rd_idx = 0;
      wr_idx = 0;
      full_i = 0;
      empty_i = 1;
    end else begin
      if(wr_en_i) begin
        data [ wr_idx ] = s_port_data;
        wr_idx = _XLAT_0;
        if(!(rd_en_i)) begin
          if(_XLAT_0 = rd_idx) begin
            full_i = 1;
          end
          empty_i = 0;
        end
      end
      if(rd_en_i) begin
        rd_idx = _XLAT_1;
        if(!(wr_en_i)) begin
          full_i = 0;
          if(_XLAT_1 = wr_idx) begin
            empty_i = 1;
          end
        end
      end
    end
  end // ms_proc
endmodule // sreg_0
module fwd_cast_0(
  input logic [0:0] clk,
  input logic [0:0] reset,
  input logic [0:0] m_ready,
  input  logic [10:0] s_ex,
  input  logic s_ex_valid,
  output logic s_ex_ready,
  input  logic [63:0] s_fp,
  input  logic s_fp_valid,
  output logic s_fp_ready,
  /* m_block not implemented */,
  output logic [0:0] m_valid 
);
  
  logic [63:0] c_int;
  logic [0:0] c_sync;
  logic [1:0] count;
  always @(c_sync or count or create_method_process or dont_initialize or posedge pos or reset_signal_is or s_ex or s_fp or sensitive or sensitive_neg or sensitive_pos) begin: mc_proc
    logic [10:0] _XLAT_0;
    logic [63:0] _XLAT_1;
    logic [2:0] _XLAT_2;
    logic [63:0] _XLAT_3;
    logic [63:0] _XLAT_4;
    c_sync = ((s_ex_valid) && (s_fp_valid)) && (m_ready);
    s_ex_ready <= (c_sync) && ((count) == ((1) - (1)));
    s_fp_ready <= c_sync;
    _XLAT_0 = s_ex_data;
    if((_XLAT_0) != (0)) begin
      _XLAT_0 = 1;
    end
    _XLAT_1 = s_fp_data;
    _XLAT_2 = (expo) != (0);
    _XLAT_3 = "//# Unimplemented: NONAME";
    if(sign) begin
      _XLAT_4 = -(_XLAT_3);
    end else begin
      _XLAT_4 = _XLAT_3;
    end
    c_int <= _XLAT_4;
  end // mc_proc
  
  always @(c_sync or count or create_method_process or dont_initialize or posedge pos or reset_signal_is or s_ex or s_fp or sensitive or sensitive_neg or sensitive_pos) begin: ms_proc
    
    if((reset) == (0)) begin
      count = 0;
      m_valid <= 0;
    end else begin
      if(c_sync) begin
        if((count) == ((1) - (1))) begin
          count = 0;
          m_valid <= 1;
        end else begin
          count = (count) + (1);
          m_valid <= 0;
        end
        m_block [ count ] <= c_int;
      end else begin
        if(m_ready) begin
          m_valid <= 0;
        end
      end
    end
  end // ms_proc
endmodule // fwd_cast_0

