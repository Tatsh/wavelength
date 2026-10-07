#pragma once

#include <set>

#include "game/axetrackdata.h"
#include "gs/axeharmony.h"
#include "mid/midivalidator.h"
#include "mid/trackbuilder.h"

/**
 * TrackBuilder that reads the chords of a guitar track into AxeTrackData.
 *
 * The RTTI includes the class name and records TrackBuilder as the base. The object is 0x7c bytes.
 * The notes that start at one tick form a chord, which lasts until the next chord starts.
 */
class AxeTrackHarmonyBuilder : public TrackBuilder {
public:
    /**
     * Construct a builder.
     *
     * @param pszName The track name the error messages give.
     * @param bValidate Check the events for authoring errors.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param nChannel The MIDI channel of the track.
     * @param nIntroTicks The length of the song intro, which must not have notes.
     * @param pData The data to fill.
     * @ghidraAddress NTSC-U/C: 0x00148c00
     * @ghidraAddress PAL: 0x0014a5c0
     */
    AxeTrackHarmonyBuilder(const char *pszName,
                           bool bValidate,
                           ErrorHandler pfnError,
                           int nChannel,
                           int nIntroTicks,
                           AxeTrackData *pData);

    /**
     * Release the builder.
     *
     * @ghidraAddress NTSC-U/C: 0x00148d10
     * @ghidraAddress PAL: 0x0014a6d0
     */
    ~AxeTrackHarmonyBuilder() override;

    /**
     * Ignore the start of the track.
     *
     * @param nTrack The track index.
     * @ghidraAddress NTSC-U/C: 0x00348a68
     * @ghidraAddress PAL: 0x003b5e98
     */
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    /**
     * Add the last chord to the data, after reporting a track with no chord.
     *
     * @ghidraAddress NTSC-U/C: 0x00148db0
     * @ghidraAddress PAL: 0x0014a770
     */
    void OnEndTrack() override;

    /**
     * Ignore the end of the file.
     *
     * @ghidraAddress NTSC-U/C: 0x00348a70
     * @ghidraAddress PAL: 0x003b5ea0
     */
    void OnAllDone() override {
    }

    /**
     * Add a note to the chord of its tick, starting a new chord at a new tick.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x00148e10
     * @ghidraAddress PAL: 0x0014a7d0
     */
    void
    OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2) override;

    /**
     * Ignore a tempo change.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The new tempo.
     * @ghidraAddress NTSC-U/C: 0x00348a78
     * @ghidraAddress PAL: 0x003b5ea8
     */
    void OnTempo([[maybe_unused]] int nTick, [[maybe_unused]] int nMicrosecondsPerBeat) override {
    }

    /**
     * Ignore a text meta event.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The meta event type.
     * @ghidraAddress NTSC-U/C: 0x00348a80
     * @ghidraAddress PAL: 0x003b5eb0
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
     * @ghidraAddress NTSC-U/C: 0x00348a88
     * @ghidraAddress PAL: 0x003b5eb8
     */
    void OnTimeSignature([[maybe_unused]] int nTick,
                         [[maybe_unused]] int nNumerator,
                         [[maybe_unused]] int nDenominator) override {
    }

    AxeTrackData *mData;            /*!< The data to fill. */
    MidiValidator mValidator;       /*!< The checker of the channel messages. */
    int mIntroTicks;                /*!< The length of the song intro. */
    int mStartTick;                 /*!< The tick of the chord being built, or -1. */
    AxeHarmony *mHarmony;           /*!< The chord being built, or null. */
    std::set<unsigned char> mNotes; /*!< The notes of the chord that still sound. */
};
