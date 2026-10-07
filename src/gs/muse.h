#pragma once

#include "app/attachment.h"
#include "os/scheduler.h"

/**
 * Piece of MIDI performance that a Scheduler plays.
 *
 * The RTTI includes the class name and records Attachment as the base. The vtable lists eight pure
 * members after the destructor. This header declares the first four.
 */
class Muse : public Attachment {
public:
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
};
