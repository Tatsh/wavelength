#pragma once

#include <vector>

#include "game/gamelogic.h"
#include "game/netarbiter.h"
#include "met/metagame.h"
#include "msg/arbiterpacket.h"
#include "msg/bumperpacket.h"
#include "msg/capturepacket.h"
#include "msg/cripplerpacket.h"
#include "msg/finalscorepacket.h"
#include "msg/freestylebumppacket.h"
#include "msg/freestylepacket.h"
#include "msg/gameendedmsg.h"
#include "msg/multiplierpacket.h"
#include "msg/playerabortedmsg.h"
#include "msg/playerupdatepacket.h"
#include "msg/slowdownpacket.h"
#include "os/command.h"
#include "os/ptr.h"
#include "os/scheduler.h"
#include "script/dataarray.h"

/**
 * Rules of a song played by several players, on one console or online.
 *
 * The RTTI records the class as deriving from GameLogic. The logic tracks the score leader, lets
 * the bumper and crippler power-ups strike the other players on the same track, and ends the song
 * with the results of every player. Online, every action of a local player is sent to the other
 * consoles as a packet, every packet received is applied to the remote player it names, and the
 * hosting console runs a NetArbiter and collects the final scores.
 */
class MultiGameLogic : public GameLogic {
public:
    /**
     * Construct the logic of a song and register the `win_cheat` script command.
     *
     * Online, the hosting console builds the arbiter, and the players are mapped from their
     * session order.
     *
     * @param pSong The song.
     * @param pConfig The configuration of the logic.
     * @param nSeed The seed of the random numbers the logic draws.
     * @ghidraAddress NTSC-U/C: 0x00124760
     * @ghidraAddress PAL: 0x00125ee0
     */
    MultiGameLogic(Song *pSong, DataArray *pConfig, int nSeed);

    /**
     * Release the arbiter, restore the song speed, and unregister the cheat.
     *
     * @ghidraAddress NTSC-U/C: 0x00124b58
     * @ghidraAddress PAL: 0x001262d8
     */
    ~MultiGameLogic() override;

    /**
     * Apply a packet from another console, or hand any other message to WorldLogic.
     *
     * @param pMsg The message.
     * @return False for a message the logic handles here, otherwise the result of
     *         WorldLogic::DispatchPriv().
     * @ghidraAddress NTSC-U/C: 0x00127748
     * @ghidraAddress PAL: 0x00128f58
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report the tick of the scheduler's clock.
     *
     * @return The tick.
     * @ghidraAddress NTSC-U/C: 0x00125210
     * @ghidraAddress PAL: 0x00126990
     */
    int GetTick() override;

    /**
     * Report the time of the scheduler's clock.
     *
     * @return The time.
     * @ghidraAddress NTSC-U/C: 0x00125230
     * @ghidraAddress PAL: 0x001269b0
     */
    float GetTime() override;

    /**
     * Move a player to a neighbouring track and ask the arbiter to settle the move.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00125250
     * @ghidraAddress PAL: 0x001269d0
     */
    void HandleInput(const RotateEvent &event) override;

    /**
     * Pause or resume, and show the pause dialog when pausing.
     *
     * Only a playing song pauses, and a request that does not change the state is ignored.
     *
     * @param bPaused Whether to pause.
     * @param nPad The pad that requested the change.
     * @param nReason Non-zero for a disconnected controller.
     * @ghidraAddress NTSC-U/C: 0x00124e10
     * @ghidraAddress PAL: 0x00126590
     */
    void SetPaused(bool bPaused, int nPad, int nReason) override;

    /**
     * Slow the song down for a player and, for a local player online, send a SlowdownPacket.
     *
     * @param pPlayer The player.
     * @return Whether the power-up was deployed.
     * @ghidraAddress NTSC-U/C: 0x00125a50
     * @ghidraAddress PAL: 0x00127260
     */
    bool DeploySlowdown(Player *pPlayer) override;

