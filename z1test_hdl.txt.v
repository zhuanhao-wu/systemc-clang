module _sc_module_0 (
);
  logic [0:0] clk;
  logic [0:0] reset;
  logic [51:0] c_tb_send_fp_data_frac;
  logic [10:0] c_tb_send_fp_data_expo;
  logic [0:0] c_tb_send_fp_data_sign;
  logic [0:0] c_tb_send_fp_valid;
  logic [0:0] c_tb_send_fp_ready;
  logic [51:0] c_dut_enc_data_frac;
  logic [10:0] c_dut_enc_data_expo;
  logic [0:0] c_dut_enc_data_sign;
  logic [0:0] c_dut_enc_valid;
  logic [0:0] c_dut_enc_ready;
  logic [10:0] c_dut_expo_data;
  logic [0:0] c_dut_expo_valid;
  logic [0:0] c_dut_expo_ready;
  _sc_module_1 u_dut(
    .clk(clk),
    .m_ex_data(c_dut_expo_data),
    .m_ex_valid(c_dut_expo_valid),
    .m_ex_ready(c_dut_expo_ready),
    .m_fp_data_frac(c_dut_enc_data_frac),
    .m_fp_data_expo(c_dut_enc_data_expo),
    .m_fp_data_sign(c_dut_enc_data_sign),
    .m_fp_valid(c_dut_enc_valid),
    .m_fp_ready(c_dut_enc_ready),
    .reset(reset),
    .s_fp_data_frac(c_tb_send_fp_data_frac),
    .s_fp_data_expo(c_tb_send_fp_data_expo),
    .s_fp_data_sign(c_tb_send_fp_data_sign),
    .s_fp_valid(c_tb_send_fp_valid),
    .s_fp_ready(c_tb_send_fp_ready)
  );
endmodule
module _sc_module_1 (
  input logic [0:0] clk,
  input logic [0:0] reset,
  input logic [51:0] s_fp_data_frac,
  input logic [10:0] s_fp_data_expo,
  input logic [0:0] s_fp_data_sign,
  input logic [0:0] s_fp_valid,
  output logic [0:0] s_fp_ready,
  output logic [51:0] m_fp_data_frac,
  output logic [10:0] m_fp_data_expo,
  output logic [0:0] m_fp_data_sign,
  output logic [0:0] m_fp_valid,
  input logic [0:0] m_fp_ready,
  output logic [10:0] m_ex_data,
  output logic [0:0] m_ex_valid,
  input logic [0:0] m_ex_ready
);
  logic [0:0] c_sync;
  logic [1:0] count;
  logic [10:0] emax;
  logic [0:0] emax_v;
  logic [51:0] c_fp_data_frac;
  logic [10:0] c_fp_data_expo;
  logic [0:0] c_fp_data_sign;
  logic [0:0] c_fp_valid;
  logic [0:0] c_fp_ready;
  logic [10:0] c_ex_data;
  logic [0:0] c_ex_valid;
  logic [0:0] c_ex_ready;
  _sc_module_2 u_que_fp(
    .clk(clk),
    .m_port_data_frac(m_fp_data_frac),
    .m_port_data_expo(m_fp_data_expo),
    .m_port_data_sign(m_fp_data_sign),
    .m_port_valid(m_fp_valid),
    .m_port_ready(m_fp_ready),
    .reset(reset),
    .s_port_data_frac(c_fp_data_frac),
    .s_port_data_expo(c_fp_data_expo),
    .s_port_data_sign(c_fp_data_sign),
    .s_port_valid(c_fp_valid),
    .s_port_ready(c_fp_ready)
  );
  _sc_module_3 u_reg_ex(
    .clk(clk),
    .m_port_data(m_ex_data),
    .m_port_valid(m_ex_valid),
    .m_port_ready(m_ex_ready),
    .reset(reset),
    .s_port_data(c_ex_data),
    .s_port_valid(c_ex_valid),
    .s_port_ready(c_ex_ready)
  );
  always @(c_ex_ready or c_fp_ready or c_sync or emax or emax_v or s_fp_data_frac or s_fp_data_expo or s_fp_data_sign or s_fp_valid) begin: mc_proc
    
    
    s_fp_ready = c_sync;
    c_fp_data_frac = s_fp_data_frac;
    c_fp_data_expo = s_fp_data_expo;
    c_fp_data_sign = s_fp_data_sign;
    c_fp_valid = c_sync;
    c_ex_data = emax;
    c_ex_valid = emax_v;
    c_sync = ((s_fp_valid) && (c_fp_ready)) && ((!(emax_v)) || (c_ex_ready));
  end
  always @(posedge clk) begin: ms_proc
    logic [0:0] _local_0;
    logic [51:0] _local_1_frac;
    logic [10:0] _local_1_expo;
    logic [0:0] _local_1_sign;
    logic [10:0] _local_2;
    
    if ((reset) == (0)) begin
      count = (1) - (1);
      emax = 0;
      emax_v = 0;
    end else begin
      _local_0 = (count) == (0);
      _local_1_frac = s_fp_data_frac;
      _local_1_expo = s_fp_data_expo;
      _local_1_sign = s_fp_data_sign;
      if (((_local_1_expo) == (0)) && ((_local_1_frac) == (0))) begin
        _local_2 = _local_1_expo;
      end else begin
        _local_2 = (_local_1_expo) + (1);
      end

      if (c_sync) begin
        if (_local_0) begin
          count = (1) - (1);
        end else begin
          count = (count) - (1);
        end

      end
      if ((emax_v) && (c_ex_ready)) begin
        if (s_fp_valid) begin
          emax = _local_2;
        end else begin
          emax = 0;
        end

      end else begin
        if ((s_fp_valid) && ((_local_2) == (emax))) begin
          emax = _local_2;
        end
      end

      if ((emax_v) && (c_ex_ready)) begin
        emax_v = 0;
      end else begin
        if ((c_sync) && (_local_0)) begin
          emax_v = 1;
        end
      end

    end

  end
