# Progress

PAL parity of the reconstruction: the European executable (`SCES-51706`) measured against
the North American one (`SCUS-97258`). The table lists every PAL routine the port must
provide. That is, each NTSC-U/C routine the port provides, carried to its PAL counterpart,
and each routine only the PAL executable has. A routine is done when a `PAL: 0x...` tag
annotates its PAL address and the tree has its body. A routine marked as a stub (an `@stub`
tag in its Doxygen block, or `(stub)` at the end of its address comment) is shown with
:construction: and is not counted as done. A routine tagged in a header with no body in the
tree yet is shown with :memo: and is not counted as done either.

Regenerate after every merged change, from a PAL build of the tree:

```shell
cmake -B build-pal -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/ps2-ee-toolchain.cmake \
  -DVIDEO_STANDARD=PAL && ninja -C build-pal
WAVELENGTH_BUILD="$PWD/build-pal" python3 .wiswa-ci/port/pal/progress.py "$PWD" --write
```

## Summary

| Measure                                                             |  Count |
| ------------------------------------------------------------------- | -----: |
| NTSC-U/C routines to port with a PAL counterpart                    |  8,582 |
| NTSC-U/C routines to port with no PAL counterpart found             |     11 |
| Routines only the PAL executable has                                |  1,602 |
| Excluded: Compiler-emitted instance                                 |  2,111 |
| Excluded: Compiler RTTI                                             |  1,357 |
| Excluded: STL instance (GCC library)                                |    526 |
| Excluded: C and compiler runtime                                    |    342 |
| Excluded: Vendored under 3rdparty                                   |    296 |
| Excluded: PAL only: Compiler-emitted instances                      |     55 |
| Excluded: PD audit: compiler or library generated, or not a routine |     49 |
| PAL routines to provide                                             | 10,184 |
| Done                                                                |  8,657 |
| Stubbed (not counted as done)                                       |      0 |
| Declared only (not counted as done)                                 |      0 |
| Share done                                                          | 85.01% |

## By component

| Component                    | Routines |  Done | Share |
| ---------------------------- | -------: | ----: | ----: |
| Game code                    |    6,902 | 6,874 | 99.6% |
| Sony online library (SCE-RT) |    1,242 | 1,235 | 99.4% |
| DNAS2 and RCOM               |    1,490 |     0 |  0.0% |
| Sony EE libraries            |      550 |   548 | 99.6% |

## IOP modules

Each module the build rebuilds is measured against the file on the PAL disc, from the
`PAL:` tags under `src/iop/<module>/`. The disc also has `dnas271.img`, the Sony
replacement image with the DNAS modules, which the build copies from the disc and does
not rebuild.

| Module                                               | Routines | Done |  Share |
| ---------------------------------------------------- | -------: | ---: | -----: |
| [synth_s.irx](progress/pal-iop-synth-s.md)           |      142 |  142 | 100.0% |
| [softfx_s.irx](progress/pal-iop-softfx-s.md)         |       74 |   74 | 100.0% |
| [ezncnf_s.irx](progress/pal-iop-ezncnf-s.md)         |       22 |   22 | 100.0% |
| [eznctl_s-eur.irx](progress/pal-iop-eznctl-s-eur.md) |       27 |   27 | 100.0% |
| [libnetb-eur.irx](progress/pal-iop-libnetb-eur.md)   |       46 |   46 | 100.0% |

## Parts

Each part lists the routines of one component in a 64 KiB block of PAL addresses.

