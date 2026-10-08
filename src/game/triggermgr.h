#pragma once

#include <list>
#include <utility>
#include <vector>

#include "game/triggeraction.h"
#include "game/triggerevent.h"
#include "os/binstream.h"
#include "os/string.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uiscreenchangemsg.h"

/**
 * Registry of the triggers that fire actions on game and display events.
 *
 * The name is inferred from the "trigger" section of the configuration Init() reads and from the
 * TriggerEvent, TriggerAction, and TriggerCondition classes the RTTI includes. The one instance is
 * TheTriggerMgr, 0x198 bytes.
 *
 * Each event routine records the values of the event for the conditions to test, fires the
 * TriggerEvent of its kind, and prints a debug line when Init() enabled the kind. Actions run on
 * one of two clocks, which MetagameEvent() advances. Poll() runs the scheduled actions whose time
 * has come and polls the running ones.
 */
class TriggerMgr {
public:
    /** The clocks an action runs on, the indices of mTime. */
    enum Clock {
        kClockGame = 0, /*!< The clock of the game, the default. */
        kClockReal = 1, /*!< The real-time clock, chosen by `realtime` in a trigger file. */
        kNumClocks = 2, /*!< The number of clocks. */
    };

    /** Sizes of the per-player and per-track records. */
    enum {
        kNumPlayers = 4, /*!< Players with a record of their own. */
        kNoPlayer = 4,   /*!< The mCapturer value of a track no player has captured. */
        kNumTracks = 8,  /*!< Tracks with a record of their own. */
    };

    /**
     * Construct the registry with an empty event of each kind.
     *
     * @ghidraAddress NTSC-U/C: 0x001fd6b8
     * @ghidraAddress PAL: 0x00206458
     */
    TriggerMgr();

    /**
     * Release every trigger.
     *
     * @ghidraAddress NTSC-U/C: 0x001fd9c8
     * @ghidraAddress PAL: 0x00206768
     */
    ~TriggerMgr();

    /**
     * Read the "trigger" section of the configuration.
     *
     * Each kind of event the `debug` list names sets its bit of mDebug.
     *
     * @ghidraAddress NTSC-U/C: 0x001fdb38
     * @ghidraAddress PAL: 0x002068d8
     */
    void Init();

    /**
     * Release every registered trigger.
     *
     * The running and scheduled actions are forgotten. Their handlers deleted them.
     *
     * @ghidraAddress NTSC-U/C: 0x001fe6e0
     * @ghidraAddress PAL: 0x00207480
     */
    void Terminate();

    /**
     * Record which paths of the arena are unlocked.
     *
     * @param paths One flag per path.
     * @ghidraAddress NTSC-U/C: 0x001fdbe8
     * @ghidraAddress PAL: 0x00206988
     */
    void SetPaths(const std::vector<bool> &paths);

    /**
     * Report whether a path exists and is unlocked.
     *
     * @param nPath The path.
     * @return Whether SetPaths() recorded the path as unlocked.
     * @ghidraAddress NTSC-U/C: 0x001fe068
     * @ghidraAddress PAL: 0x00206e08
     */
    bool IsPathUnlocked(unsigned int nPath) const;

    /**
     * Report whether a path exists.
     *
     * @param nPath The path.
     * @return Whether SetPaths() recorded a flag for the path.
     * @ghidraAddress NTSC-U/C: 0x001fe180
     * @ghidraAddress PAL: 0x00206f20
     */
    bool PathExists(unsigned int nPath) const;

    /**
     * Schedule an action to run after a delay on the current clock.
     *
     * The action follows the actions scheduled for the same time.
     *
     * @param pAction The action.
     * @param flDelay The delay.
     * @ghidraAddress NTSC-U/C: 0x001fe1d8
     * @ghidraAddress PAL: 0x00206f78
     */
    void Schedule(TriggerAction *pAction, float flDelay);

    /**
     * Poll an action every frame until it reports itself done.
     *
     * An action already running moves to the end of the list.
     *
     * @param pAction The action.
     * @ghidraAddress NTSC-U/C: 0x001fe340
     * @ghidraAddress PAL: 0x002070e0
     */
    void AddRunning(TriggerAction *pAction);

