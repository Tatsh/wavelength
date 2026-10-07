#pragma once

#include <vector>

#include "game/catchtrackdata.h"
#include "game/dueltrack.h"
#include "game/gametrackselector.h"
#include "game/pitchtrack.h"
#include "game/player.h"
#include "game/playmap.h"
#include "game/song.h"
#include "game/worldlogic.h"
#include "met/metagame.h"
#include "msg/capturepacket.h"
#include "msg/editgempacket.h"
#include "msg/finalscorepacket.h"
#include "msg/gameendedmsg.h"
#include "msg/message.h"
#include "msg/playerabortedmsg.h"
#include "msg/playerupdatepacket.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Rules of the duel rule set.
 *
 * The RTTI records the class as deriving from WorldLogic. The object is 0x13c bytes. Two players
 * take turns. The pitcher lays down a phrase of mPhraseBars bars on a DuelTrack, and the catcher
 * then repeats it. A miss scores a point for the pitcher and a catch scores a point for the
 * catcher, and the roles change after every catch phase. The duel is decided once a player has nine
 * points, unless the score stands at nine to eight. Every routine name other than those of the
 * WorldLogic overrides is inferred.
 */
class DuelLogic : public WorldLogic {
public:
    /** Values of mState. */
    enum State {
        kStateNone = 0,     /*!< The constructor has not finished. */
        kStatePlaying = 1,  /*!< The song plays. */
        kStatePaused = 2,   /*!< The pause menu shows. */
        kStateDecided = 3,  /*!< A player won, and the result shows. */
        kStateEnding = 4,   /*!< The song fades out. */
        kStateFinished = 5, /*!< The duel ended. */
    };

    /** Values of mPhase and mCuePhase. */
    enum Phase {
        kPhasePitch = 0, /*!< The pitcher lays down a phrase. */
        kPhaseCatch = 1, /*!< The catcher repeats the phrase. */
        kPhaseBreak = 2, /*!< The song passes bars the duel skips, or no cue is due. */
    };

    /** The data the duel records for each bar of the song. */
    struct BarData {
        int mTrack; /*!< The song track the duel plays in the bar. */
    };

    /**
     * Construct the rules for a song.
     *
     * @param pSong The song the duel plays.
     * @ghidraAddress NTSC-U/C: 0x00105a58
     * @ghidraAddress PAL: 0x00107190
     */
    explicit DuelLogic(Song *pSong);

    /**
     * Release the rules.
     *
     * @ghidraAddress NTSC-U/C: 0x00107278
     * @ghidraAddress PAL: 0x001089b0
     */
    ~DuelLogic() override;

    /**
     * Act on a message sent to the rules.
     *
     * The packets and messages of an online duel go to the handlers below, and every other message
     * goes to WorldLogic::DispatchPriv().
     *
     * @param pMsg The message.
     * @return The result of the handler or of WorldLogic::DispatchPriv().
     * @ghidraAddress NTSC-U/C: 0x0010aa00
     * @ghidraAddress PAL: 0x0010c138
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the music, set the first roles, and schedule the commands of the duel.
     *
     * @ghidraAddress NTSC-U/C: 0x00107ad8
     * @ghidraAddress PAL: 0x00109210
     */
    void Start() override;

    /**
     * End the duel.
     *
     * @ghidraAddress NTSC-U/C: 0x00107e28
     */
    void Stop() override;

    /**
     * Report whether the duel ended.
     *
     * @return Whether the duel ended.
     * @ghidraAddress NTSC-U/C: 0x001081d8
     */
    bool IsFinished() const override;

    /**
     * Report whether a player abandoned the duel from a menu.
     *
     * @return Non-zero when the duel was abandoned.
     * @ghidraAddress NTSC-U/C: 0x001082e8
     */
    int HasQuit() const override;

    /**
     * Report that no restart was requested. The body returns zero.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x003347a0
     */
    int IsRestartRequested() const override {
        return 0;
    }

    /**
     * Report that no player freestyles. The body returns false.
     *
     * @param nPlayer The player.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x003347a8
     */
    bool IsFreestyling([[maybe_unused]] int nPlayer) override {
        return false;
    }

    /**
     * Report the song position in ticks.
     *
     * @return The tick of the song clock.
     * @ghidraAddress NTSC-U/C: 0x001082f0
     * @ghidraAddress PAL: 0x00109a28
     */
    int GetTick() override;

    /**
     * Report the song position on the song clock.
     *
     * @return The time of the song clock.
     * @ghidraAddress NTSC-U/C: 0x00108310
     * @ghidraAddress PAL: 0x00109a48
     */
    float GetTime() override;

