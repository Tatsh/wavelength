#pragma once

#include <cstddef>
#include <list>

#include "gs/muse.h"
#include "os/command.h"
#include "os/mem.h"
#include "os/ptr.h"
#include "os/scheduler.h"

/**
 * Muse that plays other muses, each at a tick of its own.
 *
 * The RTTI includes the class name and records Muse as the base, and the nested
 * MultiMuse::MultiMuseCmd and MultiMuse::TimedMuse. The allocations come from the pool of
 * fixed-size blocks, billed to the tag "MultiMuse". The muses are kept in tick order. While the
 * sequence plays, a command starts the muses of each tick in turn.
 */
class MultiMuse : public Muse {
public:
    /** One muse of the sequence and its tick. */
    struct TimedMuse {
        Ptr<Muse> mMuse; /*!< The muse. */
        int mTick;       /*!< The tick of the muse. */
    };

    /**
     * Command that starts the muses of the next tick of a sequence.
     *
     * The RTTI includes the nested name and records Command as the base. The allocations come
     * from the pool of fixed-size blocks, billed to the tag "MultiMuse cmd".
     */
    class MultiMuseCmd : public Command {
    public:
        /**
         * Construct the command of a sequence.
         *
         * @param pMulti The sequence.
         */
        explicit MultiMuseCmd(MultiMuse *pMulti) : mMulti(pMulti) {
        }

        /**
         * Allocate a command from the pool.
         *
         * @param nSize The object size.
         * @return The block.
         */
        static void *operator new(size_t nSize) {
            return PoolAlloc(static_cast<int>(nSize), sizeof(MultiMuseCmd), "MultiMuse cmd", 0);
        }

        /**
         * Return a command to the pool.
         *
         * @param pBlock The block.
         */
        static void operator delete(void *pBlock) {
            PoolFree(sizeof(MultiMuseCmd), pBlock);
        }

        /**
         * Start the muses of the next tick.
         *
         * @ghidraAddress NTSC-U/C: 0x0034e510
         * @ghidraAddress PAL: 0x003bb938
         */
        void Execute() override;

    private:
        MultiMuse *mMulti; /*!< The sequence. */
    };

    /**
     * Construct an empty sequence.
     *
     * @param nStopPrevious Non-zero to stop the muses of a tick when the next tick starts.
     * @ghidraAddress NTSC-U/C: 0x0015a008
     * @ghidraAddress PAL: 0x0015b7f8
     */
    explicit MultiMuse(int nStopPrevious);

    /**
     * Allocate a sequence from the pool.
     *
     * @param nSize The object size.
     * @return The block.
     */
    static void *operator new(size_t nSize) {
        return PoolAlloc(static_cast<int>(nSize), sizeof(MultiMuse), "MultiMuse", 0);
    }

    /**
     * Return a sequence to the pool.
     *
     * @param pBlock The block.
     */
    static void operator delete(void *pBlock) {
        PoolFree(sizeof(MultiMuse), pBlock);
    }

    /**
     * Stop the sequence and release the muses.
     *
     * @ghidraAddress NTSC-U/C: 0x0015a100
     * @ghidraAddress PAL: 0x0015b8f0
     */
    ~MultiMuse() override;

    /**
     * Play from the start.
     *
     * @param pScheduler The scheduler that plays the muses.
     * @ghidraAddress NTSC-U/C: 0x0015a198
     * @ghidraAddress PAL: 0x0015b988
     */
    void Play(Scheduler *pScheduler) override;

    /**
     * Play from a tick of the sequence to its end.
     *
     * @param pScheduler The scheduler that plays the muses.
     * @param nOffset The tick to start from.
     * @ghidraAddress NTSC-U/C: 0x0015a1c8
     * @ghidraAddress PAL: 0x0015b9b8
     */
    void PlayFrom(Scheduler *pScheduler, int nOffset) override;

    /**
     * Play the part of the sequence in a window of ticks.
     *
     * The muses that start at or before the window start at once, from the matching place.
     *
     * @param pScheduler The scheduler that plays the muses.
     * @param nStart The first tick of the window.
     * @param nEnd The tick after the window.
     * @ghidraAddress NTSC-U/C: 0x0015a1f8
     * @ghidraAddress PAL: 0x0015b9e8
     */
    void PlayWindow(Scheduler *pScheduler, int nStart, int nEnd) override;

    /**
     * Report whether the sequence plays.
     *
     * @return Whether muses wait to start or a started muse plays.
     * @ghidraAddress NTSC-U/C: 0x0015a378
     * @ghidraAddress PAL: 0x0015bb68
     */
    bool IsPlaying() override;

    /**
     * Report the length of the sequence.
     *
     * @return The tick at which the last muse ends.
     * @ghidraAddress NTSC-U/C: 0x0015a3f8
     */
    int GetLength() override;

    /**
     * Withdraw the queued command and stop every muse.
     *
     * @ghidraAddress NTSC-U/C: 0x0015a560
     * @ghidraAddress PAL: 0x0015bd50
     */
    void Stop() override;

    /**
     * Produce a copy of the sequence and of each muse on the heap.
     *
     * @return The copy, with no reference taken.
     * @ghidraAddress NTSC-U/C: 0x0015a618
     * @ghidraAddress PAL: 0x0015be08
     */
    Muse *Clone() override;

    /**
     * Set the receiver of the notes of every muse of the sequence.
     *
     * @param pNoteCB The receiver, or null.
     * @ghidraAddress NTSC-U/C: 0x0015a6d8
     * @ghidraAddress PAL: 0x0015bec8
     */
    void SetNoteCB(NoteCB *pNoteCB) override;

    /**
     * Insert a muse at a tick, after the muses of the same tick.
     *
     * A muse added to a playing sequence ahead of the next muse to start is scheduled.
     *
     * @param pMuse The muse. The sequence takes a reference.
     * @param nTick The tick.
     * @ghidraAddress NTSC-U/C: 0x0015a760
     * @ghidraAddress PAL: 0x0015bf50
     */
    void Add(Muse *pMuse, int nTick);

private:
    /**
     * Start the muses of the next tick and schedule the tick after.
     *
     * @ghidraAddress NTSC-U/C: 0x0015a400
     * @ghidraAddress PAL: 0x0015bbf0
     */
    void Advance();

    std::list<TimedMuse> mMuses;                 /*!< The muses in tick order. */
    std::list<TimedMuse>::iterator mNext;        /*!< The next muse to start. */
    std::list<TimedMuse>::iterator mFirstActive; /*!< The first muse that may still play. */
    int mStopPrevious;                           /*!< Whether a tick stops the muses before it. */
    Scheduler *mScheduler;                       /*!< The scheduler that plays, or null. */
    Ptr<Command> mCmd;                           /*!< The MultiMuseCmd. */
    int mLength;                                 /*!< The tick at which the last muse ends. */
    int mOffset;                                 /*!< The scheduler tick of tick 0 of the window. */
    int mEnd;                                    /*!< The scheduler tick the window ends at. */
};
