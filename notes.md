# 1. Sensitivity list
`hSensvar` and `hSensedge`

# 2. `m_block` type

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
