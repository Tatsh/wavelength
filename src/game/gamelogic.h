#pragma once

#include <vector>

#include "game/catchtrack.h"
#include "game/freestyletrack.h"
#include "game/inputevents.h"
#include "game/localplayer.h"
#include "game/player.h"
#include "game/playmap.h"
#include "game/song.h"
#include "game/track.h"
#include "game/gametrackselector.h"
#include "game/worldbeat.h"
#include "game/worldlogic.h"
#include "math/rand.h"
#include "os/command.h"
#include "os/commandscheduler.h"
#include "os/ptr.h"
#include "os/ramp.h"
#include "script/dataarray.h"
#include "synth/songspeed.h"

/**
 * Rules of the main game, shared by the solo, multiplayer, and tutorial logic.
 *
 * The RTTI records the class as deriving from WorldLogic. The logic creates the players, one
 * catch track for each catch track of the song, and the freestyle track. It counts the bars and
 * the sections of the song, places the power-ups, moves the players between the tracks, and
 * finds the phrase each player should play next.
 *
 * The derived classes supply GetTick(), Reserved11(), and the hooks from OnStart() on.
 */
class GameLogic : public WorldLogic {
public:
    /** Values of mState. */
    enum State {
        kStateIdle = 0,      /*!< The song has not started. */
        kStatePlaying = 1,   /*!< The song is playing. */
        kStatePaused = 2,    /*!< The song is paused, and mSavedState records the state to resume. */
        kStateSuspended = 3, /*!< The derived logic stopped the song without pausing it. */
        kStateRunning = 5,   /*!< The derived logic runs the song without the player input. */
        kStateEnding = 6,    /*!< The song ended, and the logic waits for the display to finish. */
        kStateFinished = 7,  /*!< Stop() ran. */
    };

    /** Kinds of power-up a player can hold. */
    enum Powerup {
        kPowerupNone = 0,        /*!< No power-up. */
        kPowerupAutocatcher = 1, /*!< Captures a phrase of the current track. */
        kPowerupMultiplier = 2,  /*!< Raises the score multiplier. */
        kPowerupSlowdown = 3,    /*!< Slows the song down for a while. */
        kPowerupFreestyle = 4,   /*!< Moves the player to the freestyle track. */
        kPowerupBumper = 5,      /*!< Pushes another player off a track. */
        kPowerupCrippler = 6,    /*!< Hinders another player. */
        kPowerupCount = 7,       /*!< Number of kinds, including kPowerupNone. */
    };

    /**
     * Change of the song speed while a slowdown runs.
     *
     * The RTTI records the class as nested in GameLogic and as deriving from Ramp.
     */
    class SpeedRamp : public Ramp {
    public:
        /**
         * Construct the ramp at a speed.
         *
         * @param pScheduler The scheduler the steps run on.
         * @param fSpeed The speed to start at.
         */
        SpeedRamp(CommandScheduler *pScheduler, float fSpeed) : Ramp(pScheduler, fSpeed) {
        }

        /**
         * Release the ramp.
         *
         * @ghidraAddress NTSC-U/C: 0x00337cd8
         * @ghidraAddress PAL: 0x003a5288
         */
        ~SpeedRamp() override {
        }

        /**
         * Play the song at a new speed.
         *
         * @param fValue The speed.
         * @param nTick The tick of the step.
         * @ghidraAddress NTSC-U/C: 0x00337d48
         * @ghidraAddress PAL: 0x003a52f8
         */
        void Apply(float fValue, [[maybe_unused]] int nTick) override {
            SetSongSpeed(fValue);
        }
    };

    /**
     * Per-player record of the phrase each player should play next.
     *
     * The RTTI records the class as nested in GameLogic. The structure is not polymorphic.
     */
    class PlayerData {
    public:
        /**
         * Construct the record around the command that ends the freestyle of the player.
         *
         * @param pEndFreestyle The command.
         * @ghidraAddress NTSC-U/C: 0x00110440
         * @ghidraAddress PAL: 0x00111bd8
         */
        explicit PlayerData(Command *pEndFreestyle);

