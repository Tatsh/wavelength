<!-- markdownlint-configure-file {"MD024": { "siblings_only": true } } -->

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.1/), and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [unreleased]

### Added

- `ADD_USA_DEMO_SONG` CMake option (off by default) adds "Space" by P.O.D., the exclusive song of
  the _Amplitude (USA) (Demo)_ disc, to the built disc image in the manner of
  [Amplitude-Space-Edition](https://github.com/Squididly/Amplitude-Space-Edition).
  - `USA_DEMO_IMAGE` is the demo ISO image or disc root.
  - `USA_DEMO_SPACE_EDITION` is a checkout of Amplitude-Space-Edition. When it is unset and
    `USA_DEMO_SPACE_EDITION_DOWNLOAD` is `ON` (the default), CMake clones the repository at a
    pinned commit with Git.
  - With the song, the image is rebuilt from a disc root instead of patched in place.

### Changed

- Builds default to the `Debug` build type when `CMAKE_BUILD_TYPE` is not set.

## [1.0.0] - 2026-10-17

First release.

[unreleased]: https://github.com/Tatsh/wavelength/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/Tatsh/wavelength/releases/tag/v1.0.0