endmodule
module _sc_module_2 (
  input logic [0:0] clk,
  input logic [0:0] reset,
  input logic [51:0] s_port_data_frac,
  input logic [10:0] s_port_data_expo,
  input logic [0:0] s_port_data_sign,
  input logic [0:0] s_port_valid,
  output logic [0:0] s_port_ready,
  output logic [51:0] m_port_data_frac,
  output logic [10:0] m_port_data_expo,
  output logic [0:0] m_port_data_sign,
  output logic [0:0] m_port_valid,
  input logic [0:0] m_port_ready
);
  logic [31:0] MAX_DEPTH = 8;
  _sc_module_4 u_fifo(
    .clk(clk),
    .din_frac(s_port_data_frac),
    .din_expo(s_port_data_expo),
    .din_sign(s_port_data_sign),
    .dout_frac(m_port_data_frac),
    .dout_expo(m_port_data_expo),
    .dout_sign(m_port_data_sign),
    .empty(m_port_valid),
    .full(s_port_ready),
    .rd_en(m_port_ready),
    .reset(reset),
    .wr_en(s_port_valid)
  );
endmodule
module _sc_module_4 (
  input logic [0:0] clk,
  input logic [0:0] reset,
  input logic [51:0] din_frac,
  input logic [10:0] din_expo,
  input logic [0:0] din_sign,
  input logic [0:0] wr_en,
  input logic [0:0] rd_en,
  output logic [0:0] full,
  output logic [51:0] dout_frac,
  output logic [10:0] dout_expo,
  output logic [0:0] dout_sign,
  output logic [0:0] empty
);
  logic [51:0] data_frac;
  logic [10:0] data_expo;
  logic [0:0] data_sign;
  logic [0:0] empty_i;
  logic [0:0] full_i;
  logic [0:0] rd_en_i;
  logic [2:0] rd_idx;
  logic [0:0] wr_en_i;
  logic [2:0] wr_idx;
  logic [31:0] depth;
  logic [31:0] MAX_DEPTH = 8;
  always @(empty_i or full_i or rd_en or rd_idx or wr_en) begin: mc_proc
    
    
    if (1) begin
      dout_frac = data_frac[rd_idx];
      dout_expo = data_expo[rd_idx];
      dout_sign = data_sign[rd_idx];
    end
    wr_en_i = (wr_en) && (!(full_i));
    rd_en_i = (rd_en) && (!(empty_i));
    full = (full_i) == (0);
    empty = (empty_i) == (0);
  end
  always @(posedge clk) begin: ms_proc
    logic [2:0] _local_0;
    logic [2:0] _local_1;
    logic [31:0] _local_2;
    
    _local_0 = ((wr_idx) + (1)) % (depth);
    _local_1 = ((rd_idx) + (1)) % (depth);
    if ((reset) == (0)) begin
      if (!(1)) begin
        dout_frac = 0;
        dout_expo = 0;
        dout_sign = 0;
      end
      for (_local_2 = 0;(_local_2) < (depth);_local_2 = _local_2 + 1) begin
        data_frac[_local_2] = 0;
        data_expo[_local_2] = 0;
        data_sign[_local_2] = 0;
      end
      rd_idx = 0;
      wr_idx = 0;
      full_i = 0;
      empty_i = 1;
    end else begin
      if (!(1)) begin
        if (rd_en_i) begin
          dout_frac = data_frac[rd_idx];
          dout_expo = data_expo[rd_idx];
          dout_sign = data_sign[rd_idx];
        end
      end
      if (wr_en_i) begin
        data_frac[wr_idx] = din_frac;
        data_expo[wr_idx] = din_expo;
        data_sign[wr_idx] = din_sign;
        wr_idx = _local_0;
        if (!(rd_en_i)) begin
          if ((_local_0) == (rd_idx)) begin
            full_i = 1;
          end
          empty_i = 0;
        end
      end
      if (rd_en_i) begin
        rd_idx = _local_1;
        if (!(wr_en_i)) begin
          full_i = 0;
          if ((_local_1) == (wr_idx)) begin
            empty_i = 1;
          end
        end
      end
    end

  end
