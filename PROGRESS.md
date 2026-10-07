# Progress

Port status of Amplitude (PlayStation 2, `SCUS-97258`) from the FreQuency reconstruction. A
routine is done when a line added since the FreQuency baseline annotates its address. The
table lists every routine the port must provide. Routines the toolchain, the vendored
sources, or the compiler supply are counted and left out.

Regenerate after every merged change:

```shell
curl -X POST 'http://127.0.0.1:8089/run_ghidra_script?program=amplitude-SCUS_972.58' \
  -H 'Content-Type: application/json' \
  -d '{"script_name": "ExportFunctionTable.java", "args": "'"$PWD"'/.wiswa-ci/port/us-table.tsv"}'
python3 .wiswa-ci/port/progress.py --write
```

## Summary

| Measure                                |  Count |
| -------------------------------------- | -----: |
| Functions in the program               | 12,661 |
| Excluded: Compiler RTTI                |    819 |
| Excluded: STL instance (GCC library)   |    525 |
| Excluded: Vendored under 3rdparty      |    294 |
| Excluded: C and compiler runtime       |    227 |
| To port                                | 10,796 |
| Done                                   |     90 |
| Share done                             |  0.83% |
| FreQuency annotations left in the tree |  8,500 |

## By component

| Component                    | Routines | Done | Share |
| ---------------------------- | -------: | ---: | ----: |
| Game code                    |    6,814 |   82 |  1.2% |
| Sony online library (SCE-RT) |    1,255 |    0 |  0.0% |
| Sony EE libraries            |      437 |    0 |  0.0% |
| Compiler-emitted instances   |    2,290 |    8 |  0.3% |

## Parts

Each part lists the routines of one component in a 64 KiB address block.

