#pragma once

#include <list>
#include <vector>

#include <librtdme.h>

#include "app/msgsink.h"
#include "game/netgamescore.h"
#include "netflow/netlaunchpad.h"
#include "netflow/netlaunchpadplayer.h"
#include "netflow/netmultitask.h"
#include "netflow/nettask.h"
#include "netflow/periodictask.h"
#include "netflow/ratingstat.h"
#include "netflow/tasklist.h"

class ClientAbortedPkt;
class ClientRatingPkt;
class ClientStatusPkt;
class LoadGamePkt;
class Message;
class Packet;
class StartGamePkt;
class UpdateRanksPkt;

/**
 * The online session of this console on top of the session library, the base of HostpadRT and
 * JoinpadRT.
 *
 * The RTTI includes the class name and records MsgSink and NetLaunchpad as the bases. The
 * NetLaunchpad part is at offset 0 with the vtable at `0x003d4770`, and the MsgSink part is at
 * offset 4 with the vtable at `0x003d4748`. The object is 0x17c bytes. The constructor stores the
 * instance in TheNetLaunchpad and TheLaunchpadRT.
 *
 * The session runs three requests. mConnectTask connects to the session, mExitTask leaves it, and
 * mRankTask asks for the ladder position of the player after a game. The players are listed in the
 * order of their colours.
 */
class LaunchpadRT : public NetLaunchpad, public MsgSink {
public:
    /** Values of mState. */
    enum State {
        kStateFailed = -1,    /*!< The host turned the console away, and mError tells why. */
        kStateConnecting = 0, /*!< The console has no identifier in the session yet. */
        kStateConnected = 1,  /*!< The console has an identifier in the session. */
        kStateWelcomed = 2,   /*!< The host accepted the console. */
        kStateSynced = 3,     /*!< The console is ready to play. */
    };

    /** Values of NetLaunchpadPlayer::mReady. */
    enum Ready {
        kReadyNo = 0,      /*!< The player is not ready. */
        kReadyYes = 1,     /*!< The player is ready. */
        kReadyLoading = 2, /*!< The player loads the song. */
        kReadyLoaded = 3,  /*!< The player loaded the song. */
    };

    /** The identifier of the host in the session. */
    static constexpr int kHostId = 0;

    /** The identifier of a console that has none yet. */
    static constexpr int kNoId = -2;

    /** The value of mWorldId and mConnection without a session. */
    static constexpr int kNoWorld = -1;

    /** The chat channel the session exchanges its key on. */
    static constexpr int kKeyChannel = 4;

    /** The size of the key of the session in bytes. */
    static constexpr int kKeySize = 16;

    /** The consoles a session takes on the session library and the game server. */
    static constexpr int kMaxClients = 5;

    /** The kind of host the session reports to the servers, a host among its peers. */
    static constexpr int kGameHostPeerToPeer = 2;

    /**
     * Construct the session, stored in TheNetLaunchpad and TheLaunchpadRT.
     *
     * The constructor gives the player report its task, registers the session for the chat of
     * kKeyChannel, and adds the ladder position request to mRankTask.
     *
     * @param pGame The sink that receives the messages of the session.
     * @ghidraAddress NTSC-U/C: 0x00254e08
     * @ghidraAddress PAL: 0x0025dd20
     */
    explicit LaunchpadRT(MsgSink *pGame);

    /**
     * Stop the requests, and clear TheNetLaunchpad and TheLaunchpadRT.
     *
     * @ghidraAddress NTSC-U/C: 0x00255068
     * @ghidraAddress PAL: 0x0025df80
     */
    ~LaunchpadRT() override;

    /**
     * Exit the session with reason 0.
     *
     * @param pSink The sink that receives the end of the session.
     * @ghidraAddress NTSC-U/C: 0x002551e8
     * @ghidraAddress PAL: 0x0025e100
     */
    void Cancel(MsgSink *pSink) override;

    /**
     * Mark the session active, and report this console ready when it can be.
     *
     * @ghidraAddress NTSC-U/C: 0x00255208
     * @ghidraAddress PAL: 0x0025e120
     */
    void Activate() override;

    /**
     * Report this console loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00255228
     * @ghidraAddress PAL: 0x0025e140
     */
    void NotifyLoaded() override;

    /**
     * Report mLocalId.
     *
     * @return The identifier of this console in the session.
     * @ghidraAddress NTSC-U/C: 0x00255258
     * @ghidraAddress PAL: 0x0025e170
     */
    int IsGuest() override;

    /**
     * Report sPlayersVersion.
     *
     * @return The count.
     * @ghidraAddress NTSC-U/C: 0x00255260
     * @ghidraAddress PAL: 0x0025e178
     */
    int GetPlayersVersion() override;

