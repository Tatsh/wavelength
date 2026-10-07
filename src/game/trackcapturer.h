#pragma once

#include "game/catchtrackstate.h"
#include "game/gemcatcher.h"
#include "game/gemcursor.h"
#include "game/netfaker.h"
#include "game/player.h"
#include "game/playmap.h"
#include "game/trackreactor.h"
#include "gs/muse.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Rules of capturing the phrases of a catch track.
 *
 * The RTTI records TrackReactor as the one base, and the nested TrackCapturer::Receiver,
 * TrackCapturer::CatchReceiver, TrackCapturer::SetCaptureStartCmd, and TrackCapturer::Faker.
 * CatchTrack has one. A run is the stretch of active bars the player must catch next, at most
 * GameConfig::mBarCaptureThreshold bars long. Catching every gem of a run captures it, and a miss
 * inside the run loses it.
 */
class TrackCapturer : public TrackReactor {
public:
    /**
     * Interface told when a run is started, lost, or captured.
     *
     * The RTTI includes the nested name. CatchTrack::CaptureReceiver implements it.
     */
    class Receiver {
    public:
        /** Release the receiver. */
        virtual ~Receiver() {
        }

        /**
         * Handle a captured run.
         *
         * @param cursor The last gem of the run.
         */
        virtual void OnCapture(const GemCursor &cursor) = 0;

        /** Handle a lost run. */
        virtual void OnRunLost() = 0;

        /**
         * Handle the first gem caught in a run.
         *
         * @param cursor The gem.
         * @param nEndBar The bar after the run.
         */
        virtual void OnRunStart(const GemCursor &cursor, int nEndBar) = 0;
    };

    /**
     * Receiver of the judgements of the gem catcher, passed on to the capturer.
     *
     * The RTTI includes the nested name and records GemCatcher::Receiver as the base. Every member
     * is inline.
     */
    class CatchReceiver : public GemCatcher::Receiver {
    public:
        /**
         * Construct the receiver of a capturer.
         *
         * @param pCapturer The capturer.
         */
        explicit CatchReceiver(TrackCapturer *pCapturer) : mCapturer(pCapturer) {
        }

        /**
         * Pass a caught gem on as a local hit.
         *
         * @param nTick The tick of the press.
         * @param cursor The gem.
         * @ghidraAddress NTSC-U/C: 0x0034d188
         * @ghidraAddress PAL: 0x003ba5b8
         */
        void OnHit(int nTick, const GemCursor &cursor) override {
            mCapturer->HitGem(nTick, cursor, false);
        }

        /**
         * Pass a gem that passed without a press on as a local miss.
         *
         * @param nTick The tick the gem was judged at.
         * @param cursor The gem.
         * @ghidraAddress NTSC-U/C: 0x0034d1b8
         * @ghidraAddress PAL: 0x003ba5e8
         */
        void OnPass(int nTick, const GemCursor &cursor) override {
            mCapturer->MissGem(nTick, cursor, false);
        }

        /**
         * Pass a press that caught no gem on.
         *
         * @param nTick The tick of the press.
         * @param nLane The lane.
         * @ghidraAddress NTSC-U/C: 0x0034d1e8
         * @ghidraAddress PAL: 0x003ba618
         */
        void OnMiss(int nTick, int nLane) override {
            mCapturer->MissPress(nTick, nLane);
        }

    private:
        TrackCapturer *mCapturer; /*!< The capturer. */
    };

    /**
     * Command that looks for the next run from a bar.
     *
     * The RTTI includes the nested name and records Command as the base. UpdateRun() schedules it
     * one bar ahead while no run lies near.
     */
    class SetCaptureStartCmd : public Command {
    public:
        /**
         * Construct the command of a capturer.
         *
         * @param pCapturer The capturer.
         */
        explicit SetCaptureStartCmd(TrackCapturer *pCapturer) : mBar(0), mCapturer(pCapturer) {
        }

        /**
         * Look for the next run from mBar.
         *
         * @ghidraAddress NTSC-U/C: 0x0034d0e8
         * @ghidraAddress PAL: 0x003ba518
         */
        void Execute() override {
            mCapturer->UpdateRun(mBar);
        }

        int mBar;                 /*!< The bar to look from. */
        TrackCapturer *mCapturer; /*!< The capturer. */
    };

