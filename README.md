# Amplitude

<!-- WISWA-GENERATED-README:START -->

[![C++](https://img.shields.io/badge/C++-00599C?logo=c%2B%2B)](https://isocpp.org)
[![GitHub tag (with filter)](https://img.shields.io/github/v/tag/Tatsh/wavelength)](https://github.com/Tatsh/wavelength/tags)
[![License](https://img.shields.io/github/license/Tatsh/wavelength)](https://github.com/Tatsh/wavelength/blob/master/LICENSE.txt)
[![GitHub commits since latest release (by SemVer including pre-releases)](https://img.shields.io/github/commits-since/Tatsh/wavelength/v0.0.0/master)](https://github.com/Tatsh/wavelength/compare/v0.0.0...master)
[![Dependabot](https://img.shields.io/badge/Dependabot-enabled-blue?logo=dependabot)](https://github.com/dependabot)
[![Stargazers](https://img.shields.io/github/stars/Tatsh/wavelength?logo=github&style=flat)](https://github.com/Tatsh/wavelength/stargazers)
[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/Tatsh/wavelength/master.svg)](https://results.pre-commit.ci/latest/github/Tatsh/wavelength/master)
[![CMake](https://img.shields.io/badge/CMake-6E6E6E?logo=cmake)](https://cmake.org/)
[![Prettier](https://img.shields.io/badge/Prettier-black?logo=prettier)](https://prettier.io/)

[![@Tatsh](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fpublic.api.bsky.app%2Fxrpc%2Fapp.bsky.actor.getProfile%2F%3Factor=did%3Aplc%3Auq42idtvuccnmtl57nsucz72&query=%24.followersCount&label=Follow+%40Tatsh&logo=bluesky&style=social)](https://bsky.app/profile/Tatsh.bsky.social)
[![Mastodon Follow](https://img.shields.io/mastodon/follow/109370961877277568?domain=hostux.social&style=social)](https://hostux.social/@Tatsh)

<!-- WISWA-GENERATED-README:STOP -->

Reconstructed source code for _Amplitude_, the 2003 PlayStation 2 rhythm game developed by Harmonix
Music Systems and published by Sony Computer Entertainment.

The source is being ported from the reconstruction of _FreQuency_, the predecessor of _Amplitude_.
The game does not build yet.

## Building

The build uses the ps2dev cross compilers for the Emotion Engine and the IOP, their C and C++
runtime, and the IRX fixup tool (`srxfixup`). It does not use ps2sdk. Every Sony library routine
the game calls is reconstructed under `sce/`, and `sce/ee/runtime` connects the compiler's runtime
to the reconstructed kernel.

```shell
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/ps2-ee-toolchain.cmake -DWAVELENGTH_DISC_IMAGE=/path/to/Amplitude.iso
cmake --build build --target image
```

The `image` target writes `build/wavelength.iso`, an ISO image of the DVD, with the built
executable and IOP modules in place of the originals. The original is an ISO image of the North
American release (`SCUS_972.58`) or the European release (`SCES_517.06`). Without
`WAVELENGTH_DISC_IMAGE`, the build produces the executable and the IOP modules only. File names on
the disc are matched without regard to case, and each name must be unique once case is ignored.

`WAVELENGTH_DISC_IMAGE` may also be the disc root, the directory with `SYSTEM.CNF`. A copy of the
disc files lacks the boot logo that the original disc stores in its system area, the first 16
sectors of the ISO image. A console may refuse a disc without the boot logo. Set
`WAVELENGTH_DISC_SYSTEM_AREA` to a file with the first 32768 bytes of the original ISO image to
include the boot logo.

### IOP modules

The game's own IOP modules are rebuilt from `src/iop`, one directory per module, with the IOP
toolchain (`mipsel-none-elf`) from `PS2DEV`. Each module is registered in `src/iop/CMakeLists.txt`
with one line that lists its sources in link order:

```cmake
wavelength_add_irx(softfx_s softfx.cpp lock.cpp periodictimer.cpp effects.cpp ringbuffer.cpp
                   streambuffer.cpp imports.S)
```

The function builds `build/iop/<module>/<module>.irx`, and the `image` target puts it on the disc in
place of the module with the same name in `IOP`. A module lists its import tables in an assembler
source with the macros of `sce/iop/include/iopimports.s`, and a module with C++ sources also links
the runtime in `src/iop/runtime`. The `iop_modules` target builds every module. Builds for PAL
define `VIDEO_STANDARD_PAL` for the modules that differ between the releases.

## Provenance and licence

This is an independent reverse-engineering effort for preservation and study. It includes no code
and no assets copied from the game. _Amplitude_ and its assets remain the property of their
respective rights holders, and this project is not affiliated with or endorsed by Harmonix or
Sony Interactive Entertainment.

Third-party code (the gzip decompressor) remains under its licence and is identified as such under
`3rdparty/`.
