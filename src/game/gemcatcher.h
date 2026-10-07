#pragma once

#include "game/catchtrackstate.h"
#include "game/gemcursor.h"
#include "os/command.h"
#include "os/commandid.h"
#include "os/ptr.h"

/**
 * Judge of the presses on the lanes of one catch track.
 *
 * The class is not polymorphic and has no RTTI. The name comes from the RTTI of its nested
 * classes. Each lane has a CatchCheckCmd that waits on the next gem of the lane.
 */
class GemCatcher {
public:
    /** The number of lanes of a catch track. */
    static constexpr int kNumLanes = 3;

    /**
     * Object told of each judged gem and press.
     *
     * The RTTI includes the class name.
     */
    class Receiver {
    public:
        /**
         * Release the receiver.
         *
         * @ghidraAddress NTSC-U/C: 0x00335720
         * @ghidraAddress PAL: 0x003a2cd0
         */
        virtual ~Receiver() {
        }

        /**
         * Handle a caught gem.
         *
         * @param nTick The tick of the press.
         * @param cursor The gem.
         */
        virtual void OnHit(int nTick, const GemCursor &cursor) = 0;

        /**
         * Handle a gem that passed without a press.
         *
         * @param nTick The tick the gem was judged at.
         * @param cursor The gem.
         */
        virtual void OnPass(int nTick, const GemCursor &cursor) = 0;

        /**
         * Handle a press that caught no gem.
         *
         * @param nTick The tick of the press.
         * @param nLane The lane.
         */
        virtual void OnMiss(int nTick, int nLane) = 0;
    };

    /**
     * Command that judges the presses on one lane and passes each gem the player lets go by.
     *
     * The RTTI includes the class name. The scheduler runs the command once the time of slop
     * after the next gem of the lane has elapsed.
     */
    class CatchCheckCmd : public Command {
    public:
        /**
         * Construct the command for a lane.
         *
         * @param pReceiver The receiver of the judgements.
         * @param pState The state of the track.
         * @param pMsPerTick The length of a tick in milliseconds.
         * @param nTrack The track.
         * @param nLane The lane.
         * @param nSlopMs The largest time between a press and a gem that catches the gem.
         * @param nTicksPerBar The length of a bar in ticks.
         * @ghidraAddress NTSC-U/C: 0x001169a0
         * @ghidraAddress PAL: 0x00118138
         */
        CatchCheckCmd(Receiver *pReceiver, CatchTrackState *pState, const float *pMsPerTick,
                      int nTrack, int nLane, int nSlopMs, int nTicksPerBar);

        /**
         * Move to the next gem of the lane from the current tick and queue the command for it.
         *
         * @ghidraAddress NTSC-U/C: 0x00116a08
         * @ghidraAddress PAL: 0x001181a0
         */
        void Schedule();

        /**
         * Pass the gem when its bar is open, and wait on the next gem.
         *
         * @ghidraAddress NTSC-U/C: 0x00116ab8
         * @ghidraAddress PAL: 0x00118250
         */
        void Execute() override;

        /**
         * Withdraw the queued command and queue it again from the current tick.
         *
         * @ghidraAddress NTSC-U/C: 0x00116b58
         * @ghidraAddress PAL: 0x001182f0
         */
        void Restart();

        /**
         * Judge a press at the current tick.
         *
         * @ghidraAddress NTSC-U/C: 0x00116ba0
         * @ghidraAddress PAL: 0x00118338
         */
        void Catch();

        /**
         * Move to the next gem of the lane and queue the command for it.
         *
         * @ghidraAddress NTSC-U/C: 0x00116e00
         * @ghidraAddress PAL: 0x00118598
         */
        void ScheduleNext();

        /**
         * Report the cursor whose gem is nearer a tick.
         *
         * @param first The first cursor.
         * @param second The second cursor.
         * @param nTick The tick.
         * @return The valid cursor when only one is valid, otherwise the nearer one, and the
         *         second on a tie.
         * @ghidraAddress NTSC-U/C: 0x00116eb8
         * @ghidraAddress PAL: 0x00118650
         */
        GemCursor Nearest(const GemCursor &first, const GemCursor &second, int nTick) const;

