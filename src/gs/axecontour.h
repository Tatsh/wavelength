#pragma once

#include <vector>

#include "gs/multimuse.h"
#include "gs/muse.h"
#include "gs/muselooper.h"
#include "gs/mutablenotemuse.h"
#include "os/ptr.h"

/**
 * The notes AxeTrack plays on a guitar track, looped over the length of the track.
 *
 * The RTTI includes the class name and the nested AxeContour::AxeNoteCB and
 * AxeContour::ContourNote. The class is not polymorphic. AxeTrackContourBuilder fills one from the
 * guitar part of a song, and AxeHarmony moves its notes to fit a chord.
 */
class AxeContour {
public:
    /**
     * Receiver that records the note that started last.
     *
     * The RTTI includes the nested name and records Muse::NoteCB as the base. Every member is
     * inline.
     */
    class AxeNoteCB : public Muse::NoteCB {
    public:
        /**
         * Construct the receiver of a contour.
         *
         * @param pContour The contour.
         */
        explicit AxeNoteCB(AxeContour *pContour) : mContour(pContour) {
        }

        /**
         * Record the note.
         *
         * @param nNote The MIDI note number.
         * @param nDuration The length of the note. The body does not read it.
         * @ghidraAddress NTSC-U/C: 0x0034d3c0
         * @ghidraAddress PAL: 0x003ba770
         */
        void OnNote(unsigned char nNote, [[maybe_unused]] int nDuration) override {
            mContour->mCurrentNote = nNote;
        }

    private:
        AxeContour *mContour; /*!< The contour. */
    };

    /** One note of the contour, as written. */
    struct ContourNote {
        unsigned char mKey;     /*!< The note as written. */
        MutableNoteMuse *mMuse; /*!< The muse that plays it. */
    };

    /**
     * Construct an empty contour.
     *
     * @param nLength The ticks of one loop.
     * @param nChannel The MIDI channel of the notes.
     * @ghidraAddress NTSC-U/C: 0x00156bf0
     * @ghidraAddress PAL: 0x00158478
     */
    AxeContour(int nLength, unsigned char nChannel);

    /**
     * Release the notes.
     *
     * @ghidraAddress NTSC-U/C: 0x00156cc0
     * @ghidraAddress PAL: 0x00158548
     */
    ~AxeContour();

    /**
     * Report the ticks of one loop.
     *
     * @return The ticks.
     * @ghidraAddress NTSC-U/C: 0x00156da0
     * @ghidraAddress PAL: 0x00158628
     */
    int GetLength();

    /**
     * Report the MIDI channel of the notes.
     *
     * @return The channel.
     * @ghidraAddress NTSC-U/C: 0x00156dc0
     * @ghidraAddress PAL: 0x00158648
     */
    unsigned char GetChannel() const;

    /**
     * Add a note.
     *
     * @param nTick The tick of the note in the loop.
     * @param nKey The note.
     * @param nVelocity The MIDI velocity.
     * @param nDuration The length in ticks.
     * @ghidraAddress NTSC-U/C: 0x00156dc8
     * @ghidraAddress PAL: 0x00158650
     */
    void AddNote(int nTick, unsigned char nKey, unsigned char nVelocity, int nDuration);

    /**
     * Report the number of notes.
     *
     * @return The number.
     * @ghidraAddress NTSC-U/C: 0x00157060
     * @ghidraAddress PAL: 0x001588e8
     */
    int GetNumNotes() const;

    /**
     * Add a muse that is not a note, such as a controller change.
     *
     * @param nTick The tick of the muse in the loop.
     * @param pMuse The muse. The contour takes a reference.
     * @ghidraAddress NTSC-U/C: 0x00157078
     * @ghidraAddress PAL: 0x00158900
     */
    void AddMuse(int nTick, Muse *pMuse);

    /**
     * Report a note as written.
     *
     * @param nIndex The note.
     * @return The note number.
     * @ghidraAddress NTSC-U/C: 0x001570c0
     * @ghidraAddress PAL: 0x00158948
     */
    unsigned char GetNoteKey(int nIndex) const;

    /**
     * Change the note number a note plays.
     *
     * The note as written does not change.
     *
     * @param nIndex The note.
     * @param nKey The note number.
     * @ghidraAddress NTSC-U/C: 0x001570d8
     * @ghidraAddress PAL: 0x00158960
     */
    void SetNoteKey(int nIndex, unsigned char nKey);

    /**
     * Report the highest note as written.
     *
     * @return The note number.
     * @ghidraAddress NTSC-U/C: 0x00157108
     * @ghidraAddress PAL: 0x00158990
     */
    unsigned char GetHighestKey() const;

    /**
     * Report the lowest note as written.
     *
     * @return The note number.
     * @ghidraAddress NTSC-U/C: 0x00157110
     * @ghidraAddress PAL: 0x00158998
     */
    unsigned char GetLowestKey() const;

    /**
     * Play the loop on the song scheduler.
     *
     * @param nPosition The position in the loop.
     * @ghidraAddress NTSC-U/C: 0x00157118
     * @ghidraAddress PAL: 0x001589a0
     */
    void Play(int nPosition);

    /**
     * Stop the loop.
     *
     * @ghidraAddress NTSC-U/C: 0x00157140
     * @ghidraAddress PAL: 0x001589c8
     */
    void Stop();

    /**
     * Report whether the loop plays.
     *
     * @return Whether it plays.
     * @ghidraAddress NTSC-U/C: 0x00157168
     * @ghidraAddress PAL: 0x001589f0
     */
    bool IsPlaying() const;

    /**
     * Report the position in the loop.
     *
     * @return The ticks since the current loop started.
     * @ghidraAddress NTSC-U/C: 0x00157188
     * @ghidraAddress PAL: 0x00158a10
     */
    int GetPosition() const;

private:
    unsigned char mChannel;          /*!< The MIDI channel of the notes. */
    Ptr<MultiMuse> mMuse;            /*!< The notes and the other muses. */
    MuseLooper mLooper;              /*!< The looper of mMuse. */
    std::vector<ContourNote> mNotes; /*!< The notes, in the order they were added. */
    unsigned char mLowestKey;        /*!< The lowest note as written. */
    unsigned char mHighestKey;       /*!< The highest note as written. */
    int mCurrentNote;                /*!< The note that started last, or -1. */
    AxeNoteCB *mNoteCB;              /*!< The receiver of the notes. */
};
