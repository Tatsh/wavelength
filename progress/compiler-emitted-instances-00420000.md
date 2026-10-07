# Compiler-emitted instances, `0x00420000` to `0x00430000`

0 of 4 routines done.

Sorted by length, then reference count, then status (remaining first),
then name. Signatures are the current Ghidra prototypes and are
preliminary.

| Name                          | Status | # xref | Length | NTSC-U/C     | PAL | Signature                                                     |
| ----------------------------- | :----: | -----: | -----: | ------------ | --- | ------------------------------------------------------------- |
| `InverseDct8x8BlockClampSimd` |  :x:   |      3 |    972 | `0x0042e490` |     | `undefined InverseDct8x8BlockClampSimd()`                     |
| `InverseDct8x8RowColSimd`     |  :x:   |      3 |    756 | `0x0042e190` |     | `undefined InverseDct8x8RowColSimd()`                         |
| `DequantizeBlock8x8Simd`      |  :x:   |      3 |    396 | `0x0042df00` |     | `void DequantizeBlock8x8Simd(long bUseTableB, void * pBlock)` |
| `ScaleBlock8x8Simd`           |  :x:   |      3 |    256 | `0x0042e090` |     | `void ScaleBlock8x8Simd(long bUseTableB, void * pBlock)`      |
