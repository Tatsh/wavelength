# Vendored sources

Each directory here is an upstream release, imported with its archive's top directory name. The
port's changes are made in place and are listed below. The formatting and checking tools skip this
directory, and upstream code retains its original style.

| Directory           | Upstream archive                                                                                                  | SHA-256 of the archive                                             | Files imported            |
| ------------------- | ----------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------ | ------------------------- |
| `expat/`            | <ftp://ftp.jclark.com/pub/xml/expat1_2.zip>                                                                       | `49b9f33e45a8e8fb34bfa06d5aaf167ee61caf7de040f17f2241bb8ee9348451` | 17 of the release's files |
| `gzip-1.2.4/`       | <https://ftp.gnu.org/gnu/gzip/gzip-1.2.4.tar.gz>                                                                  | `1ca41818a23c9c59ef1d5e1d00c0d5eaa2285d931c0fb059637d7c0cc02ad967` | 16 of the release's files |
| `libtomcrypt-0.75/` | <https://web.archive.org/web/20030408094054/http://libtomcrypt.org/files/crypt-0.75.tar.bz2>                      | `a64f9baba6923e0ca4d096efc7ea7abe0cf7ec82e324af0f1080588e6d3ffa36` | 18 of the release's files |

## Expat 1.2

This is James Clark's last Expat release, from October 2000. The game's online library uses the parser for
UPnP replies. The binary's error table has the same 22 messages as this release, and its parser has
neither the memory-handling functions nor the namespace-triplet field that Expat 1.95.0 added. The
archive's top directory is `expat`.

Only the parser and tokenizer sources and their headers are imported, with `expat.html` for the
licence terms. The prebuilt Windows binaries, the project files, `xmlwf`, `gennmtab`, and the sample
are omitted. The port does not change these files yet.

## gzip 1.2.4

The game decompresses and compresses gzip streams. The imported files are `COPYING`, `README`,
`gzip.h`, `tailor.h`, `inflate.c`, the compression side (`bits.c`, `deflate.c`, `trees.c`, `zip.c`),
`util.c`, `unzip.c`, and `gzip.c`, with the headers they include (`crypt.h`, `getopt.h`, `lzw.h`,
and `revision.h`).

The game links `gzip.c`, `unzip.c`, `util.c`, `zip.c`, `bits.c`, `deflate.c`, `inflate.c`, and
`trees.c`. The port edits them in place to match the game. The external names and the in-memory
additions are those of the symbol table of the _FreQuency_ demonstration disc (`SCUS_971.57`),
which links an earlier copy of the same changes.

- `gzip.h` gives every external name a prefix (`gzip_zip`, `gzipInbuf`, and so on) and declares the
  memory source and the output routines.
- `tailor.h` has a section for the Emotion Engine. It selects the small buffers, the 260-byte name
  buffer, operating system code 11, and no input size check, and it reads files through
  `FileTableRead()`.
- `gzip.c` keeps the buffers, the variables the other modules use, and a reduced `get_method()`.
  The command-line program is removed. The game adds `GzipInit()`, a driver that compresses or
  decompresses to a buffer, the in-memory entry points, the trailer length reader, and
  `GzipFileName()`.
- `unzip.c` keeps a reduced `unzip()` that does not check the trailer.
- `util.c` reads from the memory source when the descriptor is -1 and copies its output to a
  buffer. `error()` and `read_error()` report the error without exiting. The routines the game
  does not link are removed.
- `zip.c` does not store the original name, and `file_read()` reads the memory source.
- `bits.c` always reads through `file_read()`.
- `inflate.c` calls `HuftReset()` before each block to rewind the table pool.
  [src/os/INFLATE.md](../src/os/INFLATE.md) records the change and the version evidence.

## LibTomCrypt 0.75

The game links LibTomCrypt 0.70, whose version string is `LibTomCrypt 0.70`. No copy of the 0.70
archive is published, and the project's git history starts at 0.75. The 0.75 archive is imported
from the project site's 2003 capture as the nearest release.

The game uses Blowfish, CBC mode, the cipher registry, and `zeromem()`. The imported files are
`blowfish.c`, `cbc.c`, `crypt.c`, `mem.c`, the `mycrypt*.h` headers, `authors`, `changes`, and
`crypt.tex`, the manual that records the licence terms. Three differences from 0.70 are known. 0.71
replaced the error strings with error codes, the game's `cbc_encrypt()` applies the IV before it
checks the cipher index, and the game's `cbc.c` has a routine between `cbc_start()` and
`cbc_encrypt()` that copies a new IV into a CBC state. The port is to make these files match 0.70 in
place. The port adds that routine to `cbc.c` as `cbc_setiv()`, a name that is inferred.

## Verification

Download an archive, check its SHA-256, extract it, and compare each imported file with line endings
removed. For example, `cmp <(tr -d '\r' < gzip-1.2.4/inflate.c) <(tr -d '\r' < 3rdparty/gzip-1.2.4/inflate.c)`
reports the first difference in that file.
