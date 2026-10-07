#pragma once

#include <cstddef>

#include "gs/notemusebase.h"
#include "os/mem.h"

/**
 * NoteMuseBase that can copy itself.
 *
 * The RTTI includes the class name and records NoteMuseBase as the base. The constructor has no
 * out-of-line copy. The allocations come from the pool of fixed-size blocks, billed to the tag
 * "NoteMuse".
 */
class NoteMuse : public NoteMuseBase {
public:
    /**
     * Allocate a note from the pool.
     *
     * @param nSize The object size.
     * @return The block.
     */
    static void *operator new(size_t nSize) {
        return PoolAlloc(static_cast<int>(nSize), sizeof(NoteMuse), "NoteMuse", 0);
    }

    /**
     * Return a note to the pool.
     *
     * @param pBlock The block.
     */
    static void operator delete(void *pBlock) {
        PoolFree(sizeof(NoteMuse), pBlock);
    }

    /**
     * Construct a note.
     *
     * @param nNote The MIDI note number.
     * @param nVelocity The MIDI velocity.
     * @param nDuration The length in ticks.
     * @param nChannel The MIDI channel.
     */
    NoteMuse(unsigned char nNote, unsigned char nVelocity, int nDuration, unsigned char nChannel)
        : NoteMuseBase(nNote, nVelocity, nDuration, nChannel) {
    }

    /**
     * Release the note.
     *
     * @ghidraAddress NTSC-U/C: 0x0034ea68
     * @ghidraAddress PAL: 0x003bbe90
     */
    ~NoteMuse() override {
    }
};