    /**
     * Finish the duel once the song has faded out and the outro is done.
     *
     * @ghidraAddress NTSC-U/C: 0x00108330
     * @ghidraAddress PAL: 0x00109a68
     */
    void Poll() override;

    /**
     * Report the fraction of the song played.
     *
     * @return GetTick() over the length of the song in ticks.
     * @ghidraAddress NTSC-U/C: 0x00108c68
     * @ghidraAddress PAL: 0x0010a3a0
     */
    float GetProgress() override;

    /**
     * Ignore a rotation. The body is empty.
     *
     * @param event The rotation.
     * @ghidraAddress NTSC-U/C: 0x00108cc8
     */
    void HandleInput(const RotateEvent &event) override;

    /**
     * Hand a gem button to the player who pressed it while the song plays.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00108cd0
     * @ghidraAddress PAL: 0x0010a408
     */
    void HandleInput(const PlayNoteEvent &event) override;

    /**
     * Pause the duel while the song plays.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00108d20
     * @ghidraAddress PAL: 0x0010a458
     */
    void HandleInput(const BtnEvent<3> &event) override;

    /**
     * Ignore a button event of command 4. The body is empty.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00108d90
     */
    void HandleInput(const BtnEvent<4> &event) override;

    /**
     * Ignore a button event of command 5. The body is empty.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00108d98
     */
    void HandleInput(const BtnEvent<5> &event) override;

    /**
     * Ignore a section change. The body is empty.
     *
     * @param event The section change.
     * @ghidraAddress NTSC-U/C: 0x003347b0
     */
    void HandleInput([[maybe_unused]] const ChangeSectionEvent &event) override {
    }

    /**
     * Ignore a button event of command 9. The body is empty.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x003347b8
     */
    void HandleInput([[maybe_unused]] const BtnEvent<9> &event) override {
    }

    /**
     * Ignore a button event of command 10. The body is empty.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x003347c8
     */
    void HandleInput([[maybe_unused]] const BtnEvent<10> &event) override {
    }

    /**
     * Ignore a button event of command 8. The body is empty.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x003347c0
     */
    void HandleInput([[maybe_unused]] const BtnEvent<8> &event) override {
    }

    /**
     * Ignore a stick event of command 2. The body is empty.
     *
     * @param event The stick position.
     * @ghidraAddress NTSC-U/C: 0x00108da0
     */
    void HandleInput(const StickEvent<2> &event) override;

    /**
     * Ignore a stick event of command 6. The body is empty.
     *
     * @param event The stick position.
     * @ghidraAddress NTSC-U/C: 0x00108da8
     */
    void HandleInput(const StickEvent<6> &event) override;

    /**
     * Pause or resume the duel.
     *
     * Pausing opens the pause menu, which stops the song clock unless the duel is online.
     *
     * @param bPaused Whether the duel pauses.
     * @param nPad The controller that paused the duel, or -1.
     * @param nReason Non-zero when a disconnected controller paused the duel.
     * @ghidraAddress NTSC-U/C: 0x00107e38
     * @ghidraAddress PAL: 0x00109570
     */
    void SetPaused(bool bPaused, int nPad, int nReason) override;

    /**
     * Report whether the song plays.
     *
     * @return Whether mState is kStatePlaying.
     * @ghidraAddress NTSC-U/C: 0x003347d0
     */
    bool IsPlaying() const override {
        return mState == kStatePlaying;
    }

    /**
     * Report the instrument of the song track the duel plays in a bar.
     *
     * @param nBar The bar.
     * @return The instrument.
     * @ghidraAddress NTSC-U/C: 0x00107a98
     * @ghidraAddress PAL: 0x001091d0
     */
    int GetBarInstrument(int nBar);

    /**
     * Report the side of a player.
     *
     * @param nPlayer The player.
     * @return nPlayer.
     * @ghidraAddress NTSC-U/C: 0x001081d0
     * @ghidraAddress PAL: 0x00109908
     */
    int GetPlayerSide(int nPlayer) const;

    /**
     * Fade the song out and schedule the end of the duel.
     *
     * The outro of the display plays unless a player quit. The duel is ended at once from the
     * pause menu.
     *
     * @ghidraAddress NTSC-U/C: 0x001081e8
     * @ghidraAddress PAL: 0x00109920
     */
    void StartEnding();