    /**
     * Move a player to the freestyle track.
     *
     * Offline, a player already on the freestyle track first moves to a random catch track.
     *
     * @param pPlayer The player.
     * @return true.
     * @ghidraAddress NTSC-U/C: 0x00125b00
     * @ghidraAddress PAL: 0x00127310
     */
    bool DeployFreestyle(Player *pPlayer) override;

    /**
     * Bump every other player on a player's track to a random other track.
     *
     * @param pPlayer The player.
     * @return Whether a player was bumped.
     * @ghidraAddress NTSC-U/C: 0x00125c18
     * @ghidraAddress PAL: 0x00127428
     */
    bool DeployBumper(Player *pPlayer) override;

    /**
     * Cripple every other player on a player's track.
     *
     * @param pPlayer The player.
     * @return Whether a player was crippled.
     * @ghidraAddress NTSC-U/C: 0x001260a8
     * @ghidraAddress PAL: 0x001278b8
     */
    bool DeployCrippler(Player *pPlayer) override;

    /**
     * Move a player from the freestyle track back to a catch track.
     *
     * @param pPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x00125ba8
     * @ghidraAddress PAL: 0x001273b8
     */
    void EndFreestyle(Player *pPlayer) override;

    /**
     * Enable the catch tracks, schedule the end of the song, and start the arbiter.
     *
     * @ghidraAddress NTSC-U/C: 0x00124d40
     * @ghidraAddress PAL: 0x001264c0
     */
    void OnStart() override;

    /**
     * Celebrate a won song and collect or send the final score.
     *
     * @param bWon Whether the song was won.
     * @ghidraAddress NTSC-U/C: 0x001256b0
     * @ghidraAddress PAL: 0x00126ec0
     */
    void OnFinish(bool bWon) override;

    /**
     * Update the score leader.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x00124fa0
     * @ghidraAddress PAL: 0x00126720
     */
    void OnBar(int nBar) override;

    /**
     * Announce the completed stage while sections remain.
     *
     * @ghidraAddress NTSC-U/C: 0x00124fc0
     * @ghidraAddress PAL: 0x00126740
     */
    void OnSection() override;

    /**
     * Update the score leader and, for a local player online, send a CapturePacket.
     *
     * @param pTrack The track the phrase was captured on.
     * @param pPlayer The player that captured it.
     * @param nBar Unused.
     * @param bAutocatch Whether an autocatcher captured the phrase.
     * @ghidraAddress NTSC-U/C: 0x001254d8
     * @ghidraAddress PAL: 0x00126c58
     */
    void OnPhraseCaptured(CatchTrack *pTrack, Player *pPlayer, int nBar, bool bAutocatch) override;

    /**
     * Act once a phrase of a track ended. The body is empty.
     *
     * @param pTrack The track.
     * @ghidraAddress NTSC-U/C: 0x001255c8
     */
    void OnPhraseEnded(Track *pTrack) override;

    /**
     * Act once a player missed a phrase. The body is empty.
     *
     * @param pTrack The track.
     * @ghidraAddress NTSC-U/C: 0x0033c0d0
     */
    void OnPhraseMissed([[maybe_unused]] Track *pTrack) override {
    }

private:
    /**
     * Report whether the song has passed its last bar, after which packets are ignored.
     *
     * Inline. The packet handlers expand it.
     *
     * @return Whether the current bar is at or after the end of the song.
     */
    bool IsPastEnd() const {
        return TheSongScheduler.mTick / mTicksPerBar >= mNumBars;
    }

    /**
     * Show the online pause dialog.
     *
     * @param bPaused Whether the song paused.
     * @param nPad The pad that requested the change.
     * @param nReason Non-zero for a disconnected controller.
     * @ghidraAddress NTSC-U/C: 0x00124ec8
     * @ghidraAddress PAL: 0x00126648
     */
    void PauseOnline(bool bPaused, int nPad, int nReason);