        Ptr<Command> mEndFreestyleCmd; /*!< Runs EndFreestyle() for the player. */
        int mNextPhraseBar;            /*!< First bar of the phrase to play next, or -1. */
        int mLastNextPhraseBar;        /*!< The previous mNextPhraseBar. */
        int mMissedBar;                /*!< The bar the player last missed a phrase in. */
        int mSavedStreak;              /*!< The streak of the player when the phrase was found. */
        int mCapturedTrack;            /*!< The track the search for the next phrase skipped. */
    };

    /**
     * Construct the logic of a song.
     *
     * Creates one player for each player in the game database, the catch tracks and the freestyle
     * track of the song, assigns the players to the tracks, places the power-ups, and registers
     * the script commands that deploy a power-up.
     *
     * @param pSong The song.
     * @param pConfig The configuration of the logic.
     * @param nSeed The seed of the random numbers the logic draws.
     * @ghidraAddress NTSC-U/C: 0x00110480
     * @ghidraAddress PAL: 0x00111c18
     */
    GameLogic(Song *pSong, DataArray *pConfig, int nSeed);

    /**
     * Stop the song and release the players and the tracks.
     *
     * @ghidraAddress NTSC-U/C: 0x00111838
     * @ghidraAddress PAL: 0x00112fd0
     */
    ~GameLogic() override;

    /**
     * Start the song, the tracks, the players, and the bar counting.
     *
     * @ghidraAddress NTSC-U/C: 0x00111c90
     * @ghidraAddress PAL: 0x00113428
     */
    void Start() override;

    /**
     * Stop the song, the tracks, and the players.
     *
     * @ghidraAddress NTSC-U/C: 0x00112248
     * @ghidraAddress PAL: 0x001139e0
     */
    void Stop() override;

    /**
     * Report whether Stop() ran.
     *
     * @return Whether mState is kStateFinished.
     * @ghidraAddress NTSC-U/C: 0x00112968
     */
    bool IsFinished() const override;

    /**
     * Report mQuit.
     *
     * @return mQuit.
     * @ghidraAddress NTSC-U/C: 0x00112978
     * @ghidraAddress PAL: 0x00114198
     */
    int HasQuit() const override;

    /**
     * Report mReserved60.
     *
     * @return mReserved60.
     * @ghidraAddress NTSC-U/C: 0x00112980
     */
    int GetReserved60() const override;

    /**
     * Report whether a player is on the freestyle track.
     *
     * @param nPlayer The player.
     * @return Whether the track of the player is mFreestyleTrack.
     * @ghidraAddress NTSC-U/C: 0x00112988
     * @ghidraAddress PAL: 0x00114120
     */
    bool IsFreestyling(int nPlayer) override;

    /**
     * Stop the song once the ending finished.
     *
     * Only a logic in kStateEnding acts. The song stops once the camera move of the ending is done
     * and either the scheduler stopped or it arrived at mEndTick.
     *
     * @ghidraAddress NTSC-U/C: 0x001126a0
     * @ghidraAddress PAL: 0x00113e38
     */
    void Poll() override;

    /**
     * Report how far the song has played.
     *
     * @return GetTick() over the length of the song in ticks, limited to the range 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x00111bf8
     * @ghidraAddress PAL: 0x00113390
     */
    float GetProgress() override;

    /**
     * Move a playing player to the next or previous catch track.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00114c28
     * @ghidraAddress PAL: 0x001163c0
     */
    void HandleInput(const RotateEvent &event) override;

    /**
     * Pass a note to the player.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00114cb8
     * @ghidraAddress PAL: 0x00116450
     */
    void HandleInput(const PlayNoteEvent &event) override;

    /**
     * Pause the song from the controller of the player.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00114dd8
     * @ghidraAddress PAL: 0x00116570
     */
    void HandleInput(const BtnEvent<3> &event) override;

    /**
     * Toggle the display option of a single controller and save it in the profile.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00114e48
     * @ghidraAddress PAL: 0x001165e0
     */
    void HandleInput(const BtnEvent<4> &event) override;

    /**
     * Deploy the power-up of a player and log it.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00114f30
     * @ghidraAddress PAL: 0x001166c8
     */
    void HandleInput(const BtnEvent<5> &event) override;

