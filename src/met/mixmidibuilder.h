#pragma once

#include "gs/muse.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * Reader of the MIDI file of a menu song that builds a piece of music for each track a mix of the
 * song lists.
 *
 * The RTTI includes the class name in the nested classes MixMidiBuilder::BuilderReceiver and
 * MixMidiBuilder::TrackData. The class is not polymorphic. The object is 0x7c bytes. The file is
 * read a part at a time. Only the members MetaMusicSong uses are declared, and the routines are
 * not reconstructed.
 */
class MixMidiBuilder {
public:
    /**
     * Collect the track names of the `mix` entry of a menu song.
     *
     * @param pConfig The entry of the song.
     * @ghidraAddress NTSC-U/C: 0x0016aab0
     * @ghidraAddress PAL: 0x0016dc38
     */
    explicit MixMidiBuilder(DataArray *pConfig);

    /**
     * Release the pieces and the reader.
     *
     * @ghidraAddress NTSC-U/C: 0x0016acf0
     * @ghidraAddress PAL: 0x0016de78
     */
    ~MixMidiBuilder();

    /**
     * Start reading the file the `music_shared_midi_file` entry of the metagame configuration
     * specifies into a buffer.
     *
     * @ghidraAddress NTSC-U/C: 0x0016ae78
     * @ghidraAddress PAL: 0x0016e000
     */
    void StartLoad();

    /**
     * Read more of the file, or parse more of its events once it has been read.
     *
     * @ghidraAddress NTSC-U/C: 0x0016af58
     * @ghidraAddress PAL: 0x0016e0e0
     */
    void Poll();

    /**
     * Report whether every event has been parsed.
     *
     * @return Whether the pieces are built.
     * @ghidraAddress NTSC-U/C: 0x0016b048
     * @ghidraAddress PAL: 0x0016e1d0
     */
    bool IsDone() const;

    /**
     * Report the number of pieces built.
     *
     * @return The number of pieces.
     * @ghidraAddress NTSC-U/C: 0x0016b050
     * @ghidraAddress PAL: 0x0016e1d8
     */
    int NumTracks() const;

    /**
     * Report a piece.
     *
     * @param nTrack The index of the piece.
     * @return The piece.
     * @ghidraAddress NTSC-U/C: 0x0016b070
     * @ghidraAddress PAL: 0x0016e1f8
     */
    Muse *TrackMuse(int nTrack) const;

    /**
     * Report the MIDI channel of a piece.
     *
     * @param nTrack The index of the piece.
     * @return The channel.
     * @ghidraAddress NTSC-U/C: 0x0016b098
     * @ghidraAddress PAL: 0x0016e220
     */
    unsigned char TrackChannel(int nTrack) const;

    /**
     * Report the name of the track of a piece.
     *
     * @param nTrack The index of the piece.
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x0016b0b0
     * @ghidraAddress PAL: 0x0016e238
     */
    const String &TrackName(int nTrack) const;
};
