#pragma once

#include <vector>

#include <librtdme.h>

#include "app/msgsink.h"
#include "game/netgamescore.h"
#include "netflow/launchpadrt.h"
#include "netflow/netgameparams.h"
#include "netflow/netmultitask.h"
#include "netflow/periodictask.h"
#include "os/string.h"

class ChatMsg;
class ClientInfoPkt;
class DataAckPkt;
class TimeRequestPkt;

/**
 * The online session of a console that hosts it.
 *
 * The RTTI records the class as deriving from LaunchpadRT. The vtables are at `0x003d4698` and
 * `0x003d4670`, and the object is 0x328 bytes. The constructor stores the instance in
 * TheHostpadRT. The host hands out the colours of the players, publishes the settings of the
 * game, shares the remix of the game in blocks, and starts the song once every console loaded it.
 */
class HostpadRT : public LaunchpadRT {
public:
    /**
     * The colour of a slot of the session and the player who holds it.
     *
     * The RTTI names the type, which a standard library vector records.
     */
    struct ClientClr {
        int mColor;  /*!< The colour, 0 to 3. */
        int mPlayer; /*!< The player's identifier in the session, or kNoId. */
    };

    /**
     * The rank and the rating of a player, from which the host ranks the players.
     *
     * The RTTI names the type, which a standard library vector records.
     */
    class ClientData {
    public:
        /**
         * Construct an entry.
         *
         * Inline.
         *
         * @param nId The player's identifier in the session.
         * @param nRank The position in the ranking.
         * @param nRating The rating.
         */
        ClientData(int nId, int nRank, int nRating) : mId(nId), mRank(nRank), mRating(nRating) {
        }

        /**
         * Report whether two entries are of the same player.
         *
         * Inline.
         *
         * @param other The other entry.
         * @return Whether the identifiers are equal.
         */
        bool operator==(const ClientData &other) const {
            return mId == other.mId;
        }

        /**
         * Report whether an entry has a higher rating than another, the order of SendRanks().
         *
         * @param left The first entry.
         * @param right The second entry.
         * @return Whether left has the higher rating.
         * @ghidraAddress NTSC-U/C: 0x00398d38
         * @ghidraAddress PAL: 0x004075c0
         */
        static bool HigherRating(const ClientData &left, const ClientData &right) {
            return right.mRating < left.mRating;
        }

        int mId;     /*!< The player's identifier in the session. */
        int mRank;   /*!< The position in the ranking. */
        int mRating; /*!< The rating. */
    };

    /** The number of slots of a session. */
    static constexpr int kMaxPlayers = 4;

    /**
     * Start hosting a session.
     *
     * @param pScreen The screen that receives the result of the attempt.
     * @param pGame The sink that receives the messages of the session.
     * @ghidraAddress NTSC-U/C: 0x00254d48
     * @ghidraAddress PAL: 0x0025dc60
     */
    static void Create(MsgSink *pScreen, MsgSink *pGame);

    /**
     * Construct the session, draw its key, and start mConnectTask.
     *
     * @param pScreen The screen that receives the result of the attempt.
     * @param pGame The sink that receives the messages of the session.
     * @ghidraAddress NTSC-U/C: 0x00256be0
     * @ghidraAddress PAL: 0x0025fae8
     */
    HostpadRT(MsgSink *pScreen, MsgSink *pGame);

    /**
     * Stop mShareTask and clear TheHostpadRT.
     *
     * @ghidraAddress NTSC-U/C: 0x002577d0
     * @ghidraAddress PAL: 0x002606d0
     */
    ~HostpadRT() override;

    /**
     * Take the settings of the game again and publish them once the remix name is filtered.
     *
     * @ghidraAddress NTSC-U/C: 0x002579d8
     * @ghidraAddress PAL: 0x002608d8
     */
    void EndEdit() override;

    /**
     * Share the remix of the game with the guests only.
     *
     * @return Whether the share started.
     * @ghidraAddress NTSC-U/C: 0x00257c48
     * @ghidraAddress PAL: 0x00260b48
     */
    bool ShareRemix() override;

    /**
     * Load the song on every console once every player is ready.
     *
     * A game of a saved remix first shares the remix.
     *
     * @return 1 when the game starts, otherwise 0.
     * @ghidraAddress NTSC-U/C: 0x00258008
     * @ghidraAddress PAL: 0x00260f08
     */
    int Launch() override;