    /**
     * Score a missed catch for the pitcher.
     *
     * Only the first miss or catch of a phrase counts.
     *
     * @param pTrack The track of the catcher.
     * @param pPlayer The player whose console reported the miss.
     * @param nBar The bar the catch phase started in.
     * @ghidraAddress NTSC-U/C: 0x00108f40
     * @ghidraAddress PAL: 0x0010a678
     */
    void OnCatchMissed(DuelTrack *pTrack, Player *pPlayer, int nBar);

    /**
     * Score a made catch for the catcher.
     *
     * @param pTrack The track of the catcher.
     * @param pPlayer The player whose console reported the catch.
     * @param bReact Whether the duel may play a sound that praises the catch.
     * @ghidraAddress NTSC-U/C: 0x00109100
     * @ghidraAddress PAL: 0x0010a838
     */
    void OnCatchMade(DuelTrack *pTrack, Player *pPlayer, bool bReact);

    /**
     * Move the playback of the catcher's track to the current song position.
     *
     * @ghidraAddress NTSC-U/C: 0x00109258
     * @ghidraAddress PAL: 0x0010a990
     */
    void RewindCatcherTrack();

private:
    /**
     * Report the vertical offset of a cue message on one side of the screen.
     *
     * @param bSecondSide Whether the message belongs to the second side.
     * @return 110 for the second side, -110 for the first.
     * @ghidraAddress NTSC-U/C: 0x001081b0
     * @ghidraAddress PAL: 0x001098e8
     */
    static float CueOffsetY(bool bSecondSide);

    /**
     * Act on the choice of the pause menu of a duel on one console.
     *
     * @param action The choice.
     * @param pUserData The duel.
     * @ghidraAddress NTSC-U/C: 0x00107fd0
     * @ghidraAddress PAL: 0x00109708
     */
    static void OnPauseDialog(Metagame::DialogAction action, void *pUserData);

    /**
     * Act on the choice of the pause menu of an online duel.
     *
     * @param action The choice.
     * @param pUserData The duel.
     * @ghidraAddress NTSC-U/C: 0x00108078
     * @ghidraAddress PAL: 0x001097b0
     */
    static void OnOnlinePauseDialog(Metagame::DialogAction action, void *pUserData);

    /**
     * Act on the choice of the dialog at the end of the duel.
     *
     * @param action The choice.
     * @param pUserData The duel.
     * @ghidraAddress NTSC-U/C: 0x00108150
     * @ghidraAddress PAL: 0x00109888
     */
    static void OnGameOverDialog(Metagame::DialogAction action, void *pUserData);

    /**
     * Withdraw the four commands that pace the duel, once.
     *
     * @ghidraAddress NTSC-U/C: 0x00107798
     * @ghidraAddress PAL: 0x00108ed0
     */
    void StopCommands();

    /**
     * Record the song track of every bar, cycling Song::mDuelTrackOrder by section.
     *
     * @ghidraAddress NTSC-U/C: 0x00107830
     * @ghidraAddress PAL: 0x00108f68
     */
    void BuildBarData();

    /**
     * Give the first phrase to the pitcher and show it on both tracks.
     *
     * @ghidraAddress NTSC-U/C: 0x001083b8
     * @ghidraAddress PAL: 0x00109af0
     */
    void StartFirstPhrase();

    /**
     * Set the tracks for the phrase that starts at a bar and show the phrase after it.
     *
     * @param nBar The bar the phrase starts at.
     * @param nPhase One of Phase, without kPhaseBreak.
     * @ghidraAddress NTSC-U/C: 0x001085b8
     * @ghidraAddress PAL: 0x00109cf0
     */
    void ShowNextPhrase(int nBar, int nPhase);

    /**
     * Drop the bars the song passed and move to the next phrase, once a phrase.
     *
     * mScrollCommand runs it.
     *
     * @ghidraAddress NTSC-U/C: 0x00108900
     * @ghidraAddress PAL: 0x0010a038
     */
    void OnScrollTick();

    /**
     * Report whether the duel skips a bar.
     *
     * @param nBar The bar.
     * @return Whether Song::mSkippedBars lists the bar.
     * @ghidraAddress NTSC-U/C: 0x00108a78
     * @ghidraAddress PAL: 0x0010a1b0
     */
    bool IsBarSkipped(int nBar) const;

    /**
     * Move to the next phase at a bar, changing the roles after a catch phase.
     *
     * @param nBar The bar.
     * @return The bar the next phase starts at.
     * @ghidraAddress NTSC-U/C: 0x00108ad8
     * @ghidraAddress PAL: 0x0010a210
     */
    int AdvancePhrase(int nBar);

