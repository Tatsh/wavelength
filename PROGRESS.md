# Progress

Port status of Amplitude (PlayStation 2, `SCUS-97258`) from the FreQuency reconstruction. A
routine is done when a line added since the FreQuency baseline annotates its address. The
table lists every routine the port must provide. Routines the toolchain, the vendored
sources, or the compiler supply are counted and left out. A routine marked as a stub (an
`@stub` tag in its Doxygen block, or `(stub)` at the end of its address comment) is shown
with :construction: and is not counted as done. A routine annotated in a header with no body
in the tree yet is shown with :memo: and is not counted as done either.

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
| Functions in the program               | 12,771 |
| Excluded: Compiler-emitted instance    |  1,795 |
| Excluded: Compiler RTTI                |  1,350 |
| Excluded: STL instance (GCC library)   |    525 |
| Excluded: C and compiler runtime       |    313 |
| Excluded: Vendored under 3rdparty      |    295 |
| To port                                |  8,493 |
| Done                                   |  7,124 |
| Stubbed (not counted as done)          |      0 |
| Declared only (not counted as done)    |     85 |
| Share done                             | 83.88% |
| FreQuency annotations left in the tree |  2,557 |

## By component

| Component                    | Routines |  Done | Share |
| ---------------------------- | -------: | ----: | ----: |
| Game code                    |    6,800 | 6,790 | 99.9% |
| Sony online library (SCE-RT) |    1,256 |     0 |  0.0% |
| Sony EE libraries            |      437 |   334 | 76.4% |

## IOP modules

Each IOP module is measured against its own program in the `/amplitude-IOP` folder of the
disassembler project, from the annotations under `src/iop/<module>/`.

| Module                               | Routines | Done |  Share |
| ------------------------------------ | -------: | ---: | -----: |
| [synth_s](progress/iop-synth_s.md)   |      142 |  142 | 100.0% |
| [softfx_s](progress/iop-softfx_s.md) |       74 |   74 | 100.0% |
| [cxtmdm](progress/iop-cxtmdm.md)     |       42 |   42 | 100.0% |
| [ezncnf_s](progress/iop-ezncnf_s.md) |       22 |   22 | 100.0% |
| [eznctl_s](progress/iop-eznctl_s.md) |       27 |   27 | 100.0% |
| [libnetb](progress/iop-libnetb.md)   |       46 |   46 | 100.0% |
| [SNProfil](progress/iop-snprofil.md) |        3 |    3 | 100.0% |

## Parts

Each part lists the routines of one component in a 64 KiB address block.