    /**
     * Ban a guest from the session and remove the guest.
     *
     * @param nPlayer The guest.
     * @ghidraAddress NTSC-U/C: 0x00258100
     * @ghidraAddress PAL: 0x00261000
     */
    void BootPlayer(int nPlayer) override;

    /**
     * Send the reports to the game server at the next poll.
     *
     * @ghidraAddress NTSC-U/C: 0x002589e0
     * @ghidraAddress PAL: 0x002618e0
     */
    void ReportNow() override;

    /**
     * Take the identifier of this console and enter the host as the first player.
     *
     * @param nId The identifier.
     * @ghidraAddress NTSC-U/C: 0x00258a28
     * @ghidraAddress PAL: 0x00261928
     */
    void SetLocalId(int nId) override;

    /**
     * Welcome a console that connected, or turn it away.
     *
     * @param nId The identifier of the console.
     * @ghidraAddress NTSC-U/C: 0x00258dd0
     * @ghidraAddress PAL: 0x00261cd0
     */
    void AddClient(int nId) override;

    /**
     * Free the colour of a player who left, and forget the player.
     *
     * @param nId The player's identifier in the session.
     * @ghidraAddress NTSC-U/C: 0x00258fc8
     * @ghidraAddress PAL: 0x00261ec8
     */
    void RemovePlayer(int nId) override;

    /**
     * Report the clock of this console, the clock of the session.
     *
     * @return SystemMs().
     * @ghidraAddress NTSC-U/C: 0x002593b0
     * @ghidraAddress PAL: 0x002622b0
     */
    float GetTime() override;

    /**
     * Report HostLaunchpadSuccessMsg to the sink of mConnectTask.
     *
     * @ghidraAddress NTSC-U/C: 0x00259158
     * @ghidraAddress PAL: 0x00262058
     */
    void OnConnected() override;

    /**
     * Poll the reports and the share, and watch the connection to the game server.
     *
     * @ghidraAddress NTSC-U/C: 0x002583d0
     * @ghidraAddress PAL: 0x002612d0
     */
    void PollPriv() override;

    /**
     * Leave the game and send the reports.
     *
     * @ghidraAddress NTSC-U/C: 0x00258398
     * @ghidraAddress PAL: 0x00261298
     */
    void EndGame() override;

    /**
     * Send the rating of the host and note it in mClientData.
     *
     * @param nRank The position of the host in the ranking.
     * @ghidraAddress NTSC-U/C: 0x00259868
     * @ghidraAddress PAL: 0x00262768
     */
    void SendRating(int nRank) override;

    /**
     * Act on a packet of the session, before LaunchpadRT does.
     *
     * @param pMsg The message.
     * @return The result of the handler, false for every handler here.
     * @ghidraAddress NTSC-U/C: 0x00259d40
     * @ghidraAddress PAL: 0x00262c40
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Send the settings of the game to every console, and send the world report at the next poll.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00257b40
     * @ghidraAddress PAL: 0x00260a40
     */
    void SendGameParams();

    /**
     * Share the remix of the game with every guest once every player is ready.
     *
     * The name is inferred.
     *
     * @param nShareOnly Non-zero when the remix is only shared, not played.
     * @return Whether the share started.
     * @ghidraAddress NTSC-U/C: 0x00257c68
     * @ghidraAddress PAL: 0x00260b68
     */
    bool StartShare(int nShareOnly);

    /**
     * Remove a guest from the session, and ban the guest from the game server first.
     *
     * The name is inferred.
     *
     * @param nId The guest's identifier in the session.
     * @param nBan Non-zero to ban the guest.
     * @ghidraAddress NTSC-U/C: 0x00258120
     * @ghidraAddress PAL: 0x00261020
     */
    void Kick(int nId, int nBan);

    /**
     * Send the final scores to every console, rate the game, and leave it.
     *
     * The name is inferred.
     *
     * @param scores The final score of each player.
     * @ghidraAddress NTSC-U/C: 0x00258208
     * @ghidraAddress PAL: 0x00261108
     */
    void SendScores(const std::vector<NetGameScore> &scores);

    /**
     * Start the song once every console loaded it, and remove the consoles too slow to load.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00258478
     * @ghidraAddress PAL: 0x00261378
     */
    void PollStart();

