#pragma once

#include <cstddef>

#include "gs/muse.h"
#include "os/command.h"
#include "os/mem.h"
#include "os/ptr.h"
#include "os/scheduler.h"

/**
 * Muse that plays one note.
 *
 * The RTTI includes the class name and records Muse as the base, and the nested
 * NoteMuseBase::NoteCommand. Clone() is pure in this class. Playing posts the command twice, once
 * for the note on and once for the note off after the length of the note.
 */
class NoteMuseBase : public Muse {
public:
    /**
     * Command that sends the note on and, the second time it runs, the note off.
     *
     * The RTTI includes the nested name and records Command as the base. The allocations come from
     * the pool of fixed-size blocks, billed to the tag "NoteMuse cmd".
     */
    class NoteCommand : public Command {
    public:
        /**
         * Construct the command of a note.
         *
         * @param pMuse The note.
         */
        explicit NoteCommand(NoteMuseBase *pMuse) : mMuse(pMuse), mScheduler(nullptr) {
        }

        /**
         * Allocate a command from the pool.
         *
         * @param nSize The object size.
         * @return The block.
         */
        static void *operator new(size_t nSize) {
            return PoolAlloc(static_cast<int>(nSize), sizeof(NoteCommand), "NoteMuse cmd", 0);
        }

        /**
         * Return a command to the pool.
         *
         * @param pBlock The block.
         */
        static void operator delete(void *pBlock) {
            PoolFree(sizeof(NoteCommand), pBlock);
        }

        /**
         * Send the note on, or the note off when the note sounds.
         *
         * @ghidraAddress NTSC-U/C: 0x0034ec20
         * @ghidraAddress PAL: 0x003bc048
         */
        void Execute() override;

        /**
         * Silence the note when it sounds and withdraw the command.
         *
         * @ghidraAddress NTSC-U/C: 0x0034ec70
         * @ghidraAddress PAL: 0x003bc098
         */
        void Cancel();

        /**
         * Send the note off.
         *
         * @ghidraAddress NTSC-U/C: 0x0034ecc0
         * @ghidraAddress PAL: 0x003bc0e8
         */
        void NoteOff();

        /**
         * Send the note on and report it to the receiver of the note.
         *
         * @ghidraAddress NTSC-U/C: 0x0034ed40
         * @ghidraAddress PAL: 0x003bc168
         */
        void NoteOn();

        NoteMuseBase *mMuse;   /*!< The note. */
        Scheduler *mScheduler; /*!< The scheduler that queued the command, or null. */
    };

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
     * Silence the note and release the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015b810
     * @ghidraAddress PAL: 0x0015d000
     */
    ~NoteMuseBase() override;

    /**
     * Play the note at once.
     *
     * @param pScheduler The scheduler that plays the note.
     * @ghidraAddress NTSC-U/C: 0x0015b8c0
     * @ghidraAddress PAL: 0x0015d0b0
     */
    void Play(Scheduler *pScheduler) override;

    /**
     * Play the note after the ticks a negative offset delays it. A positive offset plays nothing.
     *
     * @param pScheduler The scheduler that plays the note.
     * @param nOffset The offset in ticks.
     * @ghidraAddress NTSC-U/C: 0x0015b938
     * @ghidraAddress PAL: 0x0015d128
     */
    void PlayFrom(Scheduler *pScheduler, int nOffset) override;

    /**
     * Play the note like PlayFrom() when the window is not empty.
     *
     * @param pScheduler The scheduler that plays the note.
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

    /**
     * Report whether the note waits to sound or sounds.
     *
     * @return Whether the note plays.
     * @ghidraAddress NTSC-U/C: 0x0015bad8
     * @ghidraAddress PAL: 0x0015d2c8
     */
    bool IsPlaying() override;

    /**
     * Report the length of the note.
     *
     * @return The length in ticks.
     * @ghidraAddress NTSC-U/C: 0x0034ea60
     * @ghidraAddress PAL: 0x003bbe88
     */
    int GetLength() override {
        return mDuration;
    }

    /**
     * Set the receiver of the note.
     *
     * @param pNoteCB The receiver, or null.
     * @ghidraAddress NTSC-U/C: 0x0015bb00
     * @ghidraAddress PAL: 0x0015d2f0
     */
    void SetNoteCB(NoteCB *pNoteCB) override;

protected:
    unsigned char mNote;     /*!< The MIDI note number. */
    unsigned char mVelocity; /*!< The MIDI velocity. */
    unsigned char mChannel;  /*!< The MIDI channel. */
    unsigned char mSounding; /*!< Whether the note on was sent and the note off was not. */
    int mDuration;           /*!< The length in ticks. */
    Ptr<NoteCommand> mCmd;   /*!< The command that sends the note. */
    NoteCB *mNoteCB;         /*!< The receiver of the note, or null. */
};
