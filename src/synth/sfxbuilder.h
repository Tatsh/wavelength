#pragma once

#include "gs/musebuilder.h"
#include "mid/midireceiver.h"

/**
 * Reader of the interface sound file that builds each named track into its cue of TheFxMidi.
 *
 * The RTTI records the class as deriving from MidiReceiver, and the vtable is at `0x003d6720`.
 * The first track only sets the tempo. Each later track names its cue in its track name meta event,
 * and a MuseBuilder collects the track's messages into that cue.
 */
class SFXBuilder : public MidiReceiver {
public:
    /**
     * Construct a builder for a file.
     *
     * @param pszFile The MIDI file.
     * @ghidraAddress NTSC-U/C: 0x0027ebd0
     * @ghidraAddress PAL: 0x002884b8
     */
    explicit SFXBuilder(const char *pszFile) : mFile(pszFile), mBuilder(nullptr), mTrack(0) {
    }

    /**
     * Destroy the builder of the current track.
     *
     * @ghidraAddress NTSC-U/C: 0x0027ebf0
     * @ghidraAddress PAL: 0x002884d8
     */
    ~SFXBuilder() override;

    /**
     * Read the file.
     *
     * @ghidraAddress NTSC-U/C: 0x0027ec68
     * @ghidraAddress PAL: 0x00288550
     */
    void Read();

    /**
     * Ignore the start of a track.
     *
     * @param nTrack The track index.
     * @ghidraAddress NTSC-U/C: 0x003a63c8
     */
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    /**
     * Finish the cue of the track and count the track.
     *
     * @ghidraAddress NTSC-U/C: 0x0027eca0
     * @ghidraAddress PAL: 0x00288588
     */
    void OnEndTrack() override;

    /**
     * Ignore the end of the file.
     *
     * @ghidraAddress NTSC-U/C: 0x003a63d0
     */
    void OnAllDone() override {
    }

    /**
     * Pass a channel message to the builder of the track, if any.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0027f878
     * @ghidraAddress PAL: 0x00289160
     */
    void
    OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2) override;

    /**
     * Set TheSfxTickDuration from the tempo, at 480 ticks a beat.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The tempo.
     * @ghidraAddress NTSC-U/C: 0x0027ed10
     * @ghidraAddress PAL: 0x002885f8
     */
    void OnTempo(int nTick, int nMicrosecondsPerBeat) override;

    /**
     * Start the builder of the cue a track name identifies, after the first track.
     *
     * The name is compared in capitals. An unknown name reports an error that the shipped build
     * discards, and the track is not built.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The meta event type.
     * @ghidraAddress NTSC-U/C: 0x0027ed68
     * @ghidraAddress PAL: 0x00288650
     */
    void OnText(int nTick, const char *pszText, unsigned char nType) override;

    /**
     * Ignore a time signature.
     *
     * @param nTick The tick.
     * @param nNumerator The beats per bar.
     * @param nDenominator The beat unit.
     * @ghidraAddress NTSC-U/C: 0x003a63d8
     */
    void OnTimeSignature([[maybe_unused]] int nTick,
                         [[maybe_unused]] int nNumerator,
                         [[maybe_unused]] int nDenominator) override {
    }

private:
    const char *mFile;     // The MIDI file.
    MuseBuilder *mBuilder; // The builder of the current track's cue, or null.
    int mTrack;            // The number of tracks ended so far.
};

/**
 * The duration of one tick of the interface sounds, which the last tempo of the file set.
 *
 * @ghidraAddress NTSC-U/C: 0x00440d5c
 */
extern float *TheSfxTickDuration;