        /**
         * Report a caught gem.
         *
         * @param nTick The tick of the press.
         * @param cursor The gem.
         * @param fErrorMs The time from the press to the gem in milliseconds.
         * @ghidraAddress NTSC-U/C: 0x00116f78
         * @ghidraAddress PAL: 0x00118710
         */
        void Hit(int nTick, const GemCursor &cursor, float fErrorMs);

        /**
         * Report a gem that passed without a press.
         *
         * @param nTick The tick the gem was judged at.
         * @param cursor The gem.
         * @ghidraAddress NTSC-U/C: 0x00117008
         * @ghidraAddress PAL: 0x001187a0
         */
        void Pass(int nTick, const GemCursor &cursor);

        /**
         * Report a press too far from the nearest gem.
         *
         * @param nTick The tick of the press.
         * @param cursor The nearest gem.
         * @param fErrorMs The time from the press to the gem in milliseconds.
         * @ghidraAddress NTSC-U/C: 0x00117080
         * @ghidraAddress PAL: 0x00118818
         */
        void Miss(int nTick, const GemCursor &cursor, float fErrorMs);

        /**
         * Report a press with no gem near it.
         *
         * @param nTick The tick of the press.
         * @ghidraAddress NTSC-U/C: 0x00117100
         * @ghidraAddress PAL: 0x00118898
         */
        void Miss(int nTick);

    private:
        Receiver *mReceiver;      /*!< The receiver of the judgements. */
        CatchTrackState *mState;  /*!< The state of the track. */
        const float *mMsPerTick;  /*!< The length of a tick in milliseconds. */
        int mTrack;               /*!< The track. */
        int mLane;                /*!< The lane. */
        int mSlopMs;              /*!< The largest time between a press and a caught gem. */
        int mTicksPerBar;         /*!< The length of a bar in ticks. */
        GemCursor mCursor;        /*!< The next gem of the lane. */
        CommandId mId;            /*!< The tag of the queued command. */
    };

    /**
     * Construct the commands of every lane.
     *
     * @param pReceiver The receiver of the judgements.
     * @param pState The state of the track.
     * @param pMsPerTick The length of a tick in milliseconds.
     * @param nTrack The track.
     * @param nTicksPerBar The length of a bar in ticks.
     * @ghidraAddress NTSC-U/C: 0x00117160
     * @ghidraAddress PAL: 0x001188f8
     */
    GemCatcher(Receiver *pReceiver, CatchTrackState *pState, const float *pMsPerTick, int nTrack,
               int nTicksPerBar);

    /**
     * Withdraw the queued commands.
     *
     * @ghidraAddress NTSC-U/C: 0x00117278
     * @ghidraAddress PAL: 0x00118a10
     */
    ~GemCatcher();

    /**
     * Queue the command of every lane from the current tick.
     *
     * @ghidraAddress NTSC-U/C: 0x001172f8
     * @ghidraAddress PAL: 0x00118a90
     */
    void Start();

    /**
     * Withdraw the command of every lane.
     *
     * @ghidraAddress NTSC-U/C: 0x00117350
     * @ghidraAddress PAL: 0x00118ae8
     */
    void Stop();

    /**
     * Queue the command of every lane again from the current tick.
     *
     * @ghidraAddress NTSC-U/C: 0x001173b0
     * @ghidraAddress PAL: 0x00118b48
     */
    void Reset();

    /**
     * Judge a press on a lane at the current tick.
     *
     * @param nLane The lane.
     * @ghidraAddress NTSC-U/C: 0x00117400
     * @ghidraAddress PAL: 0x00118b98
     */
    void Catch(int nLane);

private:
    Ptr<CatchCheckCmd> mChecks[kNumLanes]; /*!< The command of each lane. */
};