    /**
     * Stand-in that presses each gem of an active bar when the autopilot is on.
     *
     * The RTTI records only the command the class schedules. The class is not polymorphic, and
     * its constructor is inline.
     */
    class Faker {
    public:
        /**
         * Construct a stand-in at the first gem of the track.
         *
         * @param pCapturer The capturer.
         */
        explicit Faker(TrackCapturer *pCapturer);

        /**
         * Move to the first gem at or after the current tick and schedule it.
         *
         * Inline. TrackCapturer expands it.
         */
        void Start();

        /**
         * Withdraw the scheduled gem.
         *
         * @ghidraAddress NTSC-U/C: 0x0034cf50
         * @ghidraAddress PAL: 0x003ba380
         */
        void Stop();

        /**
         * Press the gem of the current tick when the autopilot is on and schedule the next gem.
         *
         * @ghidraAddress NTSC-U/C: 0x0034cf88
         * @ghidraAddress PAL: 0x003ba3b8
         */
        void Update();

        TrackCapturer *mCapturer; /*!< The capturer. */
        GemCursor mCursor;        /*!< The next gem. */
        Ptr<Command> mUpdateCmd;  /*!< The command that calls Update(). */
        int mEnabled;             /*!< Whether the autopilot presses the gems. */
    };

    /**
     * Construct the rules of a track and look for the first run.
     *
     * @param pReceiver The receiver of the runs.
     * @param pState The state of the bars of the track.
     * @param pMsPerTick The length of a tick in milliseconds.
     * @param pPlayMap The play map of the song.
     * @param nReserved Stored and not read here.
     * @param nUnused Passed by the caller. The body does not read it.
     * @param nTrack The track.
     * @param nTicksPerBar The song ticks in one bar.
     * @param nStopPreviousNote Non-zero to stop the note of the previous gem at each hit.
     * @ghidraAddress NTSC-U/C: 0x00155628
     * @ghidraAddress PAL: 0x00156e90
     */
    TrackCapturer(Receiver *pReceiver,
                  CatchTrackState *pState,
                  const float *pMsPerTick,
                  PlayMap *pPlayMap,
                  int nReserved,
                  int nUnused,
                  int nTrack,
                  int nTicksPerBar,
                  int nStopPreviousNote);

    /**
     * Stop and release the helpers.
     *
     * @ghidraAddress NTSC-U/C: 0x001558f8
     * @ghidraAddress PAL: 0x00157160
     */
    ~TrackCapturer() override;

    /**
     * Look for the run from the current bar and start judging the presses.
     *
     * @ghidraAddress NTSC-U/C: 0x001559e0
     * @ghidraAddress PAL: 0x00157248
     */
    void Start();

    /**
     * Stop judging the presses and hide the run.
     *
     * @ghidraAddress NTSC-U/C: 0x00155af8
     * @ghidraAddress PAL: 0x00157360
     */
    void Stop();

    /**
     * Stop and start again when started.
     *
     * @ghidraAddress NTSC-U/C: 0x00155ba8
     * @ghidraAddress PAL: 0x00157410
     */
    void Restart();

    /**
     * Give the track to a player, ending the run of the previous player when it was catching.
     *
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x00155be8
     * @ghidraAddress PAL: 0x00157450
     */
    void SetPlayer(Player *pPlayer);

    /**
     * Judge a press on a lane, or explain the controls when the capturer is stopped.
     *
     * @param nLane The lane.
     * @ghidraAddress NTSC-U/C: 0x00155c68
     * @ghidraAddress PAL: 0x001574d0
     */
    void Press(int nLane);

    /**
     * Turn the autopilot on or off.
     *
     * @param nEnabled Non-zero to press each gem automatically.
     * @ghidraAddress NTSC-U/C: 0x00155ca0
     * @ghidraAddress PAL: 0x00157508
     */
    void SetFakeInput(int nEnabled);

    /**
     * Hide or show the run and the catch meter.
     *
     * @param nHidden Non-zero to hide them.
     * @ghidraAddress NTSC-U/C: 0x00155cb0
     * @ghidraAddress PAL: 0x00157518
     */
    void SetRunHidden(int nHidden);

    /**
     * Look for the run again from the current bar.
     *
     * @ghidraAddress NTSC-U/C: 0x00155ce8
     */
    void RefreshRun();