    /**
     * Run the actions whose time has come, and poll the running actions.
     *
     * @ghidraAddress NTSC-U/C: 0x001fe460
     * @ghidraAddress PAL: 0x00207200
     */
    void Poll();

    /**
     * Read triggers from a data file and register them.
     *
     * @param pszFile The data file, which also identifies the triggers in error messages.
     * @param pStream The stream to read instead of the file, or null.
     * @ghidraAddress NTSC-U/C: 0x001fe600
     * @ghidraAddress PAL: 0x002073a0
     */
    void Load(const char *pszFile, BinStream *pStream);

    /**
     * Register the triggers of an array, each with the event its node 0 names.
     *
     * @param pTriggers The triggers.
     * @ghidraAddress NTSC-U/C: 0x001fe650
     * @ghidraAddress PAL: 0x002073f0
     */
    void AddTriggers(DataArray *pTriggers);

    /**
     * Fire the triggers that wait on a note of the background music.
     *
     * @param chNote The note, a letter from A through G.
     * @ghidraAddress NTSC-U/C: 0x001fe780
     * @ghidraAddress PAL: 0x00207520
     */
    void BeatEvent(char chNote);

    /**
     * Advance the clocks and fire the triggers that wait on the time.
     *
     * A new random value is stored for RandomCondition. The metagame passes its clock twice and
     * 0.
     *
     * @param flStart The time of the game clock.
     * @param flTime The time of the real-time clock.
     * @param flValue The distance DistanceCondition tests.
     * @ghidraAddress NTSC-U/C: 0x001feb88
     * @ghidraAddress PAL: 0x00207928
     */
    void MetagameEvent(float flStart, float flTime, float flValue);

    /**
     * Fire the triggers that wait on a component being chosen.
     *
     * The names of the component, its panel, and its screen are stored for the conditions to
     * test.
     *
     * @param pMsg The message that reported the choice.
     * @ghidraAddress NTSC-U/C: 0x001fe7d8
     * @ghidraAddress PAL: 0x00207578
     */
    void ComponentSelectEvent(UIComponentSelectMsg *pMsg);

    /**
     * Fire the triggers that wait on the choice of a component beginning.
     *
     * @param pMsg The message that reported the choice.
     * @ghidraAddress NTSC-U/C: 0x001fe888
     * @ghidraAddress PAL: 0x00207628
     */
    void ComponentSelectStartEvent(UIComponentSelectStartMsg *pMsg);

    /**
     * Fire the triggers that wait on the focus moving between components.
     *
     * @param pMsg The message that reported the move.
     * @ghidraAddress NTSC-U/C: 0x001fe938
     * @ghidraAddress PAL: 0x002076d8
     */
    void ComponentFocusEvent(UIComponentFocusChangeMsg *pMsg);

    /**
     * Fire the triggers that wait on the focus arriving at a component, with no previous
     * component.
     *
     * @param pszComponent The component.
     * @param pszPanel The panel of the component.
     * @param pszScreen The screen of the panel.
     * @ghidraAddress NTSC-U/C: 0x001fea18
     * @ghidraAddress PAL: 0x002077b8
     */
    void ComponentFocusEvent(const char *pszComponent, const char *pszPanel, const char *pszScreen);

    /**
     * Fire the triggers that wait on a move between screens.
     *
     * @param pMsg The message that reported the move.
     * @ghidraAddress NTSC-U/C: 0x001feae0
     * @ghidraAddress PAL: 0x00207880
     */
    void ScreenChangeEvent(UIScreenChangeMsg *pMsg);

    /**
     * Fire the triggers that wait on a button.
     *
     * @param nPlayer The controller.
     * @param nButtons The buttons the controller holds.
     * @ghidraAddress NTSC-U/C: 0x001febc8
     * @ghidraAddress PAL: 0x00207968
     */
    void ButtonEvent(int nPlayer, int nButtons);

