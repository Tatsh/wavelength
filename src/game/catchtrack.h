#pragma once

#include <vector>

#include "game/backmusic.h"
#include "game/catchtrackdata.h"
#include "game/catchtrackdisplay.h"
#include "game/catchtrackmusic.h"
#include "game/catchtrackstate.h"
#include "game/gemcursor.h"
#include "game/guideticker.h"
#include "game/player.h"
#include "game/playmap.h"
#include "game/sectionboundaries.h"
#include "game/track.h"
#include "game/trackcapturer.h"
#include "os/command.h"
#include "os/mem.h"
#include "os/ptr.h"

// GameLogic and CatchTrack refer to each other.
class GameLogic;

/**
 * Track whose phrases a player captures by playing them.
 *
 * The RTTI records the class as deriving from Track, and includes the nested
 * CatchTrack::DeactivateCmd and CatchTrack::CaptureReceiver. The object is 0x148 bytes, and
 * GameLogic allocates it under the tag "CatchTrack". A captured run of bars plays its music and
 * the background music of the track until the run ends.
 */
class CatchTrack : public Track {
public:
    /**
     * Command that ends the captured run of the track.
     *
     * The RTTI includes the nested name and records Command as the base. Its members are inline.
     */
    class DeactivateCmd : public Command {
    public:
        /**
         * Construct the command of a track.
         *
         * @param pTrack The track.
         */
        explicit DeactivateCmd(CatchTrack *pTrack) : mTrack(pTrack) {
        }

        /**
         * Release the command.
         *
         * @ghidraAddress NTSC-U/C: 0x0034a970
         * @ghidraAddress PAL: 0x003b7da0
         */
        ~DeactivateCmd() override {
        }

        /**
         * End the phrase of the track and stop its background music.
         *
         * @ghidraAddress NTSC-U/C: 0x0034a9e8
         * @ghidraAddress PAL: 0x003b7e18
         */
        void Execute() override;

    private:
        CatchTrack *mTrack; /*!< The track. */
    };

    /**
     * Receiver of the runs of the capturer of a track.
     *
     * The RTTI includes the nested name and records TrackCapturer::Receiver as the base. Its
     * members are inline.
     */
    class CaptureReceiver : public TrackCapturer::Receiver {
    public:
        /**
         * Construct the receiver of a track.
         *
         * @param pTrack The track.
         */
        explicit CaptureReceiver(CatchTrack *pTrack) : mTrack(pTrack) {
        }

        /**
         * Release the receiver.
         *
         * @ghidraAddress NTSC-U/C: 0x0034aaa8
         * @ghidraAddress PAL: 0x003b7ed8
         */
        ~CaptureReceiver() override {
        }

        /**
         * Pass a capture to the track.
         *
         * @param cursor The last gem of the run.
         * @ghidraAddress NTSC-U/C: 0x0034ab28
         * @ghidraAddress PAL: 0x003b7f58
         */
        void OnCapture(const GemCursor &cursor) override {
            mTrack->OnCapture(cursor);
        }

        /**
         * Pass a lost run to the track.
         *
         * @ghidraAddress NTSC-U/C: 0x0034ab48
         * @ghidraAddress PAL: 0x003b7f78
         */
        void OnRunLost() override {
            mTrack->OnRunLost();
        }

        /**
         * Pass the start of a run to the track.
         *
         * @param cursor The first gem of the run.
         * @param nEndBar The bar after the run.
         * @ghidraAddress NTSC-U/C: 0x0034ab68
         * @ghidraAddress PAL: 0x003b7f98
         */
        void OnRunStart(const GemCursor &cursor, int nEndBar) override {
            mTrack->OnRunStart(cursor, nEndBar);
        }

    private:
        CatchTrack *mTrack; /*!< The track. */
    };