| Part                                                                                              | Component                    | Range                     | Routines | Done |  Share |
| ------------------------------------------------------------------------------------------------- | ---------------------------- | ------------------------- | -------: | ---: | -----: |
| [pal-game-code-00100000.md](progress/pal-game-code-00100000.md)                                   | Game code                    | `0x00100000`-`0x00110000` |      189 |  189 | 100.0% |
| [pal-game-code-00110000.md](progress/pal-game-code-00110000.md)                                   | Game code                    | `0x00110000`-`0x00120000` |      175 |  175 | 100.0% |
| [pal-game-code-00120000.md](progress/pal-game-code-00120000.md)                                   | Game code                    | `0x00120000`-`0x00130000` |      337 |  337 | 100.0% |
| [pal-game-code-00130000.md](progress/pal-game-code-00130000.md)                                   | Game code                    | `0x00130000`-`0x00140000` |      271 |  271 | 100.0% |
| [pal-game-code-00140000.md](progress/pal-game-code-00140000.md)                                   | Game code                    | `0x00140000`-`0x00150000` |      390 |  390 | 100.0% |
| [pal-game-code-00150000.md](progress/pal-game-code-00150000.md)                                   | Game code                    | `0x00150000`-`0x00160000` |      400 |  400 | 100.0% |
| [pal-game-code-00160000.md](progress/pal-game-code-00160000.md)                                   | Game code                    | `0x00160000`-`0x00170000` |      252 |  251 |  99.6% |
| [pal-game-code-00170000.md](progress/pal-game-code-00170000.md)                                   | Game code                    | `0x00170000`-`0x00180000` |      256 |  251 |  98.0% |
| [pal-game-code-00180000.md](progress/pal-game-code-00180000.md)                                   | Game code                    | `0x00180000`-`0x00190000` |      191 |  191 | 100.0% |
| [pal-game-code-00190000.md](progress/pal-game-code-00190000.md)                                   | Game code                    | `0x00190000`-`0x001a0000` |      211 |  211 | 100.0% |
| [pal-game-code-001a0000.md](progress/pal-game-code-001a0000.md)                                   | Game code                    | `0x001a0000`-`0x001b0000` |      244 |  243 |  99.6% |
| [pal-game-code-001b0000.md](progress/pal-game-code-001b0000.md)                                   | Game code                    | `0x001b0000`-`0x001c0000` |      329 |  329 | 100.0% |
| [pal-game-code-001c0000.md](progress/pal-game-code-001c0000.md)                                   | Game code                    | `0x001c0000`-`0x001d0000` |      237 |  237 | 100.0% |
| [pal-game-code-001d0000.md](progress/pal-game-code-001d0000.md)                                   | Game code                    | `0x001d0000`-`0x001e0000` |      173 |  173 | 100.0% |
| [pal-game-code-001e0000.md](progress/pal-game-code-001e0000.md)                                   | Game code                    | `0x001e0000`-`0x001f0000` |      201 |  201 | 100.0% |
| [pal-game-code-001f0000.md](progress/pal-game-code-001f0000.md)                                   | Game code                    | `0x001f0000`-`0x00200000` |      197 |  197 | 100.0% |
| [pal-game-code-00200000.md](progress/pal-game-code-00200000.md)                                   | Game code                    | `0x00200000`-`0x00210000` |      230 |  230 | 100.0% |
| [pal-game-code-00210000.md](progress/pal-game-code-00210000.md)                                   | Game code                    | `0x00210000`-`0x00220000` |      220 |  220 | 100.0% |
| [pal-game-code-00220000.md](progress/pal-game-code-00220000.md)                                   | Game code                    | `0x00220000`-`0x00230000` |      209 |  209 | 100.0% |
| [pal-game-code-00230000.md](progress/pal-game-code-00230000.md)                                   | Game code                    | `0x00230000`-`0x00240000` |      138 |  138 | 100.0% |
| [pal-game-code-00240000.md](progress/pal-game-code-00240000.md)                                   | Game code                    | `0x00240000`-`0x00250000` |      179 |  179 | 100.0% |
| [pal-game-code-00250000.md](progress/pal-game-code-00250000.md)                                   | Game code                    | `0x00250000`-`0x00260000` |      245 |  239 |  97.6% |
| [pal-game-code-00260000.md](progress/pal-game-code-00260000.md)                                   | Game code                    | `0x00260000`-`0x00270000` |      239 |  234 |  97.9% |
| [pal-game-code-00270000.md](progress/pal-game-code-00270000.md)                                   | Game code                    | `0x00270000`-`0x00280000` |      376 |  372 |  98.9% |
| [pal-game-code-00280000.md](progress/pal-game-code-00280000.md)                                   | Game code                    | `0x00280000`-`0x00290000` |      288 |  285 |  99.0% |
| [pal-game-code-00290000.md](progress/pal-game-code-00290000.md)                                   | Game code                    | `0x00290000`-`0x002a0000` |      372 |  372 | 100.0% |
| [pal-game-code-002a0000.md](progress/pal-game-code-002a0000.md)                                   | Game code                    | `0x002a0000`-`0x002b0000` |      346 |  343 |  99.1% |
| [pal-game-code-002b0000.md](progress/pal-game-code-002b0000.md)                                   | Game code                    | `0x002b0000`-`0x002c0000` |        6 |    6 | 100.0% |
| [pal-sony-online-library-sce-rt-002b0000.md](progress/pal-sony-online-library-sce-rt-002b0000.md) | Sony online library (SCE-RT) | `0x002b0000`-`0x002c0000` |      228 |  228 | 100.0% |
| [pal-sony-online-library-sce-rt-002c0000.md](progress/pal-sony-online-library-sce-rt-002c0000.md) | Sony online library (SCE-RT) | `0x002c0000`-`0x002d0000` |      238 |  238 | 100.0% |
| [pal-sony-online-library-sce-rt-002d0000.md](progress/pal-sony-online-library-sce-rt-002d0000.md) | Sony online library (SCE-RT) | `0x002d0000`-`0x002e0000` |      149 |  149 | 100.0% |
| [pal-sony-online-library-sce-rt-002e0000.md](progress/pal-sony-online-library-sce-rt-002e0000.md) | Sony online library (SCE-RT) | `0x002e0000`-`0x002f0000` |       28 |   28 | 100.0% |
| [pal-sony-online-library-sce-rt-002f0000.md](progress/pal-sony-online-library-sce-rt-002f0000.md) | Sony online library (SCE-RT) | `0x002f0000`-`0x00300000` |      204 |  204 | 100.0% |
| [pal-sony-online-library-sce-rt-00300000.md](progress/pal-sony-online-library-sce-rt-00300000.md) | Sony online library (SCE-RT) | `0x00300000`-`0x00310000` |      367 |  360 |  98.1% |
| [pal-sony-online-library-sce-rt-00310000.md](progress/pal-sony-online-library-sce-rt-00310000.md) | Sony online library (SCE-RT) | `0x00310000`-`0x00320000` |       28 |   28 | 100.0% |
| [pal-dnas2-and-rcom-00310000.md](progress/pal-dnas2-and-rcom-00310000.md)                         | DNAS2 and RCOM               | `0x00310000`-`0x00320000` |      237 |    0 |   0.0% |
| [pal-dnas2-and-rcom-00320000.md](progress/pal-dnas2-and-rcom-00320000.md)                         | DNAS2 and RCOM               | `0x00320000`-`0x00330000` |      183 |    0 |   0.0% |
| [pal-dnas2-and-rcom-00330000.md](progress/pal-dnas2-and-rcom-00330000.md)                         | DNAS2 and RCOM               | `0x00330000`-`0x00340000` |      288 |    0 |   0.0% |
| [pal-dnas2-and-rcom-00340000.md](progress/pal-dnas2-and-rcom-00340000.md)                         | DNAS2 and RCOM               | `0x00340000`-`0x00350000` |      214 |    0 |   0.0% |
| [pal-dnas2-and-rcom-00350000.md](progress/pal-dnas2-and-rcom-00350000.md)                         | DNAS2 and RCOM               | `0x00350000`-`0x00360000` |      258 |    0 |   0.0% |
| [pal-dnas2-and-rcom-00360000.md](progress/pal-dnas2-and-rcom-00360000.md)                         | DNAS2 and RCOM               | `0x00360000`-`0x00370000` |      236 |    0 |   0.0% |
| [pal-sony-ee-libraries-00370000.md](progress/pal-sony-ee-libraries-00370000.md)                   | Sony EE libraries            | `0x00370000`-`0x00380000` |      212 |  212 | 100.0% |
| [pal-dnas2-and-rcom-00370000.md](progress/pal-dnas2-and-rcom-00370000.md)                         | DNAS2 and RCOM               | `0x00370000`-`0x00380000` |       74 |    0 |   0.0% |
| [pal-sony-ee-libraries-00380000.md](progress/pal-sony-ee-libraries-00380000.md)                   | Sony EE libraries            | `0x00380000`-`0x00390000` |      338 |  336 |  99.4% |
| [pal-game-code-00400000.md](progress/pal-game-code-00400000.md)                                   | Game code                    | `0x00400000`-`0x00410000` |        1 |    1 | 100.0% |