    /**
     * Find the first gem of the next run from a bar.
     *
     * @param nBar The bar.
     * @param pTick Receives the tick of the gem.
     * @param pLane Receives the lane of the gem.
     * @return Whether a run was found, and true without writing the outputs past the last bar.
     * @ghidraAddress NTSC-U/C: 0x00155d28
     * @ghidraAddress PAL: 0x001575b0
     */
    bool GetRunStart(int nBar, int *pTick, int *pLane);

    /**
     * Report the song ticks in one bar.
     *
     * @return The ticks.
     * @ghidraAddress NTSC-U/C: 0x0034cf40
     */
    int GetTicksPerBar() const override {
        return mTicksPerBar;
    }

    /**
     * Report the player who plays the track.
     *
     * @return The player, or null.
     * @ghidraAddress NTSC-U/C: 0x0034cf48
     */
    Player *GetPlayer() const override {
        return mPlayer;
    }

    /**
     * Report whether a bar can be caught and is not yet captured.
     *
     * @param nBar The bar.
     * @return Whether the bar is active.
     * @ghidraAddress NTSC-U/C: 0x00155e60
     * @ghidraAddress PAL: 0x001576e8
     */
    bool IsBarActive(int nBar) override;

    /**
     * Act on a gem the player hit.
     *
     * A gem at or before the last gem hit counts as a miss. Inside the run the hit adds to the
     * catch meter and the last gem of a bar completes the bar.
     *
     * @param nTick The song tick of the hit.
     * @param cursor The gem.
     * @param bRemote Whether the hit came from another console.
     * @ghidraAddress NTSC-U/C: 0x00156758
     * @ghidraAddress PAL: 0x00157fe0
     */
    void HitGem(int nTick, const GemCursor &cursor, bool bRemote) override;

    /**
     * Act on a gem the player let pass.
     *
     * @param nTick The song tick of the miss.
     * @param cursor The gem.
     * @param bRemote Whether the miss came from another console.
     * @ghidraAddress NTSC-U/C: 0x00156b08
     * @ghidraAddress PAL: 0x00158390
     */
    void MissGem(int nTick, const GemCursor &cursor, bool bRemote) override;

    /**
     * Report a cursor at the first gem of the track.
     *
     * @return The cursor.
     * @ghidraAddress NTSC-U/C: 0x00156bc0
     * @ghidraAddress PAL: 0x00158448
     */
    GemCursor GetCursor() override;

private:
    /**
     * Report whether a gem is the last of its bar.
     *
     * @param cursor The gem.
     * @return Whether no later gem lies in its bar.
     * @ghidraAddress NTSC-U/C: 0x00155dd0
     * @ghidraAddress PAL: 0x00157658
     */
    bool IsLastGemOfBar(const GemCursor &cursor);

    /**
     * Find the first active bar of a range.
     *
     * The first bar is tested even when the range is empty.
     *
     * @param nFrom The first bar, raised to 0.
     * @param nTo The bar after the range, lowered to the last bar.
     * @return The bar, or -1.
     * @ghidraAddress NTSC-U/C: 0x00155ec0
     * @ghidraAddress PAL: 0x00157748
     */
    int FindActiveBar(int nFrom, int nTo);

    /**
     * Find the end of the run that starts at an active bar.
     *
     * @param nStart The first bar of the run.
     * @param nMaxBars The most bars of a run.
     * @return The bar after the run.
     * @ghidraAddress NTSC-U/C: 0x00155f50
     * @ghidraAddress PAL: 0x001577d8
     */
    int FindRunEnd(int nStart, int nMaxBars);

    /**
     * Find the run that starts in a range of bars.
     *
     * @param nFrom The first bar.
     * @param nTo The bar after the range.
     * @param pStart Receives the first bar of the run, or -1.
     * @param pEnd Receives the bar after the run.
     * @return Whether a run was found.
     * @ghidraAddress NTSC-U/C: 0x00156000
     * @ghidraAddress PAL: 0x00157888
     */
    bool FindRun(int nFrom, int nTo, int *pStart, int *pEnd);

    /**
     * Look for the run from a bar, or schedule the search one bar later when none lies near.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x001560a8
     * @ghidraAddress PAL: 0x00157930
     */
    void UpdateRun(int nBar);