    /**
     * Fire the triggers that wait on the begin event.
     *
     * The metagame calls it once the triggers of the arena have loaded, and GfxManager once the
     * tracks are built. mGameState becomes 4, the state of a beginning game.
     *
     * @param nRuleSet The rule set RuleSetCondition tests.
     * @ghidraAddress NTSC-U/C: 0x001fec38
     * @ghidraAddress PAL: 0x002079d8
     */
    void BeginEvent(int nRuleSet);

    /**
     * Fire the triggers that wait on a player completing a stage.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001fec90
     * @ghidraAddress PAL: 0x00207a30
     */
    void StageCompleteEvent(int nPlayer);

    /**
     * Fire the triggers that wait on the end of the game.
     *
     * @param nState The game state GameStateCondition tests.
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001fece8
     * @ghidraAddress PAL: 0x00207a88
     */
    void EndEvent(int nState, int nPlayer);

    /**
     * Fire the triggers that wait on a new score.
     *
     * @param nPlayer The player.
     * @param nScore The score of the player.
     * @ghidraAddress NTSC-U/C: 0x001fed58
     * @ghidraAddress PAL: 0x00207af8
     */
    void ScoreEvent(int nPlayer, int nScore);

    /**
     * Fire the triggers that wait on a player hitting a gem.
     *
     * @param nPlayer The player.
     * @param nGemPos The lane of the gem.
     * @ghidraAddress NTSC-U/C: 0x001fedd0
     * @ghidraAddress PAL: 0x00207b70
     */
    void HitEvent(int nPlayer, int nGemPos);

    /**
     * Fire the triggers that wait on a player missing a gem.
     *
     * @param nPlayer The player.
     * @param nGemPos The lane of the gem.
     * @ghidraAddress NTSC-U/C: 0x001fee40
     * @ghidraAddress PAL: 0x00207be0
     */
    void MissEvent(int nPlayer, int nGemPos);

    /**
     * Fire the triggers that wait on a gem of a captured track.
     *
     * Nothing fires unless the player who captured the track still plays it.
     *
     * @param nTrack The track.
     * @param nGemPos The lane of the gem.
     * @ghidraAddress NTSC-U/C: 0x001feeb8
     * @ghidraAddress PAL: 0x00207c58
     */
    void GemEvent(int nTrack, int nGemPos);

    /**
     * Fire the triggers that wait on the start of a bar.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x001fef50
     * @ghidraAddress PAL: 0x00207cf0
     */
    void NewBarEvent(int nBar);

    /**
     * Fire the triggers that wait on the end of a phrase.
     *
     * The track is no longer captured.
     *
     * @param nTrack The track the phrase ended on.
     * @ghidraAddress NTSC-U/C: 0x001fefa8
     * @ghidraAddress PAL: 0x00207d48
     */
    void PhraseEndEvent(int nTrack);

    /**
     * Fire the triggers that wait on a player capturing a phrase.
     *
     * The track of the player becomes captured by the player.
     *
     * @param nPlayer The player.
     * @param nStreak The streak of the player.
     * @ghidraAddress NTSC-U/C: 0x001ff010
     * @ghidraAddress PAL: 0x00207db0
     */
    void PhraseCaptureEvent(int nPlayer, int nStreak);

    /**
     * Fire the triggers that wait on a missed phrase.
     *
     * The streak of the player returns to 0.
     *
     * @param nPlayer The player that missed the phrase.
     * @ghidraAddress NTSC-U/C: 0x001ff0a8
     * @ghidraAddress PAL: 0x00207e48
     */
    void PhraseMissEvent(int nPlayer);

    /**
     * Fire the triggers that wait on a player moving to a new track.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param nInstrument The instrument of the track.
     * @param nMode The music mode of the track.
     * @ghidraAddress NTSC-U/C: 0x001ff110
     * @ghidraAddress PAL: 0x00207eb0
     */
    void NewTrackEvent(int nPlayer, int nTrack, int nInstrument, int nMode);

    /**
     * Fire the triggers that wait on a change of health.
     *
     * @param nPlayer The player.
     * @param nHealth The health of the player.
     * @ghidraAddress NTSC-U/C: 0x001ff1b8
     * @ghidraAddress PAL: 0x00207f58
     */
    void HealthEvent(int nPlayer, int nHealth);

