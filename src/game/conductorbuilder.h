#pragma once

#include "game/playmap.h"
#include "mid/trackbuilder.h"

/**
 * TrackBuilder that reads the time signature of the song and checks its tempo.
 *
 * The RTTI includes the class name and records TrackBuilder as the base. The time signature sets
 * the length of a bar and creates the play map of the song.
 */
class ConductorBuilder : public TrackBuilder {
public:
    /**
     * Construct a builder.
     *
     * @param nTrack The number of the MIDI track the error messages give.
     * @param bValidate Check the events for authoring errors.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param nNumBars The length of the song in bars, which the play map ends at.
     * @param nTicksPerBeat The song ticks in one beat.
     * @param pfMsPerTick The length of a tick that the song text gives, in milliseconds.
     * @param ppPlayMap Receives the play map the time signature creates.
     * @param pTicksPerBar Receives the song ticks in one bar, and must be 0 at the start.
     * @ghidraAddress NTSC-U/C: 0x0014e5c0
     * @ghidraAddress PAL: 0x0014ff60
     */
    ConductorBuilder(int nTrack,
                     bool bValidate,
                     ErrorHandler pfnError,
                     int nNumBars,
                     int nTicksPerBeat,
                     const float *pfMsPerTick,
                     PlayMap **ppPlayMap,
                     int *pTicksPerBar);

    /**
     * Release the builder.
     *
     * @ghidraAddress NTSC-U/C: 0x0014e648
     * @ghidraAddress PAL: 0x0014ffe8
     */
    ~ConductorBuilder() override;

    /**
     * Ignore the start of the track.
     *
     * @param nTrack The track index.
     * @ghidraAddress NTSC-U/C: 0x0034b000
     * @ghidraAddress PAL: 0x003b8430
     */
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    /**
     * Ignore the end of the track.
     *
     * @ghidraAddress NTSC-U/C: 0x0034b020
     * @ghidraAddress PAL: 0x003b8450
     */
    void OnEndTrack() override {
    }

    /**
     * Ignore the end of the file.
     *
     * @ghidraAddress NTSC-U/C: 0x0034b008
     * @ghidraAddress PAL: 0x003b8438
     */
    void OnAllDone() override {
    }

    /**
     * Ignore a channel message.
     *
     * @param nTick The tick of the message.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0034b010
     * @ghidraAddress PAL: 0x003b8440
     */
    void OnMidi([[maybe_unused]] int nTick,
                [[maybe_unused]] unsigned char nStatus,
                [[maybe_unused]] unsigned char nData1,
                [[maybe_unused]] unsigned char nData2) override {
    }

    /**
     * Report an error when a validating builder reads a tempo that differs from the song text by
     * more than a tenth of a millisecond a beat.
     *
     * @param nTick The tick of the tempo change.
     * @param nMicrosecondsPerBeat The tempo.
     * @ghidraAddress NTSC-U/C: 0x0014e670
     * @ghidraAddress PAL: 0x00150010
     */
    void OnTempo(int nTick, int nMicrosecondsPerBeat) override;

    /**
     * Ignore a text event.
     *
     * @param nTick The tick of the event.
     * @param pszText The text.
     * @param nType The type of the event.
     * @ghidraAddress NTSC-U/C: 0x0034b018
     * @ghidraAddress PAL: 0x003b8448
     */
    void OnText([[maybe_unused]] int nTick,
                [[maybe_unused]] const char *pszText,
                [[maybe_unused]] unsigned char nType) override {
    }

    /**
     * Set the length of a bar and create the play map. A second time signature is an error, and
     * replaces the first.
     *
     * @param nTick The tick of the time signature.
     * @param nNumerator The beats in one bar.
     * @param nDenominator The note value of a beat, which is not read.
     * @ghidraAddress NTSC-U/C: 0x0014e718
     * @ghidraAddress PAL: 0x001500b8
     */
    void OnTimeSignature(int nTick, int nNumerator, int nDenominator) override;

    int mNumBars;            /*!< The length of the song in bars. */
    int mTicksPerBeat;       /*!< The song ticks in one beat. */
    const float *mMsPerTick; /*!< The length of a tick that the song text gives. */
    PlayMap **mPlayMap;      /*!< Receives the play map. */
    int *mTicksPerBar;       /*!< Receives the song ticks in one bar. */
};
