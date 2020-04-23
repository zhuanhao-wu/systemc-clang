## Enum Support
LOC: `shared/pulse.h` L27
```c++
enum state_e {INIT_S, PRE_S, PULSE_S, POST_S};
...
if (DELAY && CYCLES) {
  st = PRE_S;
  count = DELAY-1;
  sig = !PLEV;
} 
```
```
hBinop = [
  hLiteral st NOLIST
  hVarref PRE_S NOLIST
]
```

## Array of ports
LOC: `shared/zfp.h` L235
```c++
sc_out<si_t> m_block[fpblk_sz(DIM)];
```

## Array of signals
LOC: `shared/zfp.h` L331
```c++
sc_signal<si_t> c_fc_block[fpblk_sz(DIM)];
```
```
hSigdecl c_fc_block [
    hTypeinfo  NONAME NOLIST
]
```

## Module parameterization
`zfp_encode`

---

## Port flattening
LOC: `shared/zfp.h` L346
It is flattened in the body, but not flattened in the port list
```c++
m_enc.data_w(block);
```

```
  hSigAssignL write [
    hLiteral m_enc_valid NOLIST
    hSigAssignR read [
      hLiteral c_fc_valid NOLIST
    ]
  ]
```


