#pragma once

#include <utility>
#include <vector>

#include "game/scratchtrackdata.h"
#include "gs/multimusebuilder.h"
#include "mid/midivalidator.h"
#include "mid/trackbuilder.h"

/**
 * TrackBuilder that reads one SCRATCH track of a song into the shared ScratchTrackData.
 *
 * The RTTI includes the class name and records TrackBuilder as the base. Each of the three
 * SCRATCH tracks fills one position of every scratcher set. A track holds its scratchers as
 * consecutive notes, three to a scratcher, one note on each of the three scratch buttons. The
 * other channel messages are the sound the scratchers cut their pieces from.
 */
class ScratchTrackBuilder : public TrackBuilder {
public:
    /**
     * Construct a builder.
     *
     * @param pszName The track name the error messages give.
     * @param bValidate Check the events for authoring errors.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param nChannel The MIDI channel of the track.
     * @param nIndex The position the track fills in each scratcher set.
     * @param nTicksPerBar The length of a bar.
     * @param nIntroTicks The length of the song intro, which must not have notes.
     * @param nReserved The value the constructor records at `+0x14`.
     * @param nUnused Ignored.
     * @param pData The data to fill.
     * @ghidraAddress NTSC-U/C: 0x001380d8
     * @ghidraAddress PAL: 0x00139938
     */
    ScratchTrackBuilder(const char *pszName,
                        bool bValidate,
                        ErrorHandler pfnError,
                        int nChannel,
                        int nIndex,
                        int nTicksPerBar,
                        int nIntroTicks,
                        int nReserved,
                        int nUnused,
                        ScratchTrackData *pData);

    /**
     * Release the builder.
     *
     * @ghidraAddress NTSC-U/C: 0x001381b0
     * @ghidraAddress PAL: 0x00139a10
     */
    ~ScratchTrackBuilder() override;

    /**
     * Ignore the start of the track.
     *
     * @param nTrack The track index.
     * @ghidraAddress NTSC-U/C: 0x00340010
     */
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    /**
     * Build a scratcher from each set of three notes, and check that every set is complete once
     * the last SCRATCH track ends.
     *
     * @ghidraAddress NTSC-U/C: 0x00138268
     * @ghidraAddress PAL: 0x00139ac8
     */
    void OnEndTrack() override;

    /**
     * Ignore the end of the file.
     *
     * @ghidraAddress NTSC-U/C: 0x00340018
     */
    void OnAllDone() override {
    }

    /**
     * Record a scratch button note, or collect any other channel message for the pieces.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x00138570
     * @ghidraAddress PAL: 0x00139dd0
     */
    void
    OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2) override;

    /**
     * Ignore a tempo change.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The new tempo.
     * @ghidraAddress NTSC-U/C: 0x00340020
     */
    void OnTempo([[maybe_unused]] int nTick, [[maybe_unused]] int nMicrosecondsPerBeat) override {
    }

    /**
     * Ignore a text meta event.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The meta event type.
     * @ghidraAddress NTSC-U/C: 0x00340028
     */
    void OnText([[maybe_unused]] int nTick,
                [[maybe_unused]] const char *pszText,
                [[maybe_unused]] unsigned char nType) override {
    }

    /**
     * Ignore a time signature.
     *
     * @param nTick The tick.
     * @param nNumerator The beats per bar.
     * @param nDenominator The beat unit.
     * @ghidraAddress NTSC-U/C: 0x00340030
     */
    void OnTimeSignature([[maybe_unused]] int nTick,
                         [[maybe_unused]] int nNumerator,
                         [[maybe_unused]] int nDenominator) override {
    }

    /**
     * Report the scratch button a note plays.
     *
     * @param nNote The note.
     * @return The button, 0 to 2, or -1 for a note that is not a scratch button.
     * @ghidraAddress NTSC-U/C: 0x00138078
     * @ghidraAddress PAL: 0x001398d8
     */
    static int ButtonForNote(unsigned char nNote);

    ScratchTrackData *mData;                    /*!< The data to fill. */
    int mReserved14;                            // +0x14, recorded and not yet identified.
    int mIndex;                                 /*!< The position the track fills in each set. */
    int mTicksPerBar;                           /*!< The length of a bar. */
    int mIntroTicks;                            /*!< The length of the song intro. */
    std::vector<std::pair<int, int> > mButtons; /*!< The start and end tick of each note. */
    MultiMuseBuilder mPieces;                   /*!< The sound the pieces are cut from. */
    MidiValidator mValidator;                   /*!< The checker of the channel messages. */
};