    /**
     * Ignore the event.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00337ca8
     * @ghidraAddress PAL: 0x003a5258
     */
    void HandleInput([[maybe_unused]] const ChangeSectionEvent &event) override {
    }

    /**
     * Ignore the event.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00337cb0
     * @ghidraAddress PAL: 0x003a5260
     */
    void HandleInput([[maybe_unused]] const BtnEvent<9> &event) override {
    }

    /**
     * Pass the button to the player.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00114d00
     * @ghidraAddress PAL: 0x00116498
     */
    void HandleInput(const BtnEvent<10> &event) override;

    /**
     * Ignore the event.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00337ca0
     * @ghidraAddress PAL: 0x003a5250
     */
    void HandleInput([[maybe_unused]] const BtnEvent<8> &event) override {
    }

    /**
     * Pass the stick event to the player.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00114d48
     * @ghidraAddress PAL: 0x001164e0
     */
    void HandleInput(const StickEvent<2> &event) override;

    /**
     * Pass the stick event to the player.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00114d90
     * @ghidraAddress PAL: 0x00116528
     */
    void HandleInput(const StickEvent<6> &event) override;

    /**
     * Pause or resume the song, the force feedback, and the controllers.
     *
     * @param bPaused Whether to pause.
     * @param nPad The controller that requested the change.
     * @param nReason The reason the change was requested.
     * @ghidraAddress NTSC-U/C: 0x00112558
     * @ghidraAddress PAL: 0x00113cf0
     */
    void SetPaused(bool bPaused, int nPad, int nReason) override;

    /**
     * Report whether the song is playing.
     *
     * @return Whether mState is kStatePlaying.
     * @ghidraAddress NTSC-U/C: 0x00337cc8
     */
    bool IsPlaying() const override {
        return mState == kStatePlaying;
    }

    /**
     * Slow the song down for a player.
     *
     * @param pPlayer The player.
     * @return Whether the power-up was deployed.
     * @ghidraAddress NTSC-U/C: 0x00115700
     * @ghidraAddress PAL: 0x00116e98
     */
    virtual bool DeploySlowdown(Player *pPlayer);

    /**
     * Move a player to the freestyle track for the configured number of bars.
     *
     * @param pPlayer The player.
     * @return Whether the power-up was deployed.
     * @ghidraAddress NTSC-U/C: 0x001159d8
     * @ghidraAddress PAL: 0x00117170
     */
    virtual bool DeployFreestyle(Player *pPlayer);

    /**
     * Refuse a bumper. The solo game has no bumper.
     *
     * @param pPlayer The player.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00337cb8
     * @ghidraAddress PAL: 0x003a5268
     */
    virtual bool DeployBumper([[maybe_unused]] Player *pPlayer) {
        return false;
    }

    /**
     * Refuse a crippler. The solo game has no crippler.
     *
     * @param pPlayer The player.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00337cc0
     * @ghidraAddress PAL: 0x003a5270
     */
    virtual bool DeployCrippler([[maybe_unused]] Player *pPlayer) {
        return false;
    }

    /**
     * Move a player from the freestyle track back to a catch track.
     *
     * @param pPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x00115b98
     * @ghidraAddress PAL: 0x00117330
     */
    virtual void EndFreestyle(Player *pPlayer);

    /** Act once Start() finished. */
    virtual void OnStart() = 0;

    /**
     * Act once Finish() stopped the players.
     *
     * @param bWon Whether the song was won.
     */
    virtual void OnFinish(bool bWon) = 0;

    /**
     * Act at the start of every bar.
     *
     * @param nBar The bar.
     */
    virtual void OnBar(int nBar) = 0;

    /** Act at the start of every section. */
    virtual void OnSection() = 0;

    /**
     * Act once CapturePhrase() found the next phrases.
     *
     * @param pTrack The track the phrase was captured on.
     * @param pPlayer The player that captured it.
     * @param nBar The first bar the capture clears.
     * @param bStreak Whether the capture continues the streak of the player.
     */
    virtual void OnPhraseCaptured(CatchTrack *pTrack, Player *pPlayer, int nBar, bool bStreak) = 0;

    /**
     * Act once a phrase of a track ended.
     *
     * @param pTrack The track.
     */
    virtual void OnPhraseEnded(Track *pTrack) = 0;

