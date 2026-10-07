#pragma once

#include "app/attachment.h"
#include "os/scheduler.h"

/**
 * Piece of MIDI performance that a Scheduler plays.
 *
 * The RTTI includes the class name and records Attachment as the base. The vtable lists eight pure
 * members after the destructor.
 */
class Muse : public Attachment {
public:
    /**
     * Receiver of each note a muse starts.
     *
     * The RTTI includes the nested name. The class has no data member, and its destructor is
     * inline.
     */
    class NoteCB {
    public:
        /** Release the receiver. */
        virtual ~NoteCB() {
        }

        /**
         * Receive a note as it starts.
         *
         * @param nNote The MIDI note number.
         * @param nDuration The length of the note in ticks.
         */
        virtual void OnNote(unsigned char nNote, int nDuration) = 0;
    };

    /**
     * Release the muse.
     *
     * @ghidraAddress NTSC-U/C: 0x0034e3a8
     * @ghidraAddress PAL: 0x003bb7d0
     */
    ~Muse() override {
    }

    /**
     * Play from the start.
     *
     * @param pScheduler The scheduler that plays the events.
     */
    virtual void Play(Scheduler *pScheduler) = 0;

    /**
     * Play with every event moved by an offset.
     *
     * @param pScheduler The scheduler that plays the events.
     * @param nOffset The offset in ticks.
     */
    virtual void PlayFrom(Scheduler *pScheduler, int nOffset) = 0;

    /**
     * Play the events that fall in a window.
     *
     * @param pScheduler The scheduler that plays the events.
     * @param nStart The first tick of the window.
     * @param nEnd The tick after the window.
     */
    virtual void PlayWindow(Scheduler *pScheduler, int nStart, int nEnd) = 0;

    /** Withdraw every event still queued and silence the notes that sound. */
    virtual void Stop() = 0;

    /**
     * Report whether a note of the muse still sounds or waits to sound.
     *
     * @return Whether the muse plays.
     */
    virtual bool IsPlaying() = 0;

    /**
     * Report the length of the muse.
     *
     * @return The length in ticks.
     */
    virtual int GetLength() = 0;

    /**
     * Produce a copy of the muse on the heap.
     *
     * @return The copy, with no reference taken.
     */
    virtual Muse *Clone() = 0;

    /**
     * Set the receiver of the notes the muse starts.
     *
     * @param pNoteCB The receiver, or null.
     */
    virtual void SetNoteCB(NoteCB *pNoteCB) = 0;
};