endmodule
module _sc_module_3 (
  input logic [0:0] clk,
  input logic [0:0] reset,
  input logic [10:0] s_port_data,
  input logic [0:0] s_port_valid,
  output logic [0:0] s_port_ready,
  output logic [10:0] m_port_data,
  output logic [0:0] m_port_valid,
  input logic [0:0] m_port_ready
);
  logic [10:0] data;
  logic [0:0] empty_i;
  logic [0:0] full_i;
  logic [0:0] rd_en_i;
  logic [0:0] rd_idx;
  logic [0:0] wr_en_i;
  logic [0:0] wr_idx;
  logic [31:0] IW = 1;
  logic [31:0] depth = 2;
  always @(empty_i or full_i or m_port_ready or rd_idx or s_port_valid) begin: mc_proc
    
    
    m_port_data = data[rd_idx];
    wr_en_i = (s_port_valid) && (!(full_i));
    rd_en_i = (m_port_ready) && (!(empty_i));
    s_port_ready = !(full_i);
    m_port_valid = !(empty_i);
  end
  always @(posedge clk) begin: ms_proc
    logic [0:0] _local_0;
    logic [0:0] _local_1;
    logic [31:0] _local_2;
    
    _local_0 = ((wr_idx) + (1)) % (depth);
    _local_1 = ((rd_idx) + (1)) % (depth);
    if ((reset) == (0)) begin
      for (_local_2 = 0;(_local_2) < (depth);_local_2 = _local_2 + 1) begin
        data[_local_2] = 0;
      end
      rd_idx = 0;
      wr_idx = 0;
      full_i = 0;
      empty_i = 1;
    end else begin
      if (wr_en_i) begin
        data[wr_idx] = s_port_data;
        wr_idx = _local_0;
        if (!(rd_en_i)) begin
          if ((_local_0) == (rd_idx)) begin
            full_i = 1;
          end
          empty_i = 0;
        end
      end
      if (rd_en_i) begin
        rd_idx = _local_1;
        if (!(wr_en_i)) begin
          full_i = 0;
          if ((_local_1) == (wr_idx)) begin
            empty_i = 1;
          end
        end
      end
    end

  end
endmodule