    /**
     * Act once a player missed a phrase.
     *
     * @param pTrack The track the phrase was missed on.
     */
    virtual void OnPhraseMissed(Track *pTrack) = 0;

    /**
     * Report whether a player is on the freestyle track while the song plays.
     *
     * @return Whether the freestyle track has a player and mState is kStatePlaying or
     *         kStateRunning.
     * @ghidraAddress NTSC-U/C: 0x001129c8
     * @ghidraAddress PAL: 0x00114160
     */
    bool IsFreestyleTrackActive() const;

    /**
     * Report whether a catch track has a phrase in the current bar.
     *
     * @return mPhraseThisBar.
     * @ghidraAddress NTSC-U/C: 0x00112a00
     */
    bool HasPhraseThisBar() const;

    /**
     * Start the ending of the song.
     *
     * Fades the mixer out over at least one second and schedules WorldLogic::AllNotesOff() at the
     * end of the fade, unless the song is paused. The logic enters kStateEnding either way.
     *
     * @ghidraAddress NTSC-U/C: 0x00112728
     * @ghidraAddress PAL: 0x00113ec0
     */
    void EndSong();

    /**
     * Stop the players at the end of the song.
     *
     * A won song gives the first player the freestyle track and checks for an unlocked song. A
     * demo starts the ending at once, and any other game passes the result to OnFinish().
     *
     * @param bWon Whether the song was won.
     * @ghidraAddress NTSC-U/C: 0x00112de0
     * @ghidraAddress PAL: 0x00114578
     */
    void Finish(bool bWon);

    /**
     * Record the capture of a phrase and find the next phrase of each local player.
     *
     * @param pTrack The track the phrase was captured on.
     * @param pPlayer The player that captured it.
     * @param nBar The first bar the capture clears.
     * @param bStreak Whether the capture continues the streak of the player.
     * @ghidraAddress NTSC-U/C: 0x00113360
     * @ghidraAddress PAL: 0x00114af8
     */
    void CapturePhrase(CatchTrack *pTrack, Player *pPlayer, int nBar, bool bStreak);

    /**
     * Lengthen or reset the streak of the player of a track and find the next phrase.
     *
     * @param pTrack The track.
     * @param nBar The bar the phrase ended in.
     * @param nFromBar The first bar to search for the next phrase from.
     * @ghidraAddress NTSC-U/C: 0x001135b0
     * @ghidraAddress PAL: 0x00114d48
     */
    void ContinueStreak(Track *pTrack, int nBar, int nFromBar);

    /**
     * Report the end of a phrase to the triggers and to OnPhraseEnded().
     *
     * @param pTrack The track.
     * @ghidraAddress NTSC-U/C: 0x00114ac0
     * @ghidraAddress PAL: 0x00116258
     */
    void EndPhrase(Track *pTrack);

    /**
     * Record a missed phrase and report it to the triggers and to OnPhraseMissed().
     *
     * @param pTrack The track the phrase was missed on.
     * @ghidraAddress NTSC-U/C: 0x00114b18
     * @ghidraAddress PAL: 0x001162b0
     */
    void MissPhrase(Track *pTrack);

    /**
     * Enable the catch tracks of the next step of the song's enable order.
     *
     * @param nBar The bar the tracks start at.
     * @ghidraAddress NTSC-U/C: 0x001150b0
     * @ghidraAddress PAL: 0x00116848
     */
    void EnableNextTracks(int nBar);

    /**
     * Move a player to the freestyle track.
     *
     * @param pPlayer The player.
     * @param bVictory Whether the freestyle rewards a won song.
     * @ghidraAddress NTSC-U/C: 0x00115180
     * @ghidraAddress PAL: 0x00116918
     */
    void EnterFreestyle(Player *pPlayer, bool bVictory);

    /**
     * Move a player from the freestyle track to a catch track and find the next phrase.
     *
     * @param pPlayer The player.
     * @param nTrack The catch track, or -1 for the track the camera of the player shows.
     * @ghidraAddress NTSC-U/C: 0x001151a0
     * @ghidraAddress PAL: 0x00116938
     */
    void LeaveFreestyle(Player *pPlayer, int nTrack);

