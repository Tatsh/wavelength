#pragma once

#include "game/backmusic.h"
#include "gs/multimusebuilder.h"
#include "mid/midivalidator.h"
#include "mid/trackbuilder.h"

/**
 * TrackBuilder that cuts the channel messages of a background music track into one muse per bar.
 *
 * The RTTI includes the class name and records TrackBuilder as the base. The object is 0x98 bytes.
 */
class BackMusicBuilder : public TrackBuilder {
public:
    /**
     * Construct a builder.
     *
     * @param pszName The track name the error messages give.
     * @param bValidate Check the events for authoring errors.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param nIntroBars The bars before bar 0.
     * @param nNumBars The length of the song in bars.
     * @param nTicksPerBar The song ticks in one bar.
     * @param pMusic The music to fill.
     * @ghidraAddress NTSC-U/C: 0x00149950
     * @ghidraAddress PAL: 0x0014b310
     */
    BackMusicBuilder(const char *pszName,
                     bool bValidate,
                     ErrorHandler pfnError,
                     int nIntroBars,
                     int nNumBars,
                     int nTicksPerBar,
                     BackMusic *pMusic);

    /**
     * Release the builder.
     *
     * @ghidraAddress NTSC-U/C: 0x00149a00
     * @ghidraAddress PAL: 0x0014b3c0
     */
    ~BackMusicBuilder() override;

    /**
     * Ignore the start of the track.
     *
     * @param nTrack The track index.
     * @ghidraAddress NTSC-U/C: 0x00349a48
     * @ghidraAddress PAL: 0x003b6e78
     */
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    /**
     * Give the music the muse of every bar.
     *
     * @ghidraAddress NTSC-U/C: 0x00149a60
     * @ghidraAddress PAL: 0x0014b420
     */
    void OnEndTrack() override;

    /**
     * Ignore the end of the file.
     *
     * @ghidraAddress NTSC-U/C: 0x00349a50
     * @ghidraAddress PAL: 0x003b6e80
     */
    void OnAllDone() override {
    }

    /**
     * Collect a channel message.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x00149af8
     * @ghidraAddress PAL: 0x0014b4b8
     */
    void
    OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2) override;

    /**
     * Ignore a tempo change.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The new tempo.
     * @ghidraAddress NTSC-U/C: 0x00349a58
     * @ghidraAddress PAL: 0x003b6e88
     */
    void OnTempo([[maybe_unused]] int nTick, [[maybe_unused]] int nMicrosecondsPerBeat) override {
    }

    /**
     * Ignore a text meta event.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The meta event type.
     * @ghidraAddress NTSC-U/C: 0x00349a60
     * @ghidraAddress PAL: 0x003b6e90
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
     * @ghidraAddress NTSC-U/C: 0x00349a68
     * @ghidraAddress PAL: 0x003b6e98
     */
    void OnTimeSignature([[maybe_unused]] int nTick,
                         [[maybe_unused]] int nNumerator,
                         [[maybe_unused]] int nDenominator) override {
    }

    int mIntroBars;           /*!< The bars before bar 0. */
    int mNumBars;             /*!< The length of the song in bars. */
    int mTicksPerBar;         /*!< The song ticks in one bar. */
    BackMusic *mMusic;        /*!< The music to fill. */
    MidiValidator mValidator; /*!< The checker of the channel messages. */
    MultiMuseBuilder mPieces; /*!< The collected channel messages. */
};