    /**
     * End the run when a tick lies at or after its start.
     *
     * @param nTick The tick.
     * @param nQuiet Non-zero to end the run without telling the receiver and the player.
     * @ghidraAddress NTSC-U/C: 0x001561d0
     * @ghidraAddress PAL: 0x00157a58
     */
    void EndRunIfStarted(int nTick, int nQuiet);

    /**
     * Lose the run and look for the next one.
     *
     * @param nTick The tick of the loss.
     * @param nQuiet Non-zero to end the run without telling the receiver and the player.
     * @ghidraAddress NTSC-U/C: 0x00156218
     * @ghidraAddress PAL: 0x00157aa0
     */
    void EndRun(int nTick, int nQuiet);

    /**
     * Act on the last gem of a bar of the run: award the power-up of the bar and capture the run
     * after its last bar.
     *
     * @param cursor The gem.
     * @param nQuiet Non-zero when the gem came from another console.
     * @ghidraAddress NTSC-U/C: 0x001562c8
     * @ghidraAddress PAL: 0x00157b50
     */
    void CompleteBar(const GemCursor &cursor, int nQuiet);

    /**
     * Capture the run and look for the next one.
     *
     * @param cursor The last gem of the run.
     * @param nQuiet Non-zero to capture without telling the receiver and the player.
     * @ghidraAddress NTSC-U/C: 0x001563f0
     * @ghidraAddress PAL: 0x00157c78
     */
    void Capture(const GemCursor &cursor, int nQuiet);

    /**
     * Show the catch meter of the player.
     *
     * @ghidraAddress NTSC-U/C: 0x001564e8
     * @ghidraAddress PAL: 0x00157d70
     */
    void UpdateMeter();

    /**
     * Hide the run on the player's track.
     *
     * @ghidraAddress NTSC-U/C: 0x00156578
     * @ghidraAddress PAL: 0x00157e00
     */
    void HideRun();

    /**
     * Show the run on the player's track, or hide it when there is none.
     *
     * @param nStyle The style of the display.
     * @ghidraAddress NTSC-U/C: 0x001565b8
     * @ghidraAddress PAL: 0x00157e40
     */
    void ShowRun(int nStyle);

    /**
     * Sound a wrong press and explain the energised notes unless an active bar is near.
     *
     * @param nTick The tick of the press.
     * @ghidraAddress NTSC-U/C: 0x00156650
     * @ghidraAddress PAL: 0x00157ed8
     */
    void ShowPressHint(int nTick);

    /**
     * Act on a press that caught no gem.
     *
     * @param nTick The tick of the press.
     * @param nLane The lane.
     * @ghidraAddress NTSC-U/C: 0x001569f8
     * @ghidraAddress PAL: 0x00158280
     */
    void MissPress(int nTick, int nLane);

    CatchReceiver *mCatchReceiver;            /*!< The receiver of the gem catcher. */
    CatchTrackState *mState;                  /*!< The state of the bars of the track. */
    int mReserved0C;                          // +0x0c, stored by the constructor, not read here.
    GemCatcher mCatcher;                      /*!< The judge of the presses. */
    NetFaker *mNetFaker;                      /*!< The stand-in of a remote player, online only. */
    Faker *mFaker;                            /*!< The autopilot, or null. */
    Receiver *mReceiver;                      /*!< The receiver of the runs. */
    Player *mPlayer;                          /*!< The player who plays the track, or null. */
    Muse *mLastMuse;                          /*!< The note of the last gem hit, or null. */
    int mTrack;                               /*!< The track. */
    int mTicksPerBar;                         /*!< The song ticks in one bar. */
    int mStopPreviousNote;                    /*!< Whether a hit stops the previous note. */
    int mEndBar;                              /*!< The bar the song ends at. */
    int mStarted;                             /*!< Whether the capturer judges the presses. */
    Ptr<SetCaptureStartCmd> mCaptureStartCmd; /*!< The delayed search for the next run. */
    int mGemsCaught;                          /*!< The gems caught in the run. */
    int mRunGems;                             /*!< The gems of the run. */
    int mRunStart;                            /*!< The first bar of the run, or -1. */
    int mRunEnd;                              /*!< The bar after the run, or -1. */
    int mRunHidden;                           /*!< Whether the run and the meter are hidden. */
};
