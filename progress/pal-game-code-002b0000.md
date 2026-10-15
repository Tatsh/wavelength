# Game code, PAL `0x002b0000` to `0x002c0000`

6 of 6 routines done.

Sorted by status (remaining first), then length, then reference count, then
name. Names and signatures are the current Ghidra prototypes of the PAL
program and are preliminary. The NTSC-U/C column is empty for a routine only
the PAL executable has.

| Name                 |       Status       | # xref | Length | PAL          | NTSC-U/C     | Signature                             |
| -------------------- | :----------------: | -----: | -----: | ------------ | ------------ | ------------------------------------- |
| `VIF_disassemble`    | :white_check_mark: |      0 |   1892 | `0x002b13f8` | `0x002a7488` | `undefined WalkAndDecodeVifPacket()`  |
| `VIF_writeVector`    | :white_check_mark: |      1 |   1024 | `0x002b0c20` | `0x002a6cb0` | `undefined TraceVifUnpackVuWrite()`   |
| `VIF_simulateUnpack` | :white_check_mark: |      1 |    864 | `0x002b1020` | `0x002a70b0` | `undefined SimulateVifUnpackWrites()` |
| `VIF_grabElement`    | :white_check_mark: |      1 |    512 | `0x002b0850` | `0x002a68e0` | `undefined GrabUnpackElement()`       |
| `VIF_grabVector`     | :white_check_mark: |      1 |    460 | `0x002b0a50` | `0x002a6ae0` | `undefined GrabUnpackVector()`        |
| `VIF_refill`         | :white_check_mark: |      6 |    120 | `0x002b1380` | `0x002a7410` | `undefined RefillVifDmaData()`        |
