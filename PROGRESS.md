# Progress

This report measures the European executable (`SCES-51706`) of the reconstruction against
the North American one (`SCUS-97258`). It counts every PAL routine the port must provide.
That is, each NTSC-U/C routine the port provides, carried to its PAL counterpart, and each
routine only the PAL executable has. A routine is done when a `PAL: 0x...` tag annotates its
PAL address and the tree has its body. A routine marked as a stub (an `@stub` tag in its
Doxygen block, or `(stub)` at the end of its address comment) is not counted as done, and
neither is a routine tagged in a header with no body in the tree yet. The DNAS2 library and its
RCOM crypto library (PAL `0x003115a0` to `0x003737a8`) are not recovered, and their
routines are counted as excluded. The library libdnas2 is encrypted, and RCOM is reachable
only through it. The four libdnas2 entry points the game calls are stubs and are counted
as stubbed. Six libnet and C library routines outside the range are called only from it,
and each is counted as excluded until the tree has its body.

Regenerate after every merged change, from a PAL build of the tree:

```shell
cmake -B build-pal -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/ps2-ee-toolchain.cmake \
  -DVIDEO_STANDARD=PAL && ninja -C build-pal
WAVELENGTH_BUILD="$PWD/build-pal" python3 .wiswa-ci/port/pal/progress.py "$PWD" --write
```

## Summary

| Measure                                                             |  Count |
| ------------------------------------------------------------------- | -----: |
| NTSC-U/C routines to port with a PAL counterpart                    |  8,590 |
| NTSC-U/C routines to port with no PAL counterpart found             |      3 |
| Routines only the PAL executable has                                |    112 |
| Excluded: Compiler-emitted instance                                 |  2,111 |
| Excluded: DNAS2 and RCOM (not recovered)                            |  1,486 |
| Excluded: Compiler RTTI                                             |  1,357 |
| Excluded: STL instance (GCC library)                                |    526 |
| Excluded: C and compiler runtime                                    |    342 |
| Excluded: Vendored under 3rdparty                                   |    296 |
| Excluded: PAL only: Compiler-emitted instances                      |     55 |
| Excluded: PD audit: compiler or library generated, or not a routine |     49 |
| PAL routines to provide                                             |  8,702 |
| Done                                                                |  8,698 |
| Stubbed (not counted as done)                                       |      4 |
| Declared only (not counted as done)                                 |      0 |
| Share done                                                          | 99.95% |

## By component

| Component                    | Routines |  Done | Stubbed |  Share |
| ---------------------------- | -------: | ----: | ------: | -----: |
| Game code                    |    6,906 | 6,906 |       0 | 100.0% |
| Sony online library (SCE-RT) |    1,242 | 1,242 |       0 | 100.0% |
| DNAS2 API called by the game |        4 |     0 |       4 |   0.0% |
| Sony EE libraries            |      550 |   550 |       0 | 100.0% |

## IOP modules

Each module the build rebuilds is measured against the file on the PAL disc, from the
`PAL:` tags under `src/iop/<module>/`. The disc also has `dnas271.img`, the Sony
replacement image with the DNAS modules, which the build copies from the disc and does
not rebuild.

| Module             | Routines | Done |  Share |
| ------------------ | -------: | ---: | -----: |
| `synth_s.irx`      |      142 |  142 | 100.0% |
| `softfx_s.irx`     |       74 |   74 | 100.0% |
| `ezncnf_s.irx`     |       22 |   22 | 100.0% |
| `eznctl_s-eur.irx` |       27 |   27 | 100.0% |
| `libnetb-eur.irx`  |       46 |   46 | 100.0% |