    /**
     * Remove every guest that has not loaded the song.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00258668
     * @ghidraAddress PAL: 0x00261568
     */
    void KickUnloaded();

    /**
     * Send the next block of the remix once every guest acknowledged the last, and remove the
     * guests too slow to.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00258718
     * @ghidraAddress PAL: 0x00261618
     */
    void PollShare();

    /**
     * Report whether the session takes another player.
     *
     * The name is inferred.
     *
     * @return Non-zero outside a game and a share while a slot is free.
     * @ghidraAddress NTSC-U/C: 0x00259050
     * @ghidraAddress PAL: 0x00261f50
     */
    int CanJoin();

    /**
     * Report the state of the session for the game server.
     *
     * The name is inferred.
     *
     * @return 2 during a game, otherwise 1.
     * @ghidraAddress NTSC-U/C: 0x002590b0
     * @ghidraAddress PAL: 0x00261fb0
     */
    int GetWorldStatus();

    /**
     * Give a player the first free colour.
     *
     * The name is inferred.
     *
     * @param nId The player's identifier in the session.
     * @return The colour, or -1 when every colour is taken.
     * @ghidraAddress NTSC-U/C: 0x002590c8
     * @ghidraAddress PAL: 0x00261fc8
     */
    int AssignColor(int nId);

    /**
     * Free the colour of a player.
     *
     * The name is inferred.
     *
     * @param nId The player's identifier in the session.
     * @ghidraAddress NTSC-U/C: 0x00259110
     * @ghidraAddress PAL: 0x00262010
     */
    void FreeColor(int nId);

    /**
     * Rank the players by rating and send the ranks to every console.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00259418
     * @ghidraAddress PAL: 0x00262318
     */
    void SendRanks();

    /**
     * Enter a console whose player arrived, or remove it when the session is full.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x002598e0
     * @ghidraAddress PAL: 0x002627e0
     */
    bool OnMsg(ClientInfoPkt *pPkt);

    /**
     * Answer the clock of the session.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00259b60
     * @ghidraAddress PAL: 0x00262a60
     */
    bool OnMsg(TimeRequestPkt *pPkt);

    /**
     * Note a guest that received the last block of the remix.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00259c28
     * @ghidraAddress PAL: 0x00262b28
     */
    bool OnMsg(DataAckPkt *pPkt);

    /**
     * Send the key of the session to the account that requested it.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00259c50
     * @ghidraAddress PAL: 0x00262b50
     */
    bool OnMsg(ChatMsg *pMsg);

    /**
     * Take the rating of a player and send the ranks again.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00259cd0
     * @ghidraAddress PAL: 0x00262bd0
     */
    bool OnMsg(ClientRatingPkt *pPkt);

    using LaunchpadRT::OnMsg;

    int mMgclConnected;              /*!< Non-zero while the game server library is connected. */
    int mLost;                       /*!< Non-zero once the game server library failed. */
    rtNetConnectionInfo mServerInfo; /*!< The game server the authentication returned. */
    String mSessionKey;              /*!< The key of the session on the game server. */
    NetGameParams mParams;           /*!< The settings of the game. */
    PeriodicTask mServerReport;      /*!< The report of the session to the game server. */
    PeriodicTask mWorldReport;       /*!< The report of the world to the lobby server. */
    NetMultiTask mShareTask;         /*!< The share of the remix before the song loads. */
    std::vector<ClientClr> mColors;  /*!< The colours of the slots and their players. */
    float mSyncDeadline;             /*!< When the slow consoles are removed, or 0. */
    unsigned char mKey[kKeySize];    /*!< The key of the session. */
    std::vector<int> mRecipients;    /*!< The guests the remix is shared with. */
    std::vector<int> mAwaiting;      /*!< The guests that have not acknowledged the block. */
    std::vector<ClientData> mClientData; /*!< The ranks and ratings of the players. */
    int mSharing;                        /*!< Non-zero while the remix is shared. */
    int mBlock;                          /*!< The block of the remix sent next. */
    int mBlockCount;                     /*!< The blocks of the remix. */
    int mRemixShared;                    /*!< Non-zero once the share finished. */
    float mBlockDeadline;                /*!< When the guests that did not acknowledge leave. */
};

/**
 * The hosted session, or null.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ac8
 */
extern HostpadRT *TheHostpadRT;