| Part                                                                                      | Component                    | Range                     | Routines | Done | Share |
| ----------------------------------------------------------------------------------------- | ---------------------------- | ------------------------- | -------: | ---: | ----: |
| [game-code-00100000.md](progress/game-code-00100000.md)                                   | Game code                    | `0x00100000`-`0x00110000` |      262 |   28 | 10.7% |
| [game-code-00110000.md](progress/game-code-00110000.md)                                   | Game code                    | `0x00110000`-`0x00120000` |      220 |    2 |  0.9% |
| [game-code-00120000.md](progress/game-code-00120000.md)                                   | Game code                    | `0x00120000`-`0x00130000` |      314 |    0 |  0.0% |
| [game-code-00130000.md](progress/game-code-00130000.md)                                   | Game code                    | `0x00130000`-`0x00140000` |      281 |    0 |  0.0% |
| [game-code-00140000.md](progress/game-code-00140000.md)                                   | Game code                    | `0x00140000`-`0x00150000` |      376 |    2 |  0.5% |
| [game-code-00150000.md](progress/game-code-00150000.md)                                   | Game code                    | `0x00150000`-`0x00160000` |      379 |    0 |  0.0% |
| [game-code-00160000.md](progress/game-code-00160000.md)                                   | Game code                    | `0x00160000`-`0x00170000` |      273 |   13 |  4.8% |
| [game-code-00170000.md](progress/game-code-00170000.md)                                   | Game code                    | `0x00170000`-`0x00180000` |      276 |    0 |  0.0% |
| [game-code-00180000.md](progress/game-code-00180000.md)                                   | Game code                    | `0x00180000`-`0x00190000` |      180 |    0 |  0.0% |
| [game-code-00190000.md](progress/game-code-00190000.md)                                   | Game code                    | `0x00190000`-`0x001a0000` |      246 |    0 |  0.0% |
| [game-code-001a0000.md](progress/game-code-001a0000.md)                                   | Game code                    | `0x001a0000`-`0x001b0000` |      247 |    1 |  0.4% |
| [game-code-001b0000.md](progress/game-code-001b0000.md)                                   | Game code                    | `0x001b0000`-`0x001c0000` |      292 |    0 |  0.0% |
| [game-code-001c0000.md](progress/game-code-001c0000.md)                                   | Game code                    | `0x001c0000`-`0x001d0000` |      174 |    0 |  0.0% |
| [game-code-001d0000.md](progress/game-code-001d0000.md)                                   | Game code                    | `0x001d0000`-`0x001e0000` |      181 |    0 |  0.0% |
| [game-code-001e0000.md](progress/game-code-001e0000.md)                                   | Game code                    | `0x001e0000`-`0x001f0000` |      235 |    0 |  0.0% |
| [game-code-001f0000.md](progress/game-code-001f0000.md)                                   | Game code                    | `0x001f0000`-`0x00200000` |      222 |    2 |  0.9% |
| [game-code-00200000.md](progress/game-code-00200000.md)                                   | Game code                    | `0x00200000`-`0x00210000` |      239 |    0 |  0.0% |
| [game-code-00210000.md](progress/game-code-00210000.md)                                   | Game code                    | `0x00210000`-`0x00220000` |      204 |    0 |  0.0% |
| [game-code-00220000.md](progress/game-code-00220000.md)                                   | Game code                    | `0x00220000`-`0x00230000` |      182 |    0 |  0.0% |
| [game-code-00230000.md](progress/game-code-00230000.md)                                   | Game code                    | `0x00230000`-`0x00240000` |      135 |    3 |  2.2% |
| [game-code-00240000.md](progress/game-code-00240000.md)                                   | Game code                    | `0x00240000`-`0x00250000` |      193 |    4 |  2.1% |
| [game-code-00250000.md](progress/game-code-00250000.md)                                   | Game code                    | `0x00250000`-`0x00260000` |      268 |    0 |  0.0% |
| [game-code-00260000.md](progress/game-code-00260000.md)                                   | Game code                    | `0x00260000`-`0x00270000` |      373 |    8 |  2.1% |
| [game-code-00270000.md](progress/game-code-00270000.md)                                   | Game code                    | `0x00270000`-`0x00280000` |      253 |    0 |  0.0% |
| [game-code-00280000.md](progress/game-code-00280000.md)                                   | Game code                    | `0x00280000`-`0x00290000` |      322 |    7 |  2.2% |
| [game-code-00290000.md](progress/game-code-00290000.md)                                   | Game code                    | `0x00290000`-`0x002a0000` |      421 |   12 |  2.9% |
| [sony-online-library-sce-rt-002a0000.md](progress/sony-online-library-sce-rt-002a0000.md) | Sony online library (SCE-RT) | `0x002a0000`-`0x002b0000` |      143 |    0 |  0.0% |
| [game-code-002a0000.md](progress/game-code-002a0000.md)                                   | Game code                    | `0x002a0000`-`0x002b0000` |       66 |    0 |  0.0% |
| [sony-online-library-sce-rt-002b0000.md](progress/sony-online-library-sce-rt-002b0000.md) | Sony online library (SCE-RT) | `0x002b0000`-`0x002c0000` |      211 |    0 |  0.0% |
| [sony-online-library-sce-rt-002c0000.md](progress/sony-online-library-sce-rt-002c0000.md) | Sony online library (SCE-RT) | `0x002c0000`-`0x002d0000` |      261 |    0 |  0.0% |
| [sony-online-library-sce-rt-002e0000.md](progress/sony-online-library-sce-rt-002e0000.md) | Sony online library (SCE-RT) | `0x002e0000`-`0x002f0000` |      163 |    0 |  0.0% |
| [sony-online-library-sce-rt-002f0000.md](progress/sony-online-library-sce-rt-002f0000.md) | Sony online library (SCE-RT) | `0x002f0000`-`0x00300000` |      179 |    0 |  0.0% |
| [sony-online-library-sce-rt-00300000.md](progress/sony-online-library-sce-rt-00300000.md) | Sony online library (SCE-RT) | `0x00300000`-`0x00310000` |      298 |    0 |  0.0% |
| [sony-ee-libraries-00300000.md](progress/sony-ee-libraries-00300000.md)                   | Sony EE libraries            | `0x00300000`-`0x00310000` |      124 |    0 |  0.0% |
| [sony-ee-libraries-00310000.md](progress/sony-ee-libraries-00310000.md)                   | Sony EE libraries            | `0x00310000`-`0x00320000` |      313 |    0 |  0.0% |
| [compiler-emitted-instances-00330000.md](progress/compiler-emitted-instances-00330000.md) | Compiler-emitted instances   | `0x00330000`-`0x00340000` |      303 |    3 |  1.0% |
| [compiler-emitted-instances-00340000.md](progress/compiler-emitted-instances-00340000.md) | Compiler-emitted instances   | `0x00340000`-`0x00350000` |      436 |    0 |  0.0% |
| [compiler-emitted-instances-00350000.md](progress/compiler-emitted-instances-00350000.md) | Compiler-emitted instances   | `0x00350000`-`0x00360000` |      473 |    0 |  0.0% |
| [compiler-emitted-instances-00360000.md](progress/compiler-emitted-instances-00360000.md) | Compiler-emitted instances   | `0x00360000`-`0x00370000` |      233 |    0 |  0.0% |
| [compiler-emitted-instances-00370000.md](progress/compiler-emitted-instances-00370000.md) | Compiler-emitted instances   | `0x00370000`-`0x00380000` |      298 |    0 |  0.0% |
| [compiler-emitted-instances-00380000.md](progress/compiler-emitted-instances-00380000.md) | Compiler-emitted instances   | `0x00380000`-`0x00390000` |      163 |    0 |  0.0% |
| [compiler-emitted-instances-00390000.md](progress/compiler-emitted-instances-00390000.md) | Compiler-emitted instances   | `0x00390000`-`0x003a0000` |      277 |    5 |  1.8% |
| [compiler-emitted-instances-003a0000.md](progress/compiler-emitted-instances-003a0000.md) | Compiler-emitted instances   | `0x003a0000`-`0x003b0000` |      103 |    0 |  0.0% |
| [compiler-emitted-instances-00420000.md](progress/compiler-emitted-instances-00420000.md) | Compiler-emitted instances   | `0x00420000`-`0x00430000` |        4 |    0 |  0.0% |
