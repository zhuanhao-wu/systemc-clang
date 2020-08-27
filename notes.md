# 1. unsigned short

We may use `unsigned_short` instead of `unsigned short` for the type name.

hcode: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/zfp3/z3test_hdl.txt#L11)

```
      hSigdecl maxbits [
        hTypeinfo  NONAME [
          hType sc_signal [
            hType unsigned short NOLIST
          ]
        ]
      ]
```

# 2. `hBinop []`

We may use `ARRAYSUBSCRIPT` other than `[]` because `[]` has special meaning in hcode.

hcode: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/zfp3/z3test_hdl.txt#L5005)

SystemC Source Code: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/shared2/zhw_encode.h#L914)

```
  hBinop [] [
    hVarref _local_1 NOLIST
    hVarref _local_3 NOLIST
  ]
```

# 3. `hMethodCall` for `sc_dtsc_uint_bitref_rto_bool`

Currently this is not recognized in the script.

hcode: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/zfp3/z3test_hdl.txt#L5009)

SystemC Source Code: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/shared2/zhw_encode.h#L914)

```
 hMethodCall sc_dtsc_uint_bitref_rto_bool [
    hBinop [] [
      hSigAssignR read [
        hBinop ARRAYSUBSCRIPT [
          hVarref s_block NOLIST
          hVarref _local_3 NOLIST
        ]
      ]
      hVarref _local_2 NOLIST
    ]
  ]
```

There are multiple places where the `to_bool` method is present, other places are:
[](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/zfp3/z3test_hdl.txt#L5396)

[](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/zfp3/z3test_hdl.txt#L5435)

# 4. `hNoop` for `break` in `for` loop

Currently, when there is a `break` in for loop, it is translated into a `hNoop`.

However, Verilog does not support `break` in a for loop directly.

hcode: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/zfp3/z3test_hdl.txt#L5265)

SystemC Source Code (Line 942-944): [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/shared2/zhw_encode.h#L942)

```c++
for (unsigned i = fpblk_sz(DIM); i > 0; i--) {
  if (c_bplane[k0.read()].read()[i-1]) {b = i; break;}
}
```

```
...
  hCStmt  NONAME [
    hBinop = [
      hVarref _local_1 NOLIST
      hVarref _local_2 NOLIST
    ]
    hNoop  NONAME NOLIST
  ]
...
```

# 5. `hWhileStmt` is generated in hcode

`while` is supported in Verilog, but the tool may reject it if the tool cannot determine the loop bound.

I should confirm whether the generated code is accepted by vivado.

hcode: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/zfp3/z3test_hdl.txt#L5433)

SystemC Source Code: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/shared2/zhw_encode.h#L968)

# 6. `hSwitchCase`

Currently, `hSwitchCase` leaves out some statments outside its list argument. It would be good if those left out can be moved in the `hSwitchCase`. 

In the example provided below, those left outside are `hBinop` nodes.

hcode: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/zfp3/z3test_hdl.txt#L5781-L5857)

SystemC Source Code: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/shared2/zhw_encode.h#L1175-L1193)

# 7. `fpblk_sz` is included in the `hTypedef` section

`fpblk_sz` may not appear in the `hTypedef` section.

hcode: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/shared2/zhw_encode.h#L245)

SystemC Source Code: [](https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zhw-master/src/zhw.h#L39)

# 8. `fwd_lift` is recognized as a `hTypedef`

It should be recognized as a module.

hcode: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/zfp3/z3test_hdl.txt#L6262)

SystemC Source Code: [](https://github.com/anikau31/systemc-clang/blob/scratchllnl/examples/llnl-examples/zfpsynth/shared2/zhw_encode.h#L245)
