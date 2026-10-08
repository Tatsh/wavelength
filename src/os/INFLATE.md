# Bundled gzip inflate manifest

The game decodes its gzip data with `inflate.c` from GNU gzip 1.2.4, Mark Adler's public domain
decoder at version c10p1 (10 January 1993). The routines are C with unmangled names. The image also
links the gzip compression side (`zip`, `deflate`, `trees`, and `bits`), and no caller uses it.

The upstream files are vendored at `3rdparty/gzip-1.2.4/` with the release's top directory name
preserved. Only `inflate.c`, the two headers it includes (`gzip.h` and `tailor.h`), `README`, and
`COPYING` are imported. `gzip.h` and `tailor.h` are under the GPL, and `inflate.c` is public
domain.

[inflate.c](inflate.c) is the game side. It maps the upstream names onto the loader's gzip state,
replaces the allocator, includes the vendored `inflate.c`, and defines the table pool.

## Version evidence

| Feature in the image                                                             | 1.2.4 | 1.3.4 | 1.3.9 |
| -------------------------------------------------------------------------------- | ----- | ----- | ----- |
| `huft_build` reports 0 for all-zero lengths                                      | yes   | yes   | no    |
| `inflate_dynamic` does not reject a null bit-length table                        | yes   | no    | no    |
| `fill_inbuf` is called without first storing the window position to `outcnt`     | yes   | no    | no    |
| `" incomplete literal tree\n"` and `" incomplete distance tree\n"` via `fprintf` | yes   | yes   | no    |
| `lbits` 9 and `dbits` 6 as data (`0x003b1f04`, `0x003b1f08`)                     | yes   | yes   | yes   |

No 1.2 release after 1.2.4 exists, and the 1.3 changes above are absent from both regions.

The build options read from the image are these:

- `PKZIP_BUG_WORKAROUND` is undefined. The count test is `nl > 286 || nd > 30` (`sltiu` 287 and
  31), and an incomplete distance tree fails the block.
- `CRYPT` and `DEBUG` are undefined, and `NOMEMCPY` is undefined. `inflate_codes` copies through
  `memcpy` when the source and destination do not overlap.

## Game configuration

These macros precede the include and do not modify the upstream file:

| Upstream name  | Game definition                                          |
| -------------- | -------------------------------------------------------- |
| `inbuf`        | `gzipInbuf`                                              |
| `insize`       | `gzipInsize`                                             |
| `inptr`        | `gzipInptr`                                              |
| `outcnt`       | `gzipOutcnt` (upstream `wp`)                             |
| `window`       | `gzipWindow` (upstream `slide`)                          |
| `fill_inbuf`   | `GzipRefillInputBuffer`                                  |
| `flush_window` | `GzipFlushWindow`                                        |
| `malloc`       | `HuftMalloc`, given the entry count rather than the size |
| `free`         | Nothing                                                  |

`HuftMalloc` carves tables from a 2048-entry pool. It advances the cursor even when the allocation
fails, fails an allocation that ends exactly at the end of the pool, and logs
`"HUFT MEMORY EXCEEDED!!\n"` through `printf` on failure. `huft_build` calls it.

Retail `huft_free` is reduced to `return 0`. The vendored `huft_free` still walks the table chain
and calls the empty `free` for each table. The walk only reads the link entries of tables the same
block built, and the result is the same zero.

## Edit to the vendored source

`inflate()` calls `HuftReset()` before each block, ahead of `hufts = 0`. The call is marked with a
comment in `3rdparty/gzip-1.2.4/inflate.c`. `HuftReset` records the most pool entries one block has
used and rewinds the pool. The peak is never read elsewhere.

## Differences that do not change behaviour

- `ulg` is 64 bits wide in the image and 32 bits wide in this build. The bit buffer never stores
  more than 23 bits.
- Upstream's local `h` in `inflate()` tracks the most `hufts` of one block. It is unused without
  `DEBUG`, and the image omits it.

## Addresses

| Routine           | NTSC-U/C     | PAL          |
| ----------------- | ------------ | ------------ |
| `HuftReset`       | `0x00285268` | `0x0028eb18` |
| `HuftMalloc`      | `0x002852a0` | `0x0028eb50` |
| `huft_build`      | `0x002852f0` | `0x0028eba0` |
| `huft_free`       | `0x002858c8` | `0x0028f178` |
| `inflate_codes`   | `0x002858d0` | `0x0028f180` |
| `inflate_stored`  | `0x00285ec8` | `0x0028f778` |
| `inflate_fixed`   | `0x00286148` | `0x0028f9f8` |
| `inflate_dynamic` | `0x002862e8` | `0x0028fb98` |
| `inflate_block`   | `0x00286b20` | `0x002903d0` |
| `inflate`         | `0x00286cd8` | `0x00290588` |

| Global      | NTSC-U/C     |
| ----------- | ------------ |
| `border`    | `0x003b1d90` |
| `cplens`    | `0x003b1de0` |
| `cplext`    | `0x003b1e20` |
| `cpdist`    | `0x003b1e60` |
| `cpdext`    | `0x003b1ea0` |
| `mask_bits` | `0x003b1ee0` |
| `lbits`     | `0x003b1f04` |
| `dbits`     | `0x003b1f08` |
| `bb`        | `0x0047d748` |
| `bk`        | `0x0047d750` |
| `hufts`     | `0x0047d754` |
| `huftTable` | `0x00479748` |
| `pHuftNext` | `0x003b1d88` |
| `highWater` | `0x003b1d8c` |
