#pragma once

#include "gs/muse.h"

/**
 * Muse that plays one note.
 *
 * The RTTI includes the class name and records Muse as the base. This header declares only the
 * members a ScratchTrack uses.
 */
class NoteMuseBase : public Muse {
public:
    /**
     * Construct a note.
     *
     * @param nNote The MIDI note number.
     * @param nVelocity The MIDI velocity.
     * @param nDuration The length in ticks.
     * @param nChannel The MIDI channel.
     * @ghidraAddress NTSC-U/C: 0x0015b778
     * @ghidraAddress PAL: 0x0015cf68
     */
    NoteMuseBase(unsigned char nNote,
                 unsigned char nVelocity,
                 int nDuration,
                 unsigned char nChannel);

    /**
     * Release the note.
     *
     * @ghidraAddress NTSC-U/C: 0x0015b810
     * @ghidraAddress PAL: 0x0015d000
     */
    ~NoteMuseBase() override;

    /**
     * Play from the start.
     *
     * @param pScheduler The scheduler that plays the events.
     * @ghidraAddress NTSC-U/C: 0x0015b8c0
     * @ghidraAddress PAL: 0x0015d0b0
     */
    void Play(Scheduler *pScheduler) override;

    /**
     * Play with the note moved by an offset.
     *
     * @param pScheduler The scheduler that plays the events.
     * @param nOffset The offset in ticks.
     * @ghidraAddress NTSC-U/C: 0x0015b938
     * @ghidraAddress PAL: 0x0015d128
     */
    void PlayFrom(Scheduler *pScheduler, int nOffset) override;

    /**
     * Play the note when it falls in a window.
     *
     * @param pScheduler The scheduler that plays the events.
     * @param nStart The first tick of the window.
     * @param nEnd The tick after the window.
     * @ghidraAddress NTSC-U/C: 0x0015b9d8
     * @ghidraAddress PAL: 0x0015d1c8
     */
    void PlayWindow(Scheduler *pScheduler, int nStart, int nEnd) override;

    /**
     * Withdraw the queued note and silence it.
     *
     * @ghidraAddress NTSC-U/C: 0x0015ba80
     * @ghidraAddress PAL: 0x0015d270
     */
    void Stop() override;
};
