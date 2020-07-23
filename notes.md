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

# 2. For statement

for statement with assignment

```verilog
hForStmt  NONAME [
  hNoop  NONAME NOLIST
  hBinop < [
    hVarref _local_2 NOLIST
    hVarref depth NOLIST
  ]
  hUnop ++ [
    hVarref _local_2 NOLIST
  ]
  ...
]
```

# 3. hUnimpl

In the code generated for the [counter example line 32](https://github.com/anikau31/systemc-clang/blob/master/examples/syn/first-counter/first-counter.h#L32)

```
...
  hSigAssignL write [
    hVarref counter_out NOLIST
    hVarref count NOLIST
  ]
  hUnimpl  NONAME NOLIST
]
```