    /**
     * Release a track to the pool heap GameLogic allocated it from.
     *
     * @param pBlock The block.
     */
    static void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Construct a catch track.
     *
     * @param pLogic The logic of the game.
     * @param pData The gems of the track.
     * @param pfMsPerTick The duration of one tick in milliseconds.
     * @param pPlayMap The map of the song positions.
     * @param pSections The sections of the song.
     * @param nIndex The index of the track among the catch tracks.
     * @param nIntroBars The bars before the first section.
     * @param nNumBars The length of the song in bars.
     * @param nTicksPerBar The length of a bar in ticks.
     * @param nFlags Nonzero for each bar of music to stop the bar before it.
     * @ghidraAddress NTSC-U/C: 0x0014c4a8
     * @ghidraAddress PAL: 0x0014de48
     */
    CatchTrack(GameLogic *pLogic,
               CatchTrackData *pData,
               const float *pfMsPerTick,
               PlayMap *pPlayMap,
               SectionBoundaries *pSections,
               int nIndex,
               int nIntroBars,
               int nNumBars,
               int nTicksPerBar,
               int nFlags);

    /**
     * Stop and release the track.
     *
     * @ghidraAddress NTSC-U/C: 0x0014c698
     * @ghidraAddress PAL: 0x0014e038
     */
    ~CatchTrack() override;

    /**
     * Start the display, the music, and the guide ticker of the track, once.
     *
     * @ghidraAddress NTSC-U/C: 0x0014c8e0
     * @ghidraAddress PAL: 0x0014e280
     */
    void Start() override;

    /**
     * Stop the track and every scheduled command.
     *
     * @ghidraAddress NTSC-U/C: 0x0014ca38
     * @ghidraAddress PAL: 0x0014e3d8
     */
    void Stop() override;

    /**
     * Give the track to the player it plays for.
     *
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x0014cb68
     * @ghidraAddress PAL: 0x0014e508
     */
    void SetPlayer(Player *pPlayer) override;

    /**
     * Pass a button press of the track's player to the capturer, or show another player that the
     * track is behind.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0014cde0
     * @ghidraAddress PAL: 0x0014e780
     */
    void HandleInput(Player *pPlayer, const PlayNoteEvent &event) override;

    /**
     * Ignore the input.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0034a950
     * @ghidraAddress PAL: 0x003b7d80
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const BtnEvent<8> &event) override {
    }

    /**
     * Ignore the input.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0034a958
     * @ghidraAddress PAL: 0x003b7d88
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const StickEvent<2> &event) override {
    }

    /**
     * Ignore the input.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0034a960
     * @ghidraAddress PAL: 0x003b7d90
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const StickEvent<6> &event) override {
    }

    /**
     * Ignore the input.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0034a968
     * @ghidraAddress PAL: 0x003b7d98
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const BtnEvent<10> &event) override {
    }

    /**
     * Play a background music track while a captured run of this track plays.
     *
     * @param pMusic The background music.
     * @ghidraAddress NTSC-U/C: 0x0014c798
     * @ghidraAddress PAL: 0x0014e138
     */
    void AddBackMusic(BackMusic *pMusic);

    /**
     * Redraw the track from a bar and restart the capturer after the song gained a loop.
     *
     * The name is inferred.
     *
     * @param nBar The bar the loop plays from.
     * @ghidraAddress NTSC-U/C: 0x0014cc18
     * @ghidraAddress PAL: 0x0014e5b8
     */
    void Loop(int nBar);

    /**
     * Place a power-up in a bar.
     *
     * @param nBar The bar the song plays.
     * @param nPowerup One of GameLogic::Powerup.
     * @ghidraAddress NTSC-U/C: 0x0014cc50
     * @ghidraAddress PAL: 0x0014e5f0
     */
    void SetPowerup(int nBar, int nPowerup);

    /**
     * Place a power-up in a bar as written.
     *
     * The name is inferred.
     *
     * @param nBar The bar as written.
     * @param nPowerup One of GameLogic::Powerup.
     * @ghidraAddress NTSC-U/C: 0x0014cc70
     * @ghidraAddress PAL: 0x0014e610
     */
    void SetBarPowerup(int nBar, int nPowerup);

