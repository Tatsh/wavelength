#pragma once

#include <cstddef>

#include "gs/notemusebase.h"
#include "os/mem.h"

/**
 * NoteMuseBase whose note can change after it is built.
 *
 * The RTTI includes the class name and records NoteMuseBase as the base. The allocations come
 * from the pool of fixed-size blocks, billed to the tag "MuteNoteMuse". AxeContour builds one for
 * each note of a guitar track and moves its note to fit the harmony.
 */
class MutableNoteMuse : public NoteMuseBase {
public:
    /**
     * Construct a note.
     *
     * @param nNote The MIDI note number.
     * @param nVelocity The MIDI velocity.
     * @param nDuration The length in ticks.
     * @param nChannel The MIDI channel.
     * @ghidraAddress NTSC-U/C: 0x0015bb78
     * @ghidraAddress PAL: 0x0015d368
     */
    MutableNoteMuse(unsigned char nNote,
                    unsigned char nVelocity,
                    int nDuration,
                    unsigned char nChannel);

    /**
     * Allocate a note from the pool.
     *
     * @param nSize The object size.
     * @return The block.
     */
    static void *operator new(size_t nSize) {
        return PoolAlloc(static_cast<int>(nSize), sizeof(MutableNoteMuse), "MuteNoteMuse", 0);
    }

    /**
     * Return a note to the pool.
     *
     * @param pBlock The block.
     */
    static void operator delete(void *pBlock) {
        PoolFree(sizeof(MutableNoteMuse), pBlock);
    }

    /**
     * Release the note.
     *
     * @ghidraAddress NTSC-U/C: 0x0015bbc8
     * @ghidraAddress PAL: 0x0015d3b8
     */
    ~MutableNoteMuse() override;

    /**
     * Play from the start.
     *
     * @param pScheduler The scheduler that plays the events.
     * @ghidraAddress NTSC-U/C: 0x0015bc20
     * @ghidraAddress PAL: 0x0015d410
     */
    void Play(Scheduler *pScheduler) override;

    /**
     * Play with the note moved by an offset.
     *
     * @param pScheduler The scheduler that plays the events.
     * @param nOffset The offset in ticks.
     * @ghidraAddress NTSC-U/C: 0x0015bc48
     * @ghidraAddress PAL: 0x0015d438
     */
    void PlayFrom(Scheduler *pScheduler, int nOffset) override;

    /**
     * Play the note when it falls in a window.
     *
     * @param pScheduler The scheduler that plays the events.
     * @param nStart The first tick of the window.
     * @param nEnd The tick after the window.
     * @ghidraAddress NTSC-U/C: 0x0015bc70
     * @ghidraAddress PAL: 0x0015d460
     */
    void PlayWindow(Scheduler *pScheduler, int nStart, int nEnd) override;

    /**
     * Produce a copy of the note on the heap.
     *
     * @return The copy, with no reference taken.
     * @ghidraAddress NTSC-U/C: 0x0015bc98
     * @ghidraAddress PAL: 0x0015d488
     */
    Muse *Clone() override;

    /**
     * Set the note the muse plays from now on.
     *
     * @param nNote The MIDI note number.
     * @ghidraAddress NTSC-U/C: 0x0015bd20
     * @ghidraAddress PAL: 0x0015d510
     */
    void SetNote(unsigned char nNote);

private:
    unsigned char mNextNote; /*!< The note the next play starts. */
};