    /**
     * Report mPlayers.
     *
     * Inline.
     *
     * @return The players.
     * @ghidraAddress NTSC-U/C: 0x00398d30
     * @ghidraAddress PAL: 0x004075b8
     */
    std::list<NetLaunchpadPlayer> *GetPlayers() override {
        return &mPlayers;
    }

    /**
     * Vtable slot 12. Send the player report at the next poll.
     *
     * The base does so only in a session. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00255770
     * @ghidraAddress PAL: 0x0025e688
     */
    virtual void ReportNow();

    /**
     * Vtable slot 13. Take the identifier of this console in the session.
     *
     * The name is inferred.
     *
     * @param nId The identifier.
     * @ghidraAddress NTSC-U/C: 0x002557a0
     * @ghidraAddress PAL: 0x0025e6b8
     */
    virtual void SetLocalId(int nId);

    /**
     * Vtable slot 14. Note a console that connected to the session.
     *
     * The base sends the console the player of this console once this console is welcomed. The
     * name is inferred.
     *
     * @param nId The identifier of the console.
     * @ghidraAddress NTSC-U/C: 0x002557b0
     * @ghidraAddress PAL: 0x0025e6c8
     */
    virtual void AddClient(int nId);

    /**
     * Vtable slot 15. Remove a player who left, and drop the player from the rating.
     *
     * @param nId The player's identifier in the session.
     * @ghidraAddress NTSC-U/C: 0x002558f8
     * @ghidraAddress PAL: 0x0025e810
     */
    virtual void RemovePlayer(int nId);

    /**
     * Vtable slot 16. Report the clock of the session.
     *
     * The name is inferred.
     *
     * @return The time in milliseconds.
     */
    virtual float GetTime() = 0;

    /**
     * Vtable slot 17. Report the success of mConnectTask to its sink.
     *
     * The name is inferred.
     */
    virtual void OnConnected() = 0;

    /**
     * Vtable slot 18. Service the derived class once per Poll().
     *
     * The name is inferred.
     */
    virtual void PollPriv() = 0;

    /**
     * Vtable slot 19. Leave the game of the session and return to the launchpad.
     *
     * The base closes the transport, clears the rating, marks this console not ready, and starts
     * mRankTask. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002566c0
     * @ghidraAddress PAL: 0x0025f5c8
     */
    virtual void EndGame();

    /**
     * Vtable slot 20. Send the rating of this console to the other consoles.
     *
     * The name is inferred.
     *
     * @param nRank The position of the player in the ranking.
     * @ghidraAddress NTSC-U/C: 0x002552f8
     * @ghidraAddress PAL: 0x0025e210
     */
    virtual void SendRating(int nRank);

    /**
     * Act on a packet of the session.
     *
     * @param pMsg The message.
     * @return The result of the handler, false for every handler here.
     * @ghidraAddress NTSC-U/C: 0x00256ac8
     * @ghidraAddress PAL: 0x0025f9d0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Set the state of this console and send it to every console.
     *
     * The name is inferred.
     *
     * @param nReady One of Ready.
     * @ghidraAddress NTSC-U/C: 0x00255270
     * @ghidraAddress PAL: 0x0025e188
     */
    void SetReady(int nReady);

    /**
     * Service the session once per frame.
     *
     * The routine polls the requests, sends the player of this console to the consoles that
     * connected, and starts the song once its time comes.
     *
     * @return 1 once mExitTask finished and the session should be deleted, otherwise 0.
     * @ghidraAddress NTSC-U/C: 0x002553a0
     * @ghidraAddress PAL: 0x0025e2b8
     */
    int Poll();

    /**
     * Find a player of the session.
     *
     * @param nId The player's identifier in the session.
     * @return The player, or null.
     * @ghidraAddress NTSC-U/C: 0x002559f0
     * @ghidraAddress PAL: 0x0025e908
     */
    NetLaunchpadPlayer *FindPlayer(int nId);

    /**
     * Find the player of this console.
     *
     * @return The player, or null.
     * @ghidraAddress NTSC-U/C: 0x00255a40
     * @ghidraAddress PAL: 0x0025e958
     */
    NetLaunchpadPlayer *GetLocalPlayer();

    /**
     * Insert a player before the first player with a higher colour.
     *
     * @param player The player.
     * @ghidraAddress NTSC-U/C: 0x00255a60
     * @ghidraAddress PAL: 0x0025e978
     */
    void AddPlayer(const NetLaunchpadPlayer &player);

    /**
     * Start the song, and rate the start of a rated game as a loss of this console.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00255c58
     * @ghidraAddress PAL: 0x0025eb70
     */
    void StartGame();

    /**
     * Rate a player who left as a loser, and drop every guest when the host left.
     *
     * The name is inferred.
     *
     * @param nId The player's identifier in the session.
     * @ghidraAddress NTSC-U/C: 0x00255ec0
     * @ghidraAddress PAL: 0x0025edd8
     */
    void DropRating(int nId);