    /**
     * Decide the duel when a player has won.
     *
     * @return Whether a player won.
     * @ghidraAddress NTSC-U/C: 0x00108db8
     * @ghidraAddress PAL: 0x0010a4f0
     */
    bool CheckForWinner();

    /**
     * Play a sound that regrets a missed catch.
     *
     * The sound plays when no gem of the catcher lies after the current tick but the last, and the
     * catcher has at least four gems.
     *
     * @ghidraAddress NTSC-U/C: 0x00108ea8
     * @ghidraAddress PAL: 0x0010a5e0
     */
    void PlayMissSound();

    /**
     * Change the song track at the start of a section, once a bar.
     *
     * mBarCommand runs it.
     *
     * @ghidraAddress NTSC-U/C: 0x00109278
     * @ghidraAddress PAL: 0x0010a9b0
     */
    void OnBarTick();

    /**
     * Start the next section at a bar.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x00109308
     * @ghidraAddress PAL: 0x0010aa40
     */
    void AdvanceSection(int nBar);

    /**
     * Show the name of the next phase one bar before it, once a bar.
     *
     * mPhraseCueCommand runs it.
     *
     * @ghidraAddress NTSC-U/C: 0x00109388
     * @ghidraAddress PAL: 0x0010aac0
     */
    void OnPhraseCueTick();

    /**
     * Play the coaching sounds one bar before a phase, once a bar.
     *
     * mSoundCueCommand runs it.
     *
     * @ghidraAddress NTSC-U/C: 0x001094b8
     * @ghidraAddress PAL: 0x0010abf0
     */
    void OnSoundCueTick();

    /**
     * Report the volume of a track.
     *
     * @param nTrack The track.
     * @param bMuted Whether the track plays behind the duel rather than in it.
     * @return The volume Song::GetTrackVolumes() lists, or 50 muted and 127 otherwise.
     * @ghidraAddress NTSC-U/C: 0x001096f8
     * @ghidraAddress PAL: 0x0010ae30
     */
    unsigned char GetTrackVolume(int nTrack, bool bMuted);

    /**
     * Make the song track of a bar the one the duel plays.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x00109768
     * @ghidraAddress PAL: 0x0010aea0
     */
    void ApplyBarTrack(int nBar);

    /**
     * End the play of the duel and show its result.
     *
     * @param bLocalWinner Whether the winner plays on this console.
     * @ghidraAddress NTSC-U/C: 0x00109930
     * @ghidraAddress PAL: 0x0010b068
     */
    void DecideDuel(bool bLocalWinner);

    /**
     * Announce the winner, or a tie.
     *
     * mAnnounceCommand runs it.
     *
     * @ghidraAddress NTSC-U/C: 0x00109cf0
     * @ghidraAddress PAL: 0x0010b428
     */
    void AnnounceWinner();

    /**
     * Report whether every player still in the session reported a final score.
     *
     * @return Whether the scores are complete.
     * @ghidraAddress NTSC-U/C: 0x00109d50
     * @ghidraAddress PAL: 0x0010b488
     */
    bool AllScoresReceived() const;

    /**
     * Report the final scores to the session as the hosting console.
     *
     * A player who quit scores 0.
     *
     * @ghidraAddress NTSC-U/C: 0x00109ed0
     * @ghidraAddress PAL: 0x0010b608
     */
    void ReportScores();

    /**
     * Log the end of the duel with the scores of the players.
     *
     * @ghidraAddress NTSC-U/C: 0x0010a1e0
     * @ghidraAddress PAL: 0x0010b918
     */
    void LogEndMultiGame();

    /**
     * Take the catching state of a remote player from a newer update.
     *
     * @param pPacket The update.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0010a328
     * @ghidraAddress PAL: 0x0010ba60
     */
    int OnPlayerUpdate(PlayerUpdatePacket *pPacket);

    /**
     * Add a gem the remote pitcher laid down.
     *
     * A gem less than 960 ticks ahead of the song clock is dropped.
     *
     * @param pPacket The gem.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0010a3a8
     * @ghidraAddress PAL: 0x0010bae0
     */
    int OnEditGem(EditGemPacket *pPacket);

    /**
     * End a catch the way the console of the catcher reported it.
     *
     * @param pPacket The report.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0010a550
     * @ghidraAddress PAL: 0x0010bc88
     */
    int OnCapture(CapturePacket *pPacket);

