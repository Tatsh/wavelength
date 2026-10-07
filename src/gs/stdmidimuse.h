#pragma once

#include <cstddef>

#include "gs/muse.h"
#include "os/mem.h"

/**
 * Muse that sends one standard MIDI message.
 *
 * The RTTI includes the class name and records Muse as the base. The allocations come from the
 * pool of fixed-size blocks, billed to the tag "StdMidiMuse". This header declares only the members
 * a ScratchTrack and the guitar track builder use.
 */
class StdMidiMuse : public Muse {
public:
    /**
     * Allocate a message from the pool.
     *
     * @param nSize The object size.
     * @return The block.
     */
    static void *operator new(size_t nSize) {
        return PoolAlloc(static_cast<int>(nSize), sizeof(StdMidiMuse), "StdMidiMuse", 0);
    }

    /**
     * Construct a message.
     *
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0015bf70
     * @ghidraAddress PAL: 0x0015d760
     */
    StdMidiMuse(unsigned char nStatus, unsigned char nData1, unsigned char nData2);

    /**
     * Release the message.
     *
     * @ghidraAddress NTSC-U/C: 0x0015bff8
     * @ghidraAddress PAL: 0x0015d7e8
     */
    ~StdMidiMuse() override;

    /**
     * Send the message on a scheduler.
     *
     * @param pScheduler The scheduler.
     * @ghidraAddress NTSC-U/C: 0x0015c070
     * @ghidraAddress PAL: 0x0015d860
     */
    void Play(Scheduler *pScheduler) override;

    /**
     * Send the message on a scheduler, moved by an offset.
     *
     * @param pScheduler The scheduler.
     * @param nOffset The offset in ticks.
     * @ghidraAddress NTSC-U/C: 0x0015c0c8
     * @ghidraAddress PAL: 0x0015d8b8
     */
    void PlayFrom(Scheduler *pScheduler, int nOffset) override;

    /**
     * Send the message on a scheduler when it falls in a window.
     *
     * @param pScheduler The scheduler.
     * @param nStart The first tick of the window.
     * @param nEnd The tick after the window.
     * @ghidraAddress NTSC-U/C: 0x0015c130
     * @ghidraAddress PAL: 0x0015d920
     */
    void PlayWindow(Scheduler *pScheduler, int nStart, int nEnd) override;

    /**
     * Withdraw the message if it is scheduled.
     *
     * @ghidraAddress NTSC-U/C: 0x0015c1a0
     * @ghidraAddress PAL: 0x0015d990
     */
    void Stop() override;

    /**
     * Allocate a program change message.
     *
     * @param nChannel The MIDI channel.
     * @param nProgram The program number.
     * @return The message, with no reference taken.
     * @ghidraAddress NTSC-U/C: 0x0015c260
     * @ghidraAddress PAL: 0x0015da50
     */
    static StdMidiMuse *NewProgramChange(unsigned char nChannel, unsigned char nProgram);
};