    /**
     * Enable the bars with gems from a bar on, except the checkpoint bars at the start of each
     * section, then redraw the track and start the capturer.
     *
     * @param nBar The bar. A song with a loop enables every bar.
     * @ghidraAddress NTSC-U/C: 0x0014cc90
     * @ghidraAddress PAL: 0x0014e630
     */
    void Enable(int nBar);

    /**
     * Enable every bar, give every bar to a player, and move the display to a tick.
     *
     * The name is inferred.
     *
     * @param pPlayer The player, or null.
     * @param nTick The song tick to show from.
     * @ghidraAddress NTSC-U/C: 0x0014ce80
     * @ghidraAddress PAL: 0x0014e820
     */
    void Restart(Player *pPlayer, int nTick);

    /**
     * Report whether a phrase of the track is in a bar.
     *
     * @param nBar The bar.
     * @return Whether a phrase is in the bar.
     * @ghidraAddress NTSC-U/C: 0x0014cef8
     * @ghidraAddress PAL: 0x0014e898
     */
    bool HasPhraseAt(int nBar);

    /**
     * Find the first gem of the next phrase from a bar.
     *
     * The name is inferred.
     *
     * @param nBar The bar.
     * @param pTick Receives the tick of the gem.
     * @param pLane Receives the lane of the gem.
     * @return True when a phrase was found.
     * @ghidraAddress NTSC-U/C: 0x0014cf20
     * @ghidraAddress PAL: 0x0014e8c0
     */
    bool FindPhrase(int nBar, int *pTick, int *pLane);

    /**
     * Report the tick of the first gem at or after a tick.
     *
     * @param nTick The tick.
     * @param pLane Receives the lane of the gem when not null.
     * @return The tick of the gem, or -1 when no gem follows.
     * @ghidraAddress NTSC-U/C: 0x0014cf40
     * @ghidraAddress PAL: 0x0014e8e0
     */
    int GetNextGemTick(int nTick, int *pLane);

    /**
     * Capture the phrase of a bar for a player.
     *
     * @param nBar The bar.
     * @param pPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x0014cfa0
     * @ghidraAddress PAL: 0x0014e940
     */
    void Capture(int nBar, Player *pPlayer);

    /**
     * Capture the phrase of a bar or the next bar for a player with the autocatcher power-up.
     *
     * @param nBar The bar.
     * @param pPlayer The player.
     * @return Whether a phrase was captured.
     * @ghidraAddress NTSC-U/C: 0x0014cfd0
     * @ghidraAddress PAL: 0x0014e970
     */
    bool Autocatch(int nBar, Player *pPlayer);

    /**
     * Give a player the freestyle power-up for a number of bars.
     *
     * @param nBar The first bar.
     * @param nBars The number of bars.
     * @param pPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x0014d250
     * @ghidraAddress PAL: 0x0014ebf0
     */
    void Freestyle(int nBar, int nBars, Player *pPlayer);

    /**
     * Give a run of bars as written to a player, or take them from every player.
     *
     * The name is inferred.
     *
     * @param nBar The first bar.
     * @param nBars The number of bars.
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x0014d420
     * @ghidraAddress PAL: 0x0014edc0
     */
    void AssignBars(int nBar, int nBars, Player *pPlayer);

    /**
     * Turn on or off the automatic play of the track's gems.
     *
     * The name is inferred.
     *
     * @param bAutopilot Play the gems automatically.
     * @ghidraAddress NTSC-U/C: 0x0014d790
     * @ghidraAddress PAL: 0x0014f130
     */
    void SetAutopilot(bool bAutopilot);

    /**
     * Hide or show the seeker of the track.
     *
     * The name is inferred.
     *
     * @param bNoSeeker Hide the seeker.
     * @ghidraAddress NTSC-U/C: 0x0014d7b0
     * @ghidraAddress PAL: 0x0014f150
     */
    void SetNoSeeker(bool bNoSeeker);

private:
    /**
     * Capture the bars of a run from a bar for a player, through the end of the section when the
     * run arrives at it, and report the capture.
     *
     * The name is inferred.
     *
     * @param nBar The first bar.
     * @param pPlayer The player.
     * @param bAuto Whether the autocatcher power-up captured the run.
     * @ghidraAddress NTSC-U/C: 0x0014d138
     * @ghidraAddress PAL: 0x0014ead8
     */
    void CaptureBars(int nBar, Player *pPlayer, bool bAuto);