    /**
     * Fire the triggers that wait on a player starting or stopping dying.
     *
     * Nothing fires when the state is unchanged.
     *
     * @param nPlayer The player.
     * @param nDying Non-zero while the player is dying.
     * @ghidraAddress NTSC-U/C: 0x001ff230
     * @ghidraAddress PAL: 0x00207fd0
     */
    void DyingEvent(int nPlayer, int nDying);

    /**
     * Fire the triggers that wait on the boss journey starting.
     *
     * @ghidraAddress NTSC-U/C: 0x001ff2b8
     * @ghidraAddress PAL: 0x00208058
     */
    void BossJourneyEvent();

    /**
     * Fire the triggers that wait on a lyric.
     *
     * The lyric is stored for ChangeLyricAction to show.
     *
     * @param pszLyric The lyric text.
     * @ghidraAddress NTSC-U/C: 0x001ff308
     * @ghidraAddress PAL: 0x002080a8
     */
    void LyricEvent(const char *pszLyric);

    /**
     * Fire the triggers that wait on a path being unlocked.
     *
     * @param nPath The path, printed only.
     * @ghidraAddress NTSC-U/C: 0x001ff368
     * @ghidraAddress PAL: 0x00208108
     */
    void PathUnlockedEvent(int nPath);

    // The conditions and the actions read the values below directly, and the image has no
    // accessor for them.

    std::vector<TriggerEvent> mEvents;   /*!< One event per TriggerEvent::Type. */
    std::list<TriggerAction *> mRunning; /*!< Actions Poll() polls until they are done. */
    /*!< Actions waiting on each clock, in the order of their times. */
    std::list<std::pair<TriggerAction *, float>> mScheduled[kNumClocks];
    char mNote;                   /*!< The note of the last beat event. */
    int mPlayer;                  /*!< The player of the last event that names one. */
    float mTime[kNumClocks];      /*!< The time of each clock. */
    int mClock;                   /*!< The clock of the action in progress. */
    float mDistance;              /*!< The distance of the last time event. */
    int mLeader;                  /*!< The leading player. The shipped build never sets it. */
    int mBar;                     /*!< The bar of the last new bar event. */
    int mPowerup;                 /*!< The powerup. The shipped build never sets it. */
    int mHealth[kNumPlayers];     /*!< The health of each player. */
    int mDying[kNumPlayers];      /*!< Non-zero for each dying player. */
    int mScore[kNumPlayers];      /*!< The score of each player. */
    int mTrack[kNumPlayers];      /*!< The track of each player. */
    int mStreak[kNumPlayers];     /*!< The phrase streak of each player. */
    int mInstrument[kNumPlayers]; /*!< The instrument of the track of each player. */
    int mMode[kNumPlayers];       /*!< The music mode of the track of each player. */
    int mButtons;                 /*!< The buttons of the last button event. */
    int mGemPos;                  /*!< The lane of the last hit, miss, or gem event. */
    String mComponent;            /*!< The component of the last component event. */
    String mOldComponent;         /*!< The component that lost the focus. */
    String mPanel;                /*!< The panel of the last component event. */
    String mOldPanel;             /*!< The previous panel. No event writes it. */
    String mScreen;               /*!< The screen of the last component or screen event. */
    String mOldScreen;            /*!< The screen of the last screen event left. */
    int mGameState;               /*!< The state of the game GameStateCondition tests. */
    int mRuleSet;                 /*!< The rule set of the last begin event. */
    String mLyric;                /*!< The lyric of the last lyric event. */
    int mCapturer[kNumTracks];    /*!< The player who captured each track, or kNoPlayer. */
    float mRandom;                /*!< A random value from 0 to 1, new for each time event. */
    std::vector<bool> mPaths;     /*!< Whether each path of the arena is unlocked. */
    int mDebug;                   /*!< One bit per TriggerEvent::Type to print debug lines for. */
};

/**
 * The trigger registry.
 *
 * @ghidraAddress NTSC-U/C: 0x0043b700
 */
extern TriggerMgr TheTriggerMgr;