    /**
     * Stop or restart the clock, and show the pause dialog when pausing.
     *
     * @param bPaused Whether the song paused.
     * @param nPad The pad that requested the change.
     * @param nReason Non-zero for a disconnected controller.
     * @ghidraAddress NTSC-U/C: 0x00124f10
     * @ghidraAddress PAL: 0x00126690
     */
    void PauseLocal(bool bPaused, int nPad, int nReason);

    /**
     * Act on the choice of the pause dialog of a song on one console.
     *
     * @param action The choice.
     * @param pUserData The MultiGameLogic.
     * @ghidraAddress NTSC-U/C: 0x00125098
     * @ghidraAddress PAL: 0x00126818
     */
    static void OnLocalPauseDialog(Metagame::DialogAction action, void *pUserData);

    /**
     * Act on the choice of the pause dialog of an online song.
     *
     * @param action The choice.
     * @param pUserData The MultiGameLogic.
     * @ghidraAddress NTSC-U/C: 0x00125138
     * @ghidraAddress PAL: 0x001268b8
     */
    static void OnOnlinePauseDialog(Metagame::DialogAction action, void *pUserData);

    /**
     * Play the sound of the score leader, when the game has one and several players.
     *
     * @ghidraAddress NTSC-U/C: 0x001252b0
     * @ghidraAddress PAL: 0x00126a30
     */
    void PlayLeaderSound();

    /**
     * Find the player with the best score and mark the leader on the display.
     *
     * A tie for the best score leaves no leader. A change of leader plays the leader sound, 1.5
     * seconds later when a leader sound is still playing.
     *
     * @ghidraAddress NTSC-U/C: 0x00125310
     * @ghidraAddress PAL: 0x00126a90
     */
    void UpdateLeader();

    /**
     * Act on the choice of the dialog at the end of the song.
     *
     * @param action The choice.
     * @param pUserData The MultiGameLogic.
     * @ghidraAddress NTSC-U/C: 0x001255d0
     * @ghidraAddress PAL: 0x00126d50
     */
    static void OnEndGameDialog(Metagame::DialogAction action, void *pUserData);

    /**
     * Win the song, the `win_cheat` script command.
     *
     * @param pCommand The command.
     * @param pUserData The MultiGameLogic.
     * @ghidraAddress NTSC-U/C: 0x00125630
     * @ghidraAddress PAL: 0x00126db0
     */
    static void WinCheat(DataArray *pCommand, void *pUserData);

    /**
     * Give the second player 400 points and win the song.
     *
     * @ghidraAddress NTSC-U/C: 0x00125650
     */
    void ApplyWinCheat();

    /**
     * Log the scores, announce the winner or the tie, and schedule the end-of-game dialog.
     *
     * @ghidraAddress NTSC-U/C: 0x00125870
     * @ghidraAddress PAL: 0x00127080
     */
    void ShowResults();

    /**
     * Report the player of a session order.
     *
     * @param nNetOrder The session order.
     * @return The player's index.
     * @ghidraAddress NTSC-U/C: 0x00125a38
     * @ghidraAddress PAL: 0x00127248
     */
    int GetPlayerFromNetOrder(int nNetOrder);

    /**
     * Report whether every player that has not left the song has finished it.
     *
     * @return Whether every remaining player finished.
     * @ghidraAddress NTSC-U/C: 0x00126280
     * @ghidraAddress PAL: 0x00127a90
     */
    bool AllPlayersFinished();

    /**
     * Report the final scores to the session server and show the results.
     *
     * A player that left the song scores zero.
     *
     * @ghidraAddress NTSC-U/C: 0x00126400
     * @ghidraAddress PAL: 0x00127c10
     */
    void SendResults();

    /**
     * Log the score of each of the four players.
     *
     * @ghidraAddress NTSC-U/C: 0x001266f0
     * @ghidraAddress PAL: 0x00127f00
     */
    void LogResults();

    /**
     * Apply a remote player's score, track, and catching state.
     *
     * @param pPacket The packet.
     * @ghidraAddress NTSC-U/C: 0x00126838
     * @ghidraAddress PAL: 0x00128048
     */
    void OnPlayerUpdate(PlayerUpdatePacket *pPacket);

