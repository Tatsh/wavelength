#pragma once

#include <expected>
#include <filesystem>

#include "error.h"

/** Adder of the P.O.D. demo's exclusive song to an extracted Amplitude disc. */
namespace Tools::DemoSong {

using Common::Error;

/** The inputs and the extracted disc that DemoSong::add() changes. */
struct Inputs {
    std::filesystem::path arkRoot;      /*!< The files of the retail `gen/main.ark`. */
    std::filesystem::path discRoot;     /*!< The files of the retail disc. */
    std::filesystem::path demoImage;    /*!< The P.O.D. demo ISO image or its disc root. */
    std::filesystem::path spaceEdition; /*!< The Amplitude-Space-Edition checkout. */
    bool pal = false;                   /*!< Whether the retail disc is the European release. */
};

/**
 * Adds the song `POD_DEMO` ("Space" by P.O.D.) to an extracted retail disc, the way
 * Amplitude-Space-Edition does.
 *
 * The song's charts, sample banks, movies, and band art come from `songs/pod_demo` in the demo's
 * archive. Amplitude-Space-Edition provides the recorded streams in its `ark` and `disc` trees,
 * which are copied over the archive files and the disc files, and its decompiled `gen` files,
 * whose `POD_DEMO` song entry and `POD_DEMO_*` strings are spliced into the retail compiled
 * files. Every other part of the retail files is written back unchanged. The European release
 * reads its songs from `cfg_freq2euro` and has a string table per language, and each table gets
 * the English strings.
 */
class DemoSong {
public:
    /**
     * Add the song.
     *
     * @param inputs The inputs and the extracted disc.
     * @return Nothing, or an error when an input lacks a file the song needs, a compiled file does
     * not round-trip unchanged through the writer, or a file cannot be written.
     */
    static std::expected<void, Error> add(const Inputs &inputs);
};

} // namespace Tools::DemoSong
