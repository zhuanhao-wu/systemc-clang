
# u_xt currently includes sc_vector type

```
hVardecl u_xt [
  hTypeinfo  NONAME [
    hType sc_vector [
      hType fwd_lift [
        hType fp_t [
          hLiteral 11 NOLIST
          hLiteral 52 NOLIST
        ]
      ]
    ]
  ]
]
```

- C++ Source
https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared2/zhw_encode.h#L489

- HDL code
https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/zfp3/z3test_hdl.txt#L3511-L3522

# bits_t does not have tlast

- C++ Source
https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/zfp3/z3test_hdl.txt#L7204-L7211

- HDL Code
https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared2/bits.h#L20-L22

# return type of functions

- C++ Source
https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared2/zhw.h#L39

- HDL Code
https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/zfp3/z3test_hdl.txt#L6164-L6185
