# 1. Sensitivity list

    The sensitivity list in `find_emax`is:

    ```verilog
    SC_METHOD(mc_proc);
    	sensitive << s_fp.valid_chg() << s_fp.data_chg();
    	sensitive << c_fp.ready_event() << c_ex.ready_event();
    	sensitive << emax << emax_v << c_sync;
    SC_METHOD(ms_proc);
    	sensitive << clk.pos();
    	dont_initialize();
    ```

    In `find_emax`, the resulting sensitivity lists for `mc_proc` and `ms_proc`, which are the same.

    Also, the difference between `*_event()` versus directly using `*_chg()`

    ```c++
    #define data_r() data.read()
    #define data_w(d) data.write(d)
    #define data_event() data.value_changed_event()
    #define data_chg() data //.value_changed()

    #define valid_r() valid.read()
    #define valid_w(d) valid.write(d)
    #define valid_event() valid.value_changed_event()
    #define valid_chg() valid //.value_changed()

    #define ready_r() ready.read()
    #define ready_w(d) ready.write(d)
    #define ready_event() ready.value_changed_event()
    #define ready_chg() ready //.value_changed()
    ```

    ```
    hSenslist  NONAME [
      hSensvar c_ex [
        hSensvar ready NOLIST
        hSensvar value_changed_event NOLIST
      ]
      hSensvar c_fp [
        hSensvar ready NOLIST
        hSensvar value_changed_event NOLIST
      ]
      hSensvar c_sync NOLIST
      hSensvar clk NOLIST
      hSensvar clk [
        hSensvar pos NOLIST
      ]
      hSensvar emax NOLIST
      hSensvar emax_v NOLIST
      hSensvar reset NOLIST
      hSensvar s_fp [
        hSensvar data NOLIST
      ]
      hSensvar s_fp [
        hSensvar valid NOLIST
      ]
    ]
    ```

    In `find_emax`, the `pos` may be set to `hSensedge`

    ```
    Sensvar clk NOLIST
    hSensvar clk [
      hSensvar pos NOLIST
    ]
    ```

# 2. Module name
    - Currently it is instance name (for example, `find_emax` vs `u_dut`)

# 3. Module parameter is currently not used, for example: `fp_t` for `find_emax`

```verilog
        hVardecl u_dut [
           hTypeinfo  NONAME [
             hType find_emax [
               hType fp_t [
                 hLiteral 11 NOLIST
                 hLiteral 52 NOLIST
               ]
               hLiteral 1 NOLIST
             ]
           ]
        ]
```

# 4. In sreg: CXXTemporaryObjectExpr

```verilog
   if (reset == RLEV) {
     for (unsigned i = 0; i < depth; i++) data[i] = T();

               hBinop = [
                 hBinop ARRAYSUBSCRIPT [
                   hLiteral data NOLIST
                   hVarref _XLAT_2 NOLIST
                 ]
                 hUnimpl CXXTemporaryObjectExpr NOLIST
               ]
```

# 5. For statement

```verilog
hForStmt  NONAME [
  <null child>
  hBinop < [
    hVarref _XLAT_2 NOLIST
    hVarref depth NOLIST
  ]
  hUnop ++ [
    hVarref _XLAT_2 NOLIST
  ]
  hBinop = [
    hBinop ARRAYSUBSCRIPT [
      hLiteral data NOLIST
      hVarref _XLAT_2 NOLIST
    ]
    hUnimpl CXXTemporaryObjectExpr NOLIST
  ]
]
```

# 6. The `hPortbindings` in `rvfifo_cc`, the `data`, `valid`

```verilog
rvfifo_cc(const sc_module_name& mn_, int size_ = MAX_DEPTH) :
    sc_module(mn_),
    u_fifo("u_fifo", size_)
  {
    u_fifo.clk(clk);
    u_fifo.reset(reset);
    u_fifo.din  (s_port.data);
    u_fifo.wr_en(s_port.valid);
    u_fifo.full (s_port.ready);
    u_fifo.dout (m_port.data);
    u_fifo.rd_en(m_port.ready);
    u_fifo.empty(m_port.valid);
  }

hPortbindings u_fifo [
  hPortbinding  NONAME [
    hVarref clk NOLIST
    hVarref clk NOLIST
  ]
  hPortbinding  NONAME [
    hVarref din NOLIST
    hVarref data NOLIST
  ]
  hPortbinding  NONAME [
    hVarref dout NOLIST
    hVarref data NOLIST
  ]
  hPortbinding  NONAME [
    hVarref empty NOLIST
    hVarref valid NOLIST
  ]
  hPortbinding  NONAME [
    hVarref full NOLIST
    hVarref ready NOLIST
  ]
  hPortbinding  NONAME [
    hVarref rd_en NOLIST
    hVarref ready NOLIST
  ]
  hPortbinding  NONAME [
    hVarref reset NOLIST
    hVarref reset NOLIST
  ]
  hPortbinding  NONAME [
    hVarref wr_en NOLIST
    hVarref valid NOLIST
  ]
]
```

# 7. The `expo` has no qualifier (`_XLAT_1`)

```verilog
expo_t expo;
if (fp.expo == 0 && fp.frac == 0) {
  expo = fp.expo;
} else {
  expo = fp.expo + expo_t(1);
}
...
hIfStmt  NONAME [
	hBinop && [
	  hBinop == [
	    hLiteral expo NOLIST
	    hLiteral 0 NOLIST
	  ]
	  hBinop == [
	    hLiteral frac NOLIST
	    hLiteral 0 NOLIST
	  ]
	]
...
```
