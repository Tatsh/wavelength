#pragma once

#include "gs/muse.h"
#include "gs/musefactory.h"
#include "mid/midivalidator.h"
#include "mid/trackbuilder.h"
#include "os/ptr.h"

/**
 * TrackBuilder that collects the channel messages of a track into a piece of music.
 *
 * The RTTI includes the class name and records TrackBuilder as the base. The object is 0x8c
 * bytes. At the end of the track the builder stores the piece through the owner it was given.
 * The routines with an address in the binary are not reconstructed.
 */
class MuseBuilder : public TrackBuilder {
public:
    /**
     * Construct a builder.
     *
     * @param pszName The track name the error messages give.
     * @param bValidate Check the events for authoring errors.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param pMuse The owner the finished piece is stored through.
     * @ghidraAddress NTSC-U/C: 0x0027e6f8
     * @ghidraAddress PAL: 0x00288010
     */
    MuseBuilder(const char *pszName, bool bValidate, ErrorHandler pfnError, Ptr<Muse> *pMuse);

    /**
     * Release the builder.
     *
     * @ghidraAddress NTSC-U/C: 0x0027e778
     * @ghidraAddress PAL: 0x00288090
     */
    ~MuseBuilder() override;

    /**
     * Ignore the start of the track.
     *
     * @param nTrack The track index.
     * @ghidraAddress NTSC-U/C: 0x003a5a58
     */
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    /**
     * Build the piece from the collected messages and store it through the owner.
     *
     * @ghidraAddress NTSC-U/C: 0x0027e7d8
     * @ghidraAddress PAL: 0x002880f0
     */
    void OnEndTrack() override;

    /**
     * Ignore the end of the file.
     *
     * @ghidraAddress NTSC-U/C: 0x003a5a60
     */
    void OnAllDone() override {
    }

    /**
     * Check and collect a channel message.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0027e828
     * @ghidraAddress PAL: 0x00288140
     */
    void
    OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2) override;

    /**
     * Ignore a tempo change.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The new tempo.
     * @ghidraAddress NTSC-U/C: 0x003a5a68
     */
    void OnTempo([[maybe_unused]] int nTick, [[maybe_unused]] int nMicrosecondsPerBeat) override {
    }

    /**
     * Ignore a text meta event.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The meta event type.
     * @ghidraAddress NTSC-U/C: 0x003a5a70
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
     * @ghidraAddress NTSC-U/C: 0x003a5a78
     */
    void OnTimeSignature([[maybe_unused]] int nTick,
                         [[maybe_unused]] int nNumerator,
                         [[maybe_unused]] int nDenominator) override {
    }

    Ptr<Muse> *mMuse;         /*!< The owner the finished piece is stored through. */
    MidiValidator mValidator; /*!< The checker of the messages. */
    MuseFactory mPieces;      /*!< The collected messages. */
};
