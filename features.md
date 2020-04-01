### Flattening of the port
L. 150
```
hPortout m_ex [
┊ hTypeinfo  NONAME [
┊ ┊ hType sc_stream_out NOLIST
┊ ┊ hType sc_uint NOLIST
┊ ┊ hType 11 NOLIST
┊ ]
]
```
Can be resolved with RVD

### For fp\_t
L. 318
```
hVardecl _XLAT_1 [
┊ hTypeinfo  NONAME [
┊ ┊ hType fp_t NOLIST
┊ ┊ hType 11 NOLIST
┊ ┊ hType 52 NOLIST
┊ ]
┊ hVarInit  NONAME [
┊ ┊ hMethodCall sc_coresc_stream_infp_t11_52_data_r [
┊ ┊ ┊ hLiteral s_fp NOLIST
┊ ┊ ]
┊ ]
]
```

```
// translated from: emax = fp.expo;
hBinop = [
 hLiteral expo NOLIST
 hLiteral emax NOLIST
]
```
We need some way of representing the nested structure.

1. aggregate all internal types and infer width & offset information.
2. has something like `_XLAT_1_expo`, `_XLAT_1_frac` and `_XLAT_1_sign` declared separately.

1 replies on the layout of the struct (for example, the order, padding etc) but the Verilog may be less.
2 needs extra care for assignments, but seems more versatile.

### CTOR
```
    u_que_fp.clk(clk);
    u_que_fp.reset(reset);
    u_que_fp.s_port(c_fp);
    u_que_fp.m_port(m_fp);

  // in verilog, this would be something like
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
```

We might need to have information in the constructor as well for module wire connection.

### Differentiating module initialization/module declaration
L. 172
```
hVardecl u_que_fp [
┊ hTypeinfo  NONAME [
┊ ┊ hType sfifo_cc NOLIST
┊ ┊ hType fp_t NOLIST
┊ ┊ hType 52 NOLIST
┊ ┊ hType 11 NOLIST
┊ ]
]
```

Maybe aggregate `sfifo_cc` with extra information, or also record the base class here
so that we know it's a module.

Another question is, should we make the module parameterizable to some extent?

If not, we might need `sfifo_cc_fp_t_52_11`, and `sfifo_cc_fp_t_23_8`, for example.


# Some other concerns

### hMethodCall
L. 226

```
hMethodCall sc_coresc_streamfp_t11_52_data_w [
┊ hLiteral c_fp NOLIST
┊ hMethodCall sc_coresc_stream_infp_t11_52_data_r [
┊ ┊ hLiteral s_fp NOLIST
┊ ]
]
```
Maybe `_` between `sc_core` and `sc_stream` as well.

And we need the function body as well (maybe I'm not looking at the correct location


### expo/emax in `find_emax`
L.384

We might need a reference/definition for `expo/emax`?
```
hBinop = [
 hLiteral expo NOLIST
 hLiteral emax NOLIST
]
```

### Array declaration
L.493
```
hVardecl data [
 hTypeinfo  NONAME NOLIST
]

// in fifo_cc.h
// in decleration
sc_signal<T> *data;
// in CTOR
data = new sc_signal<T>[size_];
```
We might need type information for this.
Or maybe this can be solved with RVD
