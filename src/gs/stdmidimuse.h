#pragma once

#include <cstddef>

#include "gs/muse.h"
#include "os/command.h"
#include "os/mem.h"
#include "os/ptr.h"
#include "os/scheduler.h"

/**
 * Muse that sends one standard MIDI message.
 *
 * The RTTI includes the class name and records Muse as the base, and the nested
 * StdMidiMuse::StdMidiCommand. The allocations come from the pool of fixed-size blocks, billed to
 * the tag "StdMidiMuse".
 */
class StdMidiMuse : public Muse {
public:
    /**
     * Command that sends the message of a muse when the scheduler runs it.
     *
     * The RTTI includes the nested name and records Command as the base. The allocations come from
     * the pool of fixed-size blocks, billed to the tag "StdMidiMuse cmd".
     */
    class StdMidiCommand : public Command {
    public:
        /**
         * Construct the command of a muse.
         *
         * @param pMuse The muse.
         */
        explicit StdMidiCommand(StdMidiMuse *pMuse) : mMuse(pMuse), mScheduler(nullptr) {
        }

        /**
         * Allocate a command from the pool.
         *
         * @param nSize The object size.
         * @return The block.
         */
        static void *operator new(size_t nSize) {
            return PoolAlloc(static_cast<int>(nSize), sizeof(StdMidiCommand), "StdMidiMuse cmd", 0);
        }

        /**
         * Return a command to the pool.
         *
         * @param pBlock The block.
         */
        static void operator delete(void *pBlock) {
            PoolFree(sizeof(StdMidiCommand), pBlock);
        }

        /**
         * Send the message, timed within the pump of the scheduler, and forget the scheduler.
         *
         * @ghidraAddress NTSC-U/C: 0x0034f128
         * @ghidraAddress PAL: 0x003bc550
         */
        void Execute() override;

        /**
         * Withdraw the command from the scheduler that queued it.
         *
         * @ghidraAddress NTSC-U/C: 0x0034f1a0
         * @ghidraAddress PAL: 0x003bc5c8
         */
        void Cancel();

        StdMidiMuse *mMuse;    /*!< The muse. */
        Scheduler *mScheduler; /*!< The scheduler that queued the command, or null. */
    };

    /**
     * Construct a muse for a message.
     *
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0015bf70
     * @ghidraAddress PAL: 0x0015d760
     */
    StdMidiMuse(unsigned char nStatus, unsigned char nData1, unsigned char nData2);

    /**
     * Allocate a muse from the pool.
     *
     * @param nSize The object size.
     * @return The block.
     */
    static void *operator new(size_t nSize) {
        return PoolAlloc(static_cast<int>(nSize), sizeof(StdMidiMuse), "StdMidiMuse", 0);
    }

    /**
     * Return a muse to the pool.
     *
     * @param pBlock The block.
     */
    static void operator delete(void *pBlock) {
        PoolFree(sizeof(StdMidiMuse), pBlock);
    }

    /**
     * Withdraw the message and release the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015bff8
     * @ghidraAddress PAL: 0x0015d7e8
     */
    ~StdMidiMuse() override;

    /**
     * Send the message at once.
     *
     * @param pScheduler The scheduler that sends it.
     * @ghidraAddress NTSC-U/C: 0x0015c070
     * @ghidraAddress PAL: 0x0015d860
     */
    void Play(Scheduler *pScheduler) override;

    /**
     * Send the message after the ticks a negative offset delays it, or at once.
     *
     * @param pScheduler The scheduler that sends it.
     * @param nOffset The offset in ticks.
     * @ghidraAddress NTSC-U/C: 0x0015c0c8
     * @ghidraAddress PAL: 0x0015d8b8
     */
    void PlayFrom(Scheduler *pScheduler, int nOffset) override;

    /**
     * Send the message like PlayFrom() when the window is not empty.
     *
     * @param pScheduler The scheduler that sends it.
     * @param nStart The first tick of the window.
     * @param nEnd The tick after the window.
     * @ghidraAddress NTSC-U/C: 0x0015c130
     * @ghidraAddress PAL: 0x0015d920
     */
    void PlayWindow(Scheduler *pScheduler, int nStart, int nEnd) override;

    /**
     * Withdraw the queued message.
     *
     * @ghidraAddress NTSC-U/C: 0x0015c1a0
     * @ghidraAddress PAL: 0x0015d990
     */
    void Stop() override;

    /**
     * Report whether the message waits to be sent.
     *
     * @return Whether it waits.
     * @ghidraAddress NTSC-U/C: 0x0015c1e0
     * @ghidraAddress PAL: 0x0015d9d0
     */
    bool IsPlaying() override;

    /**
     * Report the length of the message.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0034f078
     */
    int GetLength() override {
        return 0;
    }

    /**
     * Produce a copy of the muse on the heap.
     *
     * @return The copy, with no reference taken.
     * @ghidraAddress NTSC-U/C: 0x0015c208
     * @ghidraAddress PAL: 0x0015d9f8
     */
    Muse *Clone() override;

    /**
     * Ignore a receiver of notes. The body is empty.
     *
     * @param pNoteCB The receiver.
     * @ghidraAddress NTSC-U/C: 0x0015c258
     * @ghidraAddress PAL: 0x0015da48
     */
    void SetNoteCB(NoteCB *pNoteCB) override;

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

    /**
     * Allocate a channel pressure message.
     *
     * @param nChannel The MIDI channel.
     * @param nPressure The pressure.
     * @return The message, with no reference taken.
     * @ghidraAddress NTSC-U/C: 0x0015c2b8
     * @ghidraAddress PAL: 0x0015daa8
     */
    static StdMidiMuse *NewChannelPressure(unsigned char nChannel, unsigned char nPressure);

    /**
     * Allocate a control change message.
     *
     * @param nChannel The MIDI channel.
     * @param nController The controller.
     * @param nValue The value.
     * @return The message, with no reference taken.
     * @ghidraAddress NTSC-U/C: 0x0015c310
     * @ghidraAddress PAL: 0x0015db00
     */
    static StdMidiMuse *
    NewControlChange(unsigned char nChannel, unsigned char nController, unsigned char nValue);

    unsigned char mStatus;    /*!< The status byte. */
    unsigned char mData1;     /*!< The first data byte. */
    unsigned char mData2;     /*!< The second data byte. */
    Ptr<StdMidiCommand> mCmd; /*!< The command that sends the message. */
};
