#pragma once

#include <vector>

#include "game/catchtrackdata.h"
#include "game/lyric.h"
#include "gs/multimusebuilder.h"
#include "mid/midivalidator.h"
#include "mid/trackbuilder.h"

/**
 * TrackBuilder that reads the gems of one skill level of a catch track into CatchTrackData.
 *
 * The RTTI includes the class name and records TrackBuilder as the base. The object is 0xb4
 * bytes. Each gem is a note at or above 96 that gives the lane and the skill level. The other
 * channel messages are the samples the gems play, each gem the samples from its tick to the next
 * gem. The text and lyric events are the lyrics.
 */
class CatchTrackBuilder : public TrackBuilder {
public:
    /** One gem of the skill level. */
    struct GemData {
        int mTick; /*!< The tick of the gem. */
        int mLane; /*!< The lane of the gem. */
    };

    /**
     * Construct a builder.
     *
     * @param pszName The track name the error messages give.
     * @param bValidate Check the events for authoring errors.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param nChannel The MIDI channel of the track.
     * @param nIntroTicks The length of the song intro, which must not have notes.
     * @param pData The data to fill.
     * @param pLyric The lyrics to fill.
     * @param nFilterEffects Nonzero to leave controllers 93 and 94 out of the samples.
     * @param nSkill The skill level whose gems the builder reads.
     * @ghidraAddress NTSC-U/C: 0x0014b390
     * @ghidraAddress PAL: 0x0014cd30
     */
    CatchTrackBuilder(const char *pszName,
                      bool bValidate,
                      ErrorHandler pfnError,
                      int nChannel,
                      int nIntroTicks,
                      CatchTrackData *pData,
                      Lyric *pLyric,
                      int nFilterEffects,
                      int nSkill);

    /**
     * Release the builder.
     *
     * @ghidraAddress NTSC-U/C: 0x0034a008
     * @ghidraAddress PAL: 0x003b7438
     */
    ~CatchTrackBuilder() override {
    }

    /**
     * Ignore the start of the track.
     *
     * @param nTrack The track index.
     * @ghidraAddress NTSC-U/C: 0x0034a0b8
     */
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    /**
     * Check the gems and the samples, and add each gem and its samples to the data.
     *
     * @ghidraAddress NTSC-U/C: 0x0014b930
     * @ghidraAddress PAL: 0x0014d2d0
     */
    void OnEndTrack() override;

    /**
     * Ignore the end of the file.
     *
     * @ghidraAddress NTSC-U/C: 0x0034a0c0
     */
    void OnAllDone() override {
    }

    /**
     * Record a gem, or collect a sample message.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0014b470
     * @ghidraAddress PAL: 0x0014ce10
     */
    void
    OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2) override;

    /**
     * Ignore a tempo change.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The new tempo.
     * @ghidraAddress NTSC-U/C: 0x0034a0c8
     */
    void OnTempo([[maybe_unused]] int nTick, [[maybe_unused]] int nMicrosecondsPerBeat) override {
    }

    /**
     * Add the text of a text or lyric event to the lyrics.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The meta event type.
     * @ghidraAddress NTSC-U/C: 0x0014b5b0
     * @ghidraAddress PAL: 0x0014cf50
     */
    void OnText(int nTick, const char *pszText, unsigned char nType) override;

    /**
     * Ignore a time signature.
     *
     * @param nTick The tick.
     * @param nNumerator The beats per bar.
     * @param nDenominator The beat unit.
     * @ghidraAddress NTSC-U/C: 0x0034a0d0
     */
    void OnTimeSignature([[maybe_unused]] int nTick,
                         [[maybe_unused]] int nNumerator,
                         [[maybe_unused]] int nDenominator) override {
    }

    /**
     * Report the lane and the skill level of a gem note.
     *
     * @param nNote The note, at least 96.
     * @param pnLane Receives the lane.
     * @param pnSkill Receives the skill level, also when the note is not a gem.
     * @return Whether the note is a gem.
     * @ghidraAddress NTSC-U/C: 0x0014b328
     * @ghidraAddress PAL: 0x0014ccc8
     */
    static bool DecodeGemNote(unsigned char nNote, int *pnLane, int *pnSkill);

    int mIntroTicks;            /*!< The length of the song intro. */
    CatchTrackData *mData;      /*!< The data to fill. */
    int mFilterEffects;         /*!< Nonzero to leave controllers 93 and 94 out of the samples. */
    Lyric *mLyric;              /*!< The lyrics to fill. */
    MidiValidator mValidator;   /*!< The checker of the channel messages. */
    int mSkill;                 /*!< The skill level whose gems the builder reads. */
    std::vector<GemData> mGems; /*!< The gems in tick order. */
    MultiMuseBuilder mSamples;  /*!< The sample messages. */
    int mLastNoteOnTick;        /*!< The tick of the last note-on message, or -1. */
    int mFirstSampleTick;       /*!< The tick of the first sample note, or -1. */

private:
    /**
     * Record a gem of the skill level the builder reads.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @return False when the message is not a gem note, so that it is a sample.
     * @ghidraAddress NTSC-U/C: 0x0014b5f0
     * @ghidraAddress PAL: 0x0014cf90
     */
    bool AddGem(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2);
};