    /**
     * Rate the players from the final scores of the game, and clear the rating.
     *
     * The name is inferred.
     *
     * @param scores The final score of each player.
     * @ghidraAddress NTSC-U/C: 0x00256010
     * @ghidraAddress PAL: 0x0025ef28
     */
    void RateGame(const std::vector<NetGameScore> &scores);

    /**
     * Rate the player of this console from mRatings and publish the rating.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00256120
     * @ghidraAddress PAL: 0x0025f038
     */
    void UpdateRating();

    /**
     * End the game and start mExitTask.
     *
     * The name is inferred.
     *
     * @param pSink The sink that receives the end of the session.
     * @param nReason A LaunchpadAbortedMsg::Reason, or 0.
     * @ghidraAddress NTSC-U/C: 0x00256178
     * @ghidraAddress PAL: 0x0025f090
     */
    void Exit(MsgSink *pSink, int nReason);

    /**
     * Find the identifier in the session of an account.
     *
     * The name is inferred.
     *
     * @param nAccountId The account.
     * @return The identifier, or kNoId.
     * @ghidraAddress NTSC-U/C: 0x00256290
     * @ghidraAddress PAL: 0x0025f1a8
     */
    int FindIdByAccount(int nAccountId);

    /**
     * Exit the session for mGameSink.
     *
     * The name is inferred.
     *
     * @param nReason A LaunchpadAbortedMsg::Reason.
     * @ghidraAddress NTSC-U/C: 0x002562d0
     * @ghidraAddress PAL: 0x0025f1e8
     */
    void Abort(int nReason);

    /**
     * Encrypt a packet and send it to the consoles its receiver selects.
     *
     * @param pPacket The packet.
     * @ghidraAddress NTSC-U/C: 0x002562f0
     * @ghidraAddress PAL: 0x0025f208
     */
    void Send(Packet *pPacket);

    /**
     * Leave the game and tell the other consoles.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002565d8
     * @ghidraAddress PAL: 0x0025f4e0
     */
    void Leave();

    /**
     * Count a change of the players.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00256650
     * @ghidraAddress PAL: 0x0025f558
     */
    void PlayersChanged();

    /**
     * Report this console ready once the session is active and synced.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00256668
     * @ghidraAddress PAL: 0x0025f570
     */
    void CheckReady();

    /**
     * Take the state of a player.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00256760
     * @ghidraAddress PAL: 0x0025f668
     */
    bool OnMsg(ClientStatusPkt *pPkt);

    /**
     * Take the rating of a player.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x002567b0
     * @ghidraAddress PAL: 0x0025f6b8
     */
    bool OnMsg(ClientRatingPkt *pPkt);

    /**
     * Enter the game, and tell mGameSink to load the song.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00256830
     * @ghidraAddress PAL: 0x0025f738
     */
    bool OnMsg(LoadGamePkt *pPkt);

    /**
     * Take the time the song starts at and the seed of the game.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x002568b8
     * @ghidraAddress PAL: 0x0025f7c0
     */
    bool OnMsg(StartGamePkt *pPkt);

    /**
     * Drop a player who left the game, and end the game when the host left.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00256940
     * @ghidraAddress PAL: 0x0025f848
     */
    bool OnMsg(ClientAbortedPkt *pPkt);

    /**
     * Take the ranks of the players.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00256a48
     * @ghidraAddress PAL: 0x0025f950
     */
    bool OnMsg(UpdateRanksPkt *pPkt);

    /**
     * Register the packets of the session with the session library.
     *
     * The name is inferred.
     *
     * @return 0, or an error code of the library.
     * @ghidraAddress NTSC-U/C: 0x00254c90
     * @ghidraAddress PAL: 0x0025dba8
     */
    static int RegisterMessages();

    /**
     * Set sMgclReady.
     *
     * The name is inferred.
     *
     * @param nReady Non-zero once the game server library is ready.
     * @ghidraAddress NTSC-U/C: 0x00254ce0
     * @ghidraAddress PAL: 0x0025dbf8
     */
    static void SetMgclReady(int nReady);

    /**
     * Report sMgclReady.
     *
     * The name is inferred.
     *
     * @return Non-zero once the game server library is ready.
     * @ghidraAddress NTSC-U/C: 0x00254cf0
     * @ghidraAddress PAL: 0x0025dc08
     */
    static int IsMgclReady();

    /**
     * Read a byte EncodeByte() wrote.
     *
     * Each character is a digit offset from '0' by one half of the byte. The name is inferred.
     *
     * @param pszText The two characters.
     * @return The byte.
     * @ghidraAddress NTSC-U/C: 0x00254d00
     * @ghidraAddress PAL: 0x0025dc18
     */
    static unsigned char DecodeByte(const char *pszText);