    /**
     * Slow the song down.
     *
     * @ghidraAddress NTSC-U/C: 0x00115288
     * @ghidraAddress PAL: 0x00116a20
     */
    void StartSlowdown();

    /**
     * Return the song to its speed after StartSlowdown().
     *
     * @ghidraAddress NTSC-U/C: 0x001152f0
     * @ghidraAddress PAL: 0x00116a88
     */
    void StopSlowdown();

    /**
     * Capture a phrase of the track of a player and pulse the controllers of every player on it.
     *
     * @param pPlayer The player.
     * @return Whether the power-up was deployed.
     * @ghidraAddress NTSC-U/C: 0x001157f8
     * @ghidraAddress PAL: 0x00116f90
     */
    bool DeployAutocatcher(Player *pPlayer);

    /**
     * Raise the multiplier of a player.
     *
     * @param pPlayer The player.
     * @return Whether the power-up was deployed.
     * @ghidraAddress NTSC-U/C: 0x00115990
     * @ghidraAddress PAL: 0x00117128
     */
    bool DeployMultiplier(Player *pPlayer);

    Song *mSong;                              /*!< The song. */
    int mState;                               /*!< One of State. */
    int mReserved0c;                          // +0x0c, cleared by the constructor.
    int mTicksPerBar;                         /*!< Length of a bar, in ticks. */
    int mNumBars;                             /*!< Length of the song, in bars. */
    PlayMap *mPlayMap;                        /*!< The map of the song positions. */
    std::vector<Player *> mPlayers;           /*!< Every player, by index. */
    std::vector<LocalPlayer *> mLocalPlayers; /*!< The players on this console. */
    std::vector<CatchTrack *> mCatchTracks;   /*!< The catch tracks, by index. */
    FreestyleTrack *mFreestyleTrack;          /*!< The freestyle track, or null. */
    GameTrackSelector *mTrackSelector;        /*!< The assignment of the players to the tracks. */
    SpeedRamp *mSpeedRamp;                    /*!< The speed of the song. */
    bool mPhraseThisBar;                      /*!< Whether a catch track has a phrase this bar. */
    int mQuit;                                /*!< Set by the derived logic when a player quit. */
    int mReserved60;                          // +0x60, reported by GetReserved60().
    int mSectionStartBar;                     /*!< First bar of the current section. */
    int mNextSectionBar;                      /*!< First bar of the next section. */
    int mSection;                             /*!< Index of the current section. */
    WorldBeat mWorldBeat;                     /*!< The events of the track named "WORLD". */
    int mSavedState;                          /*!< The state SetPaused() restores. */
    int mReserved8c;                          // +0x8c, not written by the constructor.
    bool mSlowdown;                           /*!< Whether StartSlowdown() slowed the song. */
    int mNextEnableStep;                      /*!< The step EnableNextTracks() enables next. */
    std::vector<PlayerData> mPlayerData;      /*!< The record of each player. */
    Ptr<Command> mBarCmd;                     /*!< Runs TickBar(). */
    Ptr<Command> mStopSlowdownCmd;            /*!< Runs StopSlowdown(). */
    Rand mRand;                               /*!< The random numbers of the logic. */
    int mEndTick;                             /*!< The tick the song stops at. */

protected:
    /**
     * Assign each player to a random catch track.
     *
     * @ghidraAddress NTSC-U/C: 0x001111c8
     * @ghidraAddress PAL: 0x00112960
     */
    void AssignTracks();

    /**
     * Apply one effect set of the song.
     *
     * @param nSet The effect set.
     * @ghidraAddress NTSC-U/C: 0x00111800
     * @ghidraAddress PAL: 0x00112f98
     */
    void ApplyFreestyleEffect(int nSet);

    /**
     * Show the localised text of a power-up.
     *
     * Only the solo game shows the text.
     *
     * @param pszToken The token of the text.
     * @param nPlayer The player. The body does not read it.
     * @ghidraAddress NTSC-U/C: 0x001128f0
     * @ghidraAddress PAL: 0x00114088
     */
    void ShowPowerupText(const char *pszToken, int nPlayer);