    /**
     * Give a range of bars to a player, start the background music, schedule the end of the run,
     * and show the run.
     *
     * The name is inferred.
     *
     * @param nStartBar The first bar.
     * @param nEndBar The bar after the range, limited to the end of the song.
     * @param pPlayer The player.
     * @param bAuto Whether the autocatcher power-up captured the run.
     * @param bFreestyle Whether the freestyle power-up gave the run.
     * @ghidraAddress NTSC-U/C: 0x0014d298
     * @ghidraAddress PAL: 0x0014ec38
     */
    void ActivateBars(int nStartBar, int nEndBar, Player *pPlayer, bool bAuto, bool bFreestyle);

    /**
     * Show whether the bar the song plays has notes the player can energise.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0014d490
     * @ghidraAddress PAL: 0x0014ee30
     */
    void UpdateHint();

    /**
     * Schedule UpdateHint() five bars from now, unless that is past the end of the song.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0014d540
     * @ghidraAddress PAL: 0x0014eee0
     */
    void ScheduleHint();

    /**
     * Report the bars a capture clears ahead of the captured bar, for the number of players.
     *
     * The name is inferred.
     *
     * @return The bars.
     * @ghidraAddress NTSC-U/C: 0x0014d5f8
     * @ghidraAddress PAL: 0x0014ef98
     */
    static int GetStrandBars();

    /**
     * Capture the run the player completed.
     *
     * The name is inferred.
     *
     * @param cursor The last gem of the run.
     * @ghidraAddress NTSC-U/C: 0x0014d638
     * @ghidraAddress PAL: 0x0014efd8
     */
    void OnCapture(const GemCursor &cursor);

    /**
     * Report a missed phrase and the streak it broke.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0014d6b8
     * @ghidraAddress PAL: 0x0014f058
     */
    void OnRunLost();

    /**
     * Continue the streak of the player into the run that starts.
     *
     * The name is inferred.
     *
     * @param cursor The first gem of the run.
     * @param nEndBar The bar after the run.
     * @ghidraAddress NTSC-U/C: 0x0014d738
     * @ghidraAddress PAL: 0x0014f0d8
     */
    void OnRunStart(const GemCursor &cursor, int nEndBar);

    PlayMap *mPlayMap;                   /*!< The map of the song positions. */
    GameLogic *mLogic;                   /*!< The logic of the game. */
    CatchTrackState mState;              /*!< The state of the bars of the track. */
    const SectionBoundaries *mSections;  /*!< The sections of the song. */
    int mTicksPerBar;                    /*!< The length of a bar in ticks. */
    int mNumBars;                        /*!< The length of the song in bars. */
    CatchTrackDisplay mDisplay;          /*!< The display of the bars and gems. */
    CatchTrackMusic mMusic;              /*!< The music of the captured bars. */
    GuideTicker mGuideTicker;            /*!< The guide sound of the gems. */
    CaptureReceiver *mReceiver;          /*!< The receiver of the runs of mCapturer. */
    Ptr<Command> mDeactivateCommand;     /*!< The DeactivateCmd of the track. */
    Ptr<Command> mIdleCommand;           /*!< Never set. Stop() withdraws it. */
    Ptr<Command> mHintCommand;           /*!< Calls UpdateHint(). */
    std::vector<BackMusic *> mBackMusic; /*!< The music a captured run plays. */
    int mShowHints;                      /*!< Whether the hints are shown, outside the tutorial. */
    TrackCapturer mCapturer;             /*!< The rules of the runs of the track. */
    int mStarted;                        /*!< Whether Start() ran since the last Stop(). */
    int mEnabled;                        /*!< Whether Enable() ran. */
};