    /**
     * Write a byte as two characters and a terminator.
     *
     * The name is inferred.
     *
     * @param pszText Receives the three characters.
     * @param nByte The byte.
     * @ghidraAddress NTSC-U/C: 0x00254d20
     * @ghidraAddress PAL: 0x0025dc38
     */
    static void EncodeByte(char *pszText, unsigned char nByte);

    /**
     * Report the score of a player in the final scores.
     *
     * The name is inferred.
     *
     * @param scores The final score of each player.
     * @param nId The player's identifier in the session.
     * @return The score, or 0 when the player has none.
     * @ghidraAddress NTSC-U/C: 0x00255fd0
     * @ghidraAddress PAL: 0x0025eee8
     */
    static int FindScore(const std::vector<NetGameScore> &scores, int nId);

    /**
     * Receive a message of the session, the handler rtDmeRegisterMessageHandler() registers.
     *
     * The routine decrypts the message, reads the packet, and passes it to the receivers its
     * Packet::Route() selects.
     *
     * @param nConnection The connection.
     * @param nType The registered type.
     * @param nSender The peer that sent the message.
     * @param pMessage The message.
     * @return The number of bytes consumed, the length and 2.
     * @ghidraAddress NTSC-U/C: 0x00256440
     * @ghidraAddress PAL: 0x0025f348
     */
    static int Receive(int nConnection, int nType, int nSender, rtDmeMessage *pMessage);

    /**
     * Receive the result of NetHostTask and NetConnectTask, and finish sNetTask.
     *
     * The routine takes the identifier of this console from the session library. The host must
     * have identifier kHostId and a guest must not. The name is inferred.
     *
     * @param nConnection The connection.
     * @param nReserved1 Not read.
     * @param nReserved2 Not read.
     * @param nStatus 0 for success, otherwise an error of the library.
     * @ghidraAddress NTSC-U/C: 0x0025bd68
     * @ghidraAddress PAL: 0x00264dc0
     */
    static void OnNetConnect(int nConnection, int nReserved1, int nReserved2, int nStatus);

    /**
     * Note a console that connected to the session.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nClient The identifier of the console.
     * @param pszAddress The address of the console. Not read.
     * @ghidraAddress NTSC-U/C: 0x0025be58
     * @ghidraAddress PAL: 0x00264eb0
     */
    static void OnNetClientJoin(int nConnection, int nClient, const char *pszAddress);

    /**
     * Remove a console that left the session.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nClient The identifier of the console.
     * @param pszAddress The address the departure was reported for. Not read.
     * @ghidraAddress NTSC-U/C: 0x0025be90
     * @ghidraAddress PAL: 0x00264ee8
     */
    static void OnNetClientLeave(int nConnection, int nClient, const char *pszAddress);

    int mWorldId;                        /*!< The session on the game server, or kNoWorld. */
    rtNetConnectionInfo mConnectionInfo; /*!< The connection to the session. */
    int mConnection;                     /*!< The connection of the session library, or -1. */
    int mLocalId;                        /*!< The identifier of this console, or kNoId. */
    int mState;                          /*!< One of State. */
    int mError;                          /*!< Why the host turned the console away. */
    MsgSink *mGameSink;                  /*!< The sink that receives the session's messages. */
    NetMultiTask mConnectTask;           /*!< The request that connects to the session. */
    NetMultiTask mExitTask;              /*!< The request that leaves the session. */
    NetMultiTask mRankTask;              /*!< The request for the ladder position after a game. */
    PeriodicTask mPlayerReport;          /*!< The report of the player to the game server. */
    TaskList mTasks;                     /*!< Further requests that run side by side. */
    std::list<NetLaunchpadPlayer> mPlayers; /*!< The players, in the order of their colours. */
    float mStartTime;                       /*!< The clock of the session the song starts at. */
    int mSeed;                              /*!< The seed of the game. */
    int mActive;                            /*!< Non-zero once Activate() ran. */
    int mShareOnly;                         /*!< The share-only flag of the remix of the game. */
    int mStarting;                          /*!< Non-zero while the song is about to start. */
    int mInGame;                            /*!< Non-zero during the game. */
    std::vector<int> mClients;              /*!< The consoles AddClient() noted. */
    std::vector<RatingStat> mRatings;       /*!< The rating of the game, while it runs. */

    /**
     * The registered type of the packets of the session.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0abc
     */
    static int sMessageType;

    /**
     * Non-zero once the game server library is ready.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0ac0
     */
    static int sMgclReady;

    /**
     * A count that changes whenever the players of the session change.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0ad4
     */
    static int sPlayersVersion;

    /**
     * The NetHostTask or NetConnectTask that waits for OnNetConnect(), or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0ae0
     */
    static NetTask *sNetTask;
};

/**
 * The online session, or null outside one.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ac4
 */
extern LaunchpadRT *TheLaunchpadRT;