    /**
     * Record that a player reported a final score, and report the scores once all are in.
     *
     * @param pPacket The report.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0010a5b0
     * @ghidraAddress PAL: 0x0010bce8
     */
    int OnFinalScore(FinalScorePacket *pPacket);

    /**
     * Act on the end of the online session.
     *
     * @param pMsg The message.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0010a6d8
     * @ghidraAddress PAL: 0x0010be10
     */
    int OnGameEnded(GameEndedMsg *pMsg);

    /**
     * Take a player who quit the session out of the duel and end it.
     *
     * @param pMsg The message.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0010a8d0
     * @ghidraAddress PAL: 0x0010c008
     */
    int OnPlayerAborted(PlayerAbortedMsg *pMsg);

    Player *mPitcher;                          /*!< The player who lays down the phrase. */
    Player *mCatcher;                          /*!< The player who repeats the phrase. */
    int mPhase;                                /*!< One of Phase. */
    int mCuePhase;                             /*!< The Phase OnPhraseCueTick() announces. */
    int mCuePlayer;                            /*!< The index in mPlayers of the cued player. */
    int mNextPhraseBar;                        /*!< The bar the next phrase starts at. */
    int mState;                                /*!< One of State. */
    int mStateBeforePause;                     /*!< mState when the duel paused. */
    int mPaused;                               /*!< Whether the duel is paused. */
    int mQuit;                                 /*!< Whether a player quit from a menu. */
    int mEndTick;                              /*!< The tick the faded song ends at. */
    int mCommandsStopped;                      /*!< Whether StopCommands() ran. */
    DuelTrack *mPitcherTrack;                  /*!< The track of mPitcher. */
    DuelTrack *mCatcherTrack;                  /*!< The track of mCatcher. */
    CatchTrackData *mPitcherGems;              /*!< The gems mPitcher lays down. */
    CatchTrackData *mCatcherGems;              /*!< The gems mCatcher catches. */
    int mCatchMissed;                          /*!< Whether the catch of this phrase was missed. */
    int mCatchMade;                            /*!< Whether the catch of this phrase was made. */
    Song *mSong;                               /*!< The song. */
    int mTicksPerBar;                          /*!< The song ticks in one bar. */
    int mPhraseBars;                           /*!< The length of a phrase in bars. */
    PlayMap *mPlayMap;                         /*!< The map of the song positions. */
    int mReserved5c;                           // +0x5c, cleared by the constructor and not read.
    int mSectionStartBar;                      /*!< The bar the current section started at. */
    int mNextSectionBar;                       /*!< The bar the next section starts at. */
    int mSection;                              /*!< The number of sections started. */
    std::vector<Player *> mPlayers;            /*!< The players, one for each side. */
    std::vector<DuelTrack *> mTracks;          /*!< The track of each side. */
    std::vector<CatchTrackData *> mGems;       /*!< The gems of each side. */
    GameTrackSelector *mTrackSelector;         /*!< The assignment of the players to mTracks. */
    std::vector<int> mInputPlayers;            /*!< The index in mPlayers of each input player. */
    std::vector<PitchTrack *> mPitchTracks;    /*!< The song tracks that play behind the duel. */
    Ptr<Command> mBarCommand;                  /*!< The command that calls OnBarTick(). */
    Ptr<Command> mScrollCommand;               /*!< The command that calls OnScrollTick(). */
    Ptr<Command> mPhraseCueCommand;            /*!< The command that calls OnPhraseCueTick(). */
    Ptr<Command> mSoundCueCommand;             /*!< The command that calls OnSoundCueTick(). */
    Ptr<Command> mAnnounceCommand;             /*!< The command that calls AnnounceWinner(). */
    int mSlopTicks;                            /*!< GameConfig::mSlopMs in song ticks. */
    int mCatchMadeTick;                        /*!< The tick of the last catch made. */
    Player *mLocalPlayer;                      /*!< The player on this console, online. */
    Player *mRemotePlayer;                     /*!< The player on the other console, online. */
    std::vector<unsigned int> mUpdateVersions; /*!< The newest update of each player. */
    std::vector<int> mSeatPlayers;             /*!< The index in mPlayers of each seat, or -1. */
    std::vector<bool> mScoresReceived;         /*!< Whether each player reported a final score. */
    std::vector<BarData> mBarData;             /*!< The data of each bar of the song. */
    int mCuesPlayed;                           /*!< The phases whose coaching sound was played. */
    int mOneLetterPlayed;                      /*!< Whether `DUEL_ONELETTER` was played. */
    int mMissThisPlayed;                       /*!< Whether `DUEL_MISSTHIS` was played. */
};
