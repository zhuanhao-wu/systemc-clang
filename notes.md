# 1. `m_block` type

`m_block` type (around line 1524 of the txt file) has `bits` type, defined in

[https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared/zfp.h#L235](https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared/zfp.h#L235)

[https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared/zfp.h#L331](https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared/zfp.h#L331)

```python
hPortout m_block [
	hTypeinfo NONAME [
		hType sc_out [
			hType sc_int [
				hType bits NOLIST
			]
		]
	]
]

template<typename FP, int DIM>
SC_MODULE(fwd_cast)
{
	typedef typename FP::expo_t expo_t;
	typedef typename FP::si_t si_t;
	typedef typename FP::ui_t ui_t;
	// ...
	sc_out<si_t> m_block[fpblk_sz(DIM)];
}
```

# 2. Line 123 (depth has no value)

[https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared/fifo_cc.h#L121](https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared/fifo_cc.h#L123)

```python
hVardecl depth [
	hTypeinfo  NONAME [
		hType unsigned_int NOLIST
	]
]
```

# 3. `sc_signal<T> data[MAX_DEPTH];`

[https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared/fifo_cc.h#L55](https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared/fifo_cc.h#L55)

```python
hSigdecl data [
  hTypeinfo  NONAME [
    hType sc_signal [
      hType fp_t [
        hLiteral 11 NOLIST
        hLiteral 52 NOLIST
      ]
    ]
  ]
]
```

# 4. `fpblk_sz(DIM)` should be 3

[https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared/zfp.h#L153](https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared/zfp.h#L153)
[https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared/zfp.h#L24](https://github.com/anikau31/systemc-clang/blob/master/examples/llnl-examples/zfpsynth/shared/zfp.h#L24)

```python
hBinop = [
  hVarref count NOLIST
  hBinop - [
    hLiteral 1 NOLIST
    hLiteral 1 NOLIST
  ]
]
```