    /**
     * Apply the arbiter's assignment of the players to the tracks.
     *
     * @param pPacket The packet.
     * @ghidraAddress NTSC-U/C: 0x00126988
     * @ghidraAddress PAL: 0x00128198
     */
    void OnArbiter(ArbiterPacket *pPacket);

    /**
     * Apply a phrase a remote player captured.
     *
     * @param pPacket The packet.
     * @ghidraAddress NTSC-U/C: 0x00126b40
     * @ghidraAddress PAL: 0x00128350
     */
    void OnCapture(CapturePacket *pPacket);

    /**
     * Apply a remote player's bumper.
     *
     * @param pPacket The packet.
     * @ghidraAddress NTSC-U/C: 0x00126c50
     * @ghidraAddress PAL: 0x00128460
     */
    void OnBumper(BumperPacket *pPacket);

    /**
     * Apply a remote player's crippler.
     *
     * @param pPacket The packet.
     * @ghidraAddress NTSC-U/C: 0x00126de8
     * @ghidraAddress PAL: 0x001285f8
     */
    void OnCrippler(CripplerPacket *pPacket);

    /**
     * Apply a remote player's slowdown.
     *
     * @param pPacket The packet.
     * @ghidraAddress NTSC-U/C: 0x00126ed0
     * @ghidraAddress PAL: 0x001286e0
     */
    void OnSlowdown(SlowdownPacket *pPacket);

    /**
     * Apply a remote player's multiplier.
     *
     * @param pPacket The packet.
     * @ghidraAddress NTSC-U/C: 0x00126f58
     * @ghidraAddress PAL: 0x00128768
     */
    void OnMultiplier(MultiplierPacket *pPacket);

    /**
     * Apply a remote player's input on the freestyle track.
     *
     * @param pPacket The packet.
     * @ghidraAddress NTSC-U/C: 0x00126ff0
     * @ghidraAddress PAL: 0x00128800
     */
    void OnFreestyle(FreestylePacket *pPacket);

    /**
     * Move a player the arbiter bumped off the freestyle track.
     *
     * @param pPacket The packet.
     * @ghidraAddress NTSC-U/C: 0x00127158
     * @ghidraAddress PAL: 0x00128968
     */
    void OnFreestyleBump(FreestyleBumpPacket *pPacket);

    /**
     * Record a remote player's final score and send the results once every player finished.
     *
     * @param pPacket The packet.
     * @ghidraAddress NTSC-U/C: 0x00127208
     * @ghidraAddress PAL: 0x00128a18
     */
    void OnFinalScore(FinalScorePacket *pPacket);

    /**
     * Record the final scores of the session and end the song.
     *
     * @param pMsg The message.
     * @ghidraAddress NTSC-U/C: 0x00127370
     * @ghidraAddress PAL: 0x00128b80
     */
    void OnGameEnded(GameEndedMsg *pMsg);

    /**
     * Take a player who left the session out of the song.
     *
     * @param pMsg The message.
     * @ghidraAddress NTSC-U/C: 0x00127600
     * @ghidraAddress PAL: 0x00128e10
     */
    void OnPlayerAborted(PlayerAbortedMsg *pMsg);

    Player *mLocalPlayer;            /*!< The player on this console, online. */
    int mLeader;                     /*!< The index of the score leader, or -1. */
    Ptr<Command> mFinishCommand;     /*!< The command that calls Finish() at the song end. */
    Ptr<Command> mLeaderCommand;     /*!< The command that calls PlayLeaderSound(). */
    std::vector<unsigned> mVersions; /*!< The last update sequence number of each player. */
    unsigned mArbiterVersion;        /*!< The sequence number of the last arbiter packet. */
    std::vector<int> mNetToPlayer;   /*!< The player of each session order. */
    NetArbiter *mArbiter;            /*!< The arbiter, on the hosting console. */
    std::vector<bool> mFinished;     /*!< Whether each player finished the song. */
};
