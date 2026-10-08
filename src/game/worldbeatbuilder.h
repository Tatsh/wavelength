#pragma once

#include "game/worldtrack.h"
#include "mid/midivalidator.h"
#include "mid/trackbuilder.h"

/**
 * TrackBuilder that collects the events of the track named "WORLD".
 *
 * The RTTI includes the class name and records TrackBuilder as the base. The object is 0x64 bytes.
 * The routines with an address in the binary are not reconstructed.
 */
class WorldBeatBuilder : public TrackBuilder {
public:
    /**
     * Construct a builder.
     *
     * @param nTrack The number of the MIDI track the error messages give.
     * @param bValidate Check the events for authoring errors.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param nIntroTicks The length of the song intro.
     * @param pTrack The events to fill.
     * @ghidraAddress NTSC-U/C: 0x00281568
     * @ghidraAddress PAL: 0x0028ae68
     */
    WorldBeatBuilder(
        int nTrack, bool bValidate, ErrorHandler pfnError, int nIntroTicks, WorldTrack *pTrack);

    /**
     * Release the builder.
     *
     * @ghidraAddress NTSC-U/C: 0x002815f0
     * @ghidraAddress PAL: 0x0028aef0
     */
    ~WorldBeatBuilder() override;

    /**
     * Ignore the start of the track.
     *
     * @param nTrack The track.
     */
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    /** Ignore the end of the track. */
    void OnEndTrack() override {
    }

    /** Ignore the end of the file. */
    void OnAllDone() override {
    }

    /**
     * Record the event a channel message marks.
     *
     * @param nTick The tick of the message.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x00281640
     * @ghidraAddress PAL: 0x0028af40
     */
    void
    OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2) override;

    /**
     * Ignore a tempo change.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The tempo.
     */
    void OnTempo([[maybe_unused]] int nTick, [[maybe_unused]] int nMicrosecondsPerBeat) override {
    }

    /**
     * Ignore a text event.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The type of the event.
     */
    void OnText([[maybe_unused]] int nTick,
                [[maybe_unused]] const char *pszText,
                [[maybe_unused]] unsigned char nType) override {
    }

    /**
     * Ignore a time signature.
     *
     * @param nTick The tick.
     * @param nNumerator The beats in a bar.
     * @param nDenominator The length of a beat.
     */
    void OnTimeSignature([[maybe_unused]] int nTick,
                         [[maybe_unused]] int nNumerator,
                         [[maybe_unused]] int nDenominator) override {
    }

    int mIntroTicks;          /*!< The length of the song intro. */
    WorldTrack *mTrack;       /*!< The events to fill. */
    MidiValidator mValidator; /*!< The checker of the messages. */
};
