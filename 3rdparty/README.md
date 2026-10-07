# Vendored sources

Each directory here is an upstream release, imported with its archive's top directory name. The
port's changes are made in place and are listed below. The formatting and checking tools skip this
directory, and upstream code retains its original style.

| Directory     | Upstream archive                                 | SHA-256 of the archive                                             | Files imported           |
| ------------- | ------------------------------------------------ | ------------------------------------------------------------------ | ------------------------ |
| `gzip-1.2.4/` | <https://ftp.gnu.org/gnu/gzip/gzip-1.2.4.tar.gz> | `1ca41818a23c9c59ef1d5e1d00c0d5eaa2285d931c0fb059637d7c0cc02ad967` | 5 of the release's files |

## gzip 1.2.4

Only `COPYING`, `README`, `gzip.h`, `inflate.c`, and `tailor.h` are imported. The port changes one
file. `inflate.c` calls `HuftReset()` before each block to rewind the table pool.
[src/os/INFLATE.md](../src/os/INFLATE.md) records the change and the version evidence.

## Verification

Download an archive, check its SHA-256, extract it, and compare each imported file with line endings
removed. For example, `cmp <(tr -d '\r' < gzip-1.2.4/inflate.c) <(tr -d '\r' < 3rdparty/gzip-1.2.4/inflate.c)`
reports the first difference in that file.