    /**
     * Create the catch tracks and the freestyle track of the song.
     *
     * @param pFreestyleType Receives the track type of the freestyle track.
     * @param pFreestyleInstrument Receives the instrument of the freestyle track.
     * @ghidraAddress NTSC-U/C: 0x00112a08
     * @ghidraAddress PAL: 0x001141a0
     */
    void CreateTracks(int *pFreestyleType, int *pFreestyleInstrument);

    /**
     * Count one bar, starting the next section when it begins.
     *
     * The command in mBarCmd runs the routine at the start of every bar.
     *
     * @ghidraAddress NTSC-U/C: 0x00113190
     * @ghidraAddress PAL: 0x00114928
     */
    void TickBar();

    /**
     * Start the next section.
     *
     * @ghidraAddress NTSC-U/C: 0x001132b0
     * @ghidraAddress PAL: 0x00114a48
     */
    void AdvanceSection();

    /**
     * Forget the next phrase of a player.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x00113650
     * @ghidraAddress PAL: 0x00114de8
     */
    void ClearNextPhrase(int nPlayer);

    /**
     * Find the first phrase on another catch track at or after a bar, and point the player at it.
     *
     * @param nPlayer The player.
     * @param nTrack The catch track to skip.
     * @param nFromBar The first bar to search.
     * @ghidraAddress NTSC-U/C: 0x001136d8
     * @ghidraAddress PAL: 0x00114e70
     */
    void FindNextPhrase(int nPlayer, int nTrack, int nFromBar);

    /**
     * Place a random power-up in every bar of every catch track.
     *
     * @ghidraAddress NTSC-U/C: 0x001138c0
     * @ghidraAddress PAL: 0x00115058
     */
    void PlaceRandomPowerups();

    /**
     * Place the power-ups of every section in the busiest bars of the catch tracks.
     *
     * @ghidraAddress NTSC-U/C: 0x001139b8
     * @ghidraAddress PAL: 0x00115150
     */
    void PlacePowerups();

    /**
     * Report the `controller` argument of a script command.
     *
     * @param pCommand The command.
     * @return The controller, or -1 when the command does not list one.
     * @ghidraAddress NTSC-U/C: 0x00115348
     * @ghidraAddress PAL: 0x00116ae0
     */
    static int GetControllerArgument(DataArray *pCommand);

    /**
     * Deploy the autocatcher for a controller on the `autocatcher` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x00115380
     * @ghidraAddress PAL: 0x00116b18
     */
    static void OnAutocatcherCommand(DataArray *pCommand, void *pUserData);

    /**
     * Deploy the multiplier for a controller on the `multiplier` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x001153d0
     * @ghidraAddress PAL: 0x00116b68
     */
    static void OnMultiplierCommand(DataArray *pCommand, void *pUserData);

    /**
     * Turn the slowdown on or off outside an online game on the `slow` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x00115420
     * @ghidraAddress PAL: 0x00116bb8
     */
    static void OnSlowCommand(DataArray *pCommand, void *pUserData);

    /**
     * Deploy the freestyle for a controller on the `freestyle` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x00115480
     * @ghidraAddress PAL: 0x00116c18
     */
    static void OnFreestyleCommand(DataArray *pCommand, void *pUserData);

    /**
     * Move the player of a controller onto or off the freestyle track on the `freestyle_toggle`
     * script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x001154f0
     * @ghidraAddress PAL: 0x00116c88
     */
    static void OnFreestyleToggleCommand(DataArray *pCommand, void *pUserData);

    /**
     * Deploy the bumper for a controller on the `bumper` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x00115560
     * @ghidraAddress PAL: 0x00116cf8
     */
    static void OnBumperCommand(DataArray *pCommand, void *pUserData);

    /**
     * Deploy the crippler for a controller on the `crippler` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x001155d0
     * @ghidraAddress PAL: 0x00116d68
     */
    static void OnCripplerCommand(DataArray *pCommand, void *pUserData);

    /**
     * Give a power-up to the player of a controller on the `powerup` script command while the
     * power-up cheat is on.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x00115640
     * @ghidraAddress PAL: 0x00116dd8
     */
    static void OnPowerupCommand(DataArray *pCommand, void *pUserData);
};
