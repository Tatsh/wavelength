#pragma once

#include <vector>

#include "game/pitchtrackriffdata.h"
#include "gs/musefactory.h"
#include "mid/midivalidator.h"
#include "mid/trackbuilder.h"

/**
 * Builder of the riffs of a pitch track from its MIDI track.
 *
 * The RTTI records the class as deriving from TrackBuilder. Gem notes 96, 100, and 103 mark where
 * the riff of each gem button starts, in that order, and every other message belongs to the riffs.
 * The names of the members other than the MidiReceiver overrides are inferred.
 */
class PitchTrackBuilder : public TrackBuilder {
public:
    /** The start of the riff of a gem button. */
    struct Gem {
        signed char mSlot; /*!< The gem button. */
        int mTick;         /*!< The tick the riff starts at, from the start of the song. */
    };

    /**
     * Construct a builder.
     *
     * @param nTrack The number of the MIDI track the error messages give.
     * @param bValidate Check the events for authoring errors.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param riffName The name the riff data receives.
     * @param nChannel The channel every message must use.
     * @param nStartTick The tick the song starts at, after its intro.
     * @param nEndTick The tick the song ends at.
     * @param pRiffData The riff data built.
     * @ghidraAddress NTSC-U/C: 0x00129978
     * @ghidraAddress PAL: 0x0012b170
     */
    PitchTrackBuilder(int nTrack,
                      bool bValidate,
                      ErrorHandler pfnError,
                      const String &riffName,
                      int nChannel,
                      int nStartTick,
                      int nEndTick,
                      PitchTrackRiffData *pRiffData);

    /**
     * Release the builder.
     *
     * @ghidraAddress NTSC-U/C: 0x0033cc68
     * @ghidraAddress PAL: 0x003aa1a0
     */
    ~PitchTrackBuilder() override {
    }

    /**
     * Ignore the start of the track. The body is empty.
     *
     * @param nTrack The track index.
     * @ghidraAddress NTSC-U/C: 0x0033cd18
     */
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    /**
     * Cut the riffs of each gem button out of the collected messages, and add every three to the
     * riff data.
     *
     * @ghidraAddress NTSC-U/C: 0x00129a48
     * @ghidraAddress PAL: 0x0012b240
     */
    void OnEndTrack() override;

    /**
     * Ignore the end of the file. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x0033cd20
     */
    void OnAllDone() override {
    }

    /**
     * Record a gem note, or collect any other message.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x00129bb0
     * @ghidraAddress PAL: 0x0012b3a8
     */
    void
    OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2) override;

    /**
     * Ignore a tempo change. The body is empty.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The new tempo.
     * @ghidraAddress NTSC-U/C: 0x0033cd28
     */
    void OnTempo([[maybe_unused]] int nTick, [[maybe_unused]] int nMicrosecondsPerBeat) override {
    }

    /**
     * Ignore a text meta event. The body is empty.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The meta event type.
     * @ghidraAddress NTSC-U/C: 0x0033cd30
     */
    void OnText([[maybe_unused]] int nTick,
                [[maybe_unused]] const char *pszText,
                [[maybe_unused]] unsigned char nType) override {
    }

    /**
     * Ignore a time signature. The body is empty.
     *
     * @param nTick The tick.
     * @param nNumerator The beats per bar.
     * @param nDenominator The beat unit.
     * @ghidraAddress NTSC-U/C: 0x0033cd38
     */
    void OnTimeSignature([[maybe_unused]] int nTick,
                         [[maybe_unused]] int nNumerator,
                         [[maybe_unused]] int nDenominator) override {
    }

private:
    /**
     * Report the gem button a gem note marks.
     *
     * @param nNote The note.
     * @return The gem button, or -1 for a note that is not a gem note.
     * @ghidraAddress NTSC-U/C: 0x00129918
     * @ghidraAddress PAL: 0x0012b110
     */
    static int GetGemSlot(unsigned char nNote);

    int mStartTick;                /*!< The tick the song starts at, after its intro. */
    int mEndTick;                  /*!< The tick the song ends at. */
    PitchTrackRiffData *mRiffData; /*!< The riff data built. */
    MidiValidator mValidator;      /*!< The checker of the messages. */
    MuseFactory mRiffs;            /*!< The collected messages of the riffs. */
    std::vector<Gem> mGems;        /*!< The gem notes, in tick order. */
};
