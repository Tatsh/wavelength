#pragma once

#include "met/metamusic.h"
#include "script/dataarray.h"

/**
 * One menu song of the `songs` entry of the metagame configuration, loading its tracks into a
 * MetaMusic.
 *
 * The class is not polymorphic and has no RTTI, so its name is inferred. Only the members the
 * metagame uses are declared.
 */
class MetaMusicSong {
public:
    /**
     * Build the song and the path of its sample bank.
     *
     * @param pMusic The music the tracks load into.
     * @param pConfig The entry of the song, with its `bank_file` and `mix` entries.
     * @ghidraAddress NTSC-U/C: 0x0016a440
     * @ghidraAddress PAL: 0x0016d5c8
     */
    MetaMusicSong(MetaMusic *pMusic, DataArray *pConfig);

    /**
     * Release the reader of the song.
     *
     * @ghidraAddress NTSC-U/C: 0x0016a4b0
     * @ghidraAddress PAL: 0x0016d638
     */
    ~MetaMusicSong();

    /**
     * Start reading the song's MIDI file.
     *
     * @ghidraAddress NTSC-U/C: 0x0016a510
     * @ghidraAddress PAL: 0x0016d698
     */
    void StartLoad();

    /**
     * Read more of the song, and hand its tracks to the music once the file has been read.
     *
     * @ghidraAddress NTSC-U/C: 0x0016a558
     * @ghidraAddress PAL: 0x0016d6e0
     */
    void Poll();

    /**
     * Report whether the tracks have been handed to the music.
     *
     * @return Whether the song has loaded.
     * @ghidraAddress NTSC-U/C: 0x0016a5b0
     * @ghidraAddress PAL: 0x0016d738
     */
    bool IsLoaded() const;

    /**
     * Report the sample bank of the song.
     *
     * @return The bank file.
     * @ghidraAddress NTSC-U/C: 0x0016a5c8
     * @ghidraAddress PAL: 0x0016d750
     */
    const char *BankFile() const;
};