| Part                                                                                      | Component                    | Range                     | Routines | Done |  Share |
| ----------------------------------------------------------------------------------------- | ---------------------------- | ------------------------- | -------: | ---: | -----: |
| [game-code-00100000.md](progress/game-code-00100000.md)                                   | Game code                    | `0x00100000`-`0x00110000` |      196 |  196 | 100.0% |
| [game-code-00110000.md](progress/game-code-00110000.md)                                   | Game code                    | `0x00110000`-`0x00120000` |      220 |  220 | 100.0% |
| [game-code-00120000.md](progress/game-code-00120000.md)                                   | Game code                    | `0x00120000`-`0x00130000` |      314 |  314 | 100.0% |
| [game-code-00130000.md](progress/game-code-00130000.md)                                   | Game code                    | `0x00130000`-`0x00140000` |      281 |  280 |  99.6% |
| [game-code-00140000.md](progress/game-code-00140000.md)                                   | Game code                    | `0x00140000`-`0x00150000` |      382 |  382 | 100.0% |
| [game-code-00150000.md](progress/game-code-00150000.md)                                   | Game code                    | `0x00150000`-`0x00160000` |      381 |  381 | 100.0% |
| [game-code-00160000.md](progress/game-code-00160000.md)                                   | Game code                    | `0x00160000`-`0x00170000` |      273 |  273 | 100.0% |
| [game-code-00170000.md](progress/game-code-00170000.md)                                   | Game code                    | `0x00170000`-`0x00180000` |      276 |  276 | 100.0% |
| [game-code-00180000.md](progress/game-code-00180000.md)                                   | Game code                    | `0x00180000`-`0x00190000` |      180 |  180 | 100.0% |
| [game-code-00190000.md](progress/game-code-00190000.md)                                   | Game code                    | `0x00190000`-`0x001a0000` |      248 |  248 | 100.0% |
| [game-code-001a0000.md](progress/game-code-001a0000.md)                                   | Game code                    | `0x001a0000`-`0x001b0000` |      247 |  247 | 100.0% |
| [game-code-001b0000.md](progress/game-code-001b0000.md)                                   | Game code                    | `0x001b0000`-`0x001c0000` |      293 |  293 | 100.0% |
| [game-code-001c0000.md](progress/game-code-001c0000.md)                                   | Game code                    | `0x001c0000`-`0x001d0000` |      174 |  174 | 100.0% |
| [game-code-001d0000.md](progress/game-code-001d0000.md)                                   | Game code                    | `0x001d0000`-`0x001e0000` |      181 |  181 | 100.0% |
| [game-code-001e0000.md](progress/game-code-001e0000.md)                                   | Game code                    | `0x001e0000`-`0x001f0000` |      236 |  236 | 100.0% |
| [game-code-001f0000.md](progress/game-code-001f0000.md)                                   | Game code                    | `0x001f0000`-`0x00200000` |      222 |  222 | 100.0% |
| [game-code-00200000.md](progress/game-code-00200000.md)                                   | Game code                    | `0x00200000`-`0x00210000` |      239 |  239 | 100.0% |
| [game-code-00210000.md](progress/game-code-00210000.md)                                   | Game code                    | `0x00210000`-`0x00220000` |      204 |  204 | 100.0% |
| [game-code-00220000.md](progress/game-code-00220000.md)                                   | Game code                    | `0x00220000`-`0x00230000` |      183 |  183 | 100.0% |
| [game-code-00230000.md](progress/game-code-00230000.md)                                   | Game code                    | `0x00230000`-`0x00240000` |      136 |  136 | 100.0% |
| [game-code-00240000.md](progress/game-code-00240000.md)                                   | Game code                    | `0x00240000`-`0x00250000` |      196 |  196 | 100.0% |
| [game-code-00250000.md](progress/game-code-00250000.md)                                   | Game code                    | `0x00250000`-`0x00260000` |      268 |  267 |  99.6% |
| [game-code-00260000.md](progress/game-code-00260000.md)                                   | Game code                    | `0x00260000`-`0x00270000` |      389 |  389 | 100.0% |
| [game-code-00270000.md](progress/game-code-00270000.md)                                   | Game code                    | `0x00270000`-`0x00280000` |      254 |  254 | 100.0% |
| [game-code-00280000.md](progress/game-code-00280000.md)                                   | Game code                    | `0x00280000`-`0x00290000` |      333 |  326 |  97.9% |
| [game-code-00290000.md](progress/game-code-00290000.md)                                   | Game code                    | `0x00290000`-`0x002a0000` |      427 |  426 |  99.8% |
| [sony-online-library-sce-rt-002a0000.md](progress/sony-online-library-sce-rt-002a0000.md) | Sony online library (SCE-RT) | `0x002a0000`-`0x002b0000` |      143 |    0 |   0.0% |
| [game-code-002a0000.md](progress/game-code-002a0000.md)                                   | Game code                    | `0x002a0000`-`0x002b0000` |       67 |   67 | 100.0% |
| [sony-online-library-sce-rt-002b0000.md](progress/sony-online-library-sce-rt-002b0000.md) | Sony online library (SCE-RT) | `0x002b0000`-`0x002c0000` |      211 |    0 |   0.0% |
| [sony-online-library-sce-rt-002c0000.md](progress/sony-online-library-sce-rt-002c0000.md) | Sony online library (SCE-RT) | `0x002c0000`-`0x002d0000` |      261 |    0 |   0.0% |
| [sony-online-library-sce-rt-002e0000.md](progress/sony-online-library-sce-rt-002e0000.md) | Sony online library (SCE-RT) | `0x002e0000`-`0x002f0000` |      163 |    0 |   0.0% |
| [sony-online-library-sce-rt-002f0000.md](progress/sony-online-library-sce-rt-002f0000.md) | Sony online library (SCE-RT) | `0x002f0000`-`0x00300000` |      180 |    0 |   0.0% |
| [sony-online-library-sce-rt-00300000.md](progress/sony-online-library-sce-rt-00300000.md) | Sony online library (SCE-RT) | `0x00300000`-`0x00310000` |      298 |    0 |   0.0% |
| [sony-ee-libraries-00300000.md](progress/sony-ee-libraries-00300000.md)                   | Sony EE libraries            | `0x00300000`-`0x00310000` |      124 |   21 |  16.9% |
| [sony-ee-libraries-00310000.md](progress/sony-ee-libraries-00310000.md)                   | Sony EE libraries            | `0x00310000`-`0x00320000` |      313 |  313 | 100.0% |
