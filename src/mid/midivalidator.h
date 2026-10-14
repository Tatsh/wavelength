#pragma once

#include <set>

#include "mid/trackbuilder.h"
#include "os/string.h"

/**
 * Checker of the channel messages of one MIDI track for authoring errors.
 *
 * The errors are messages on the wrong channel, duplicate notes, program changes, and controller
 * messages within one tick, and a note before the first program or bank change. The class is not
 * polymorphic and has no RTTI. The name is inferred. The object is 0x4c bytes.
 */
class MidiValidator {
public:
    /** The value of the channel and tick members before one is known. */
    static constexpr int kNone = -1;

    /**
     * Construct a checker.
     *
     * @param nTrack The number of the MIDI track the error messages give.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param nChannel The channel every message must use, or kNone for any.
     * @param bRequirePrograms Report a note that precedes the first program or bank change.
     * @ghidraAddress NTSC-U/C: 0x0027e1e8
     * @ghidraAddress PAL: 0x00287b00
     */
    MidiValidator(int nTrack,
                  TrackBuilder::ErrorHandler pfnError,
                  int nChannel,
                  bool bRequirePrograms);

    /**
     * Release the checker.
     *
     * @ghidraAddress NTSC-U/C: 0x0033cd90
     * @ghidraAddress PAL: 0x003aa2c8
     */
    ~MidiValidator();

    /**
     * Check one channel message.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0027e340
     * @ghidraAddress PAL: 0x00287c58
     */
    void Check(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2);

private:
    /**
     * Report an error at a tick through the error routine, or print it when there is none.
     *
     * @param nTick The tick.
     * @param pszMessage The message.
     * @ghidraAddress NTSC-U/C: 0x0027e628
     * @ghidraAddress PAL: 0x00287f40
     */
    void Error(int nTick, const char *pszMessage);

    int mTrack;                               /*!< The number of the MIDI track. */
    int mChannel;                             /*!< The required channel, or kNone. */
    TrackBuilder::ErrorHandler mErrorHandler; /*!< The routine that reports an error, or null. */
    int mRequirePrograms;                     /*!< Whether a note needs a program and a bank. */
    int mTick;                                /*!< The tick of the messages in the sets. */
    std::set<unsigned char> mNotes;           /*!< The notes started at mTick. */
    std::set<unsigned char> mControllers;     /*!< The controllers changed at mTick. */
    int mProgramChanged;                      /*!< Whether a program changed at mTick. */
    int mFirstNoteTick;                       /*!< The tick of the first note, or kNone. */
    int mFirstProgramTick;                    /*!< The tick of the first program change. */
    int mFirstBankTick;                       /*!< The tick of the first bank select. */
    int mOrderChecked;    /*!< Whether the order of the first events is known. */
    int mChannelReported; /*!< Whether a wrong channel was reported. */
};
