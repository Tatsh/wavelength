#pragma once

#include "app/msgsink.h"
#include "game/remixinfo.h"
#include "netflow/launchpadrt.h"
#include "netflow/netgameparams.h"

class BootPkt;
class ChatMsg;
class ClientInfoPkt;
class ClockSyncTask;
class DataBlockPkt;
class GameOverPkt;
class GameParamsPkt;
class JoinFailedPkt;
class ShareRemixPkt;
class TimeResponsePkt;
class WelcomePkt;

/**
 * The online session of a console that joins it.
 *
 * The RTTI records the class as deriving from LaunchpadRT. The vtables are at `0x003d45c0` and
 * `0x003d4598`, and the object is 0x278 bytes. The constructor stores the instance in
 * TheJoinpadRT. A guest asks the host for the key of the session, follows the clock of the host,
 * and receives the settings and the remix of the game.
 */
class JoinpadRT : public LaunchpadRT {
public:
    /**
     * Start joining a session.
     *
     * @param pScreen The screen that receives the result of the attempt.
     * @param pGame The sink that receives the messages of the session.
     * @param nWorldId The session on the game server.
     * @param nHostAccount The account of the host, who sends the key of the session.
     * @ghidraAddress NTSC-U/C: 0x00254d98
     * @ghidraAddress PAL: 0x0025dcb0
     */
    static void Create(MsgSink *pScreen, MsgSink *pGame, int nWorldId, int nHostAccount);

    /**
     * Construct the session, request the key from the host, and start mConnectTask.
     *
     * @param pScreen The screen that receives the result of the attempt.
     * @param pGame The sink that receives the messages of the session.
     * @param nWorldId The session on the game server.
     * @param nHostAccount The account of the host.
     * @ghidraAddress NTSC-U/C: 0x00259e30
     * @ghidraAddress PAL: 0x00262d30
     */
    JoinpadRT(MsgSink *pScreen, MsgSink *pGame, int nWorldId, int nHostAccount);

    /**
     * Stop and delete mClockSync, and clear TheJoinpadRT.
     *
     * @ghidraAddress NTSC-U/C: 0x0025a0a0
     * @ghidraAddress PAL: 0x00262fa0
     */
    ~JoinpadRT() override;

    /**
     * Remove a player who left, and exit the session when the host left.
     *
     * @param nId The player's identifier in the session.
     * @ghidraAddress NTSC-U/C: 0x0025a170
     * @ghidraAddress PAL: 0x00263080
     */
    void RemovePlayer(int nId) override;

    /**
     * Report the clock of the host.
     *
     * @return SystemMs() plus mClockOffset.
     * @ghidraAddress NTSC-U/C: 0x0025a408
     * @ghidraAddress PAL: 0x00263318
     */
    float GetTime() override;

    /**
     * Report JoinLaunchpadSuccessMsg to the sink of mConnectTask.
     *
     * @ghidraAddress NTSC-U/C: 0x0025a1b8
     * @ghidraAddress PAL: 0x002630c8
     */
    void OnConnected() override;

    /**
     * Poll mClockSync.
     *
     * @ghidraAddress NTSC-U/C: 0x0025a150
     * @ghidraAddress PAL: 0x00263050
     */
    void PollPriv() override;

    /**
     * Act on a packet of the session, before LaunchpadRT does.
     *
     * @param pMsg The message.
     * @return The result of the handler, false for every handler here.
     * @ghidraAddress NTSC-U/C: 0x0025ad30
     * @ghidraAddress PAL: 0x00263d80
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Ask the host for the key of the session.
     *
     * The name is inferred.
     *
     * @param nHostAccount The account of the host.
     * @ghidraAddress NTSC-U/C: 0x0025a480
     * @ghidraAddress PAL: 0x00263390
     */
    void RequestKey(int nHostAccount);

    /**
     * Take the offset of the clock of the host, and report this console ready when it can be.
     *
     * ClockSyncTask calls the member. The name is inferred.
     *
     * @param fOffset The clock of the host less the clock of this console, in milliseconds.
     * @param fRoundTrip The mean round trip to the host, in milliseconds. It is not read.
     * @ghidraAddress NTSC-U/C: 0x0025a4a8
     * @ghidraAddress PAL: 0x002633b8
     */
    void OnClockSynced(float fOffset, float fRoundTrip);

    /**
     * Take the colour and the settings the host welcomed this console with.
     *
     * A song the console does not have fails the session.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0025a4d0
     * @ghidraAddress PAL: 0x00263520
     */
    bool OnMsg(WelcomePkt *pPkt);

    /**
     * Fail the session with the reason of the host.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0025a720
     * @ghidraAddress PAL: 0x00263770
     */
    bool OnMsg(JoinFailedPkt *pPkt);

    /**
     * Enter a player of the session.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0025a738
     * @ghidraAddress PAL: 0x00263788
     */
    bool OnMsg(ClientInfoPkt *pPkt);

    /**
     * Pass the clock of the host to mClockSync.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0025a758
     * @ghidraAddress PAL: 0x002637a8
     */
    bool OnMsg(TimeResponsePkt *pPkt);

    /**
     * Pass the settings the host published to mGameSink, or exit for a song this console lacks.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0025a780
     * @ghidraAddress PAL: 0x002637d0
     */
    bool OnMsg(GameParamsPkt *pPkt);

    /**
     * Exit when the host removed this console, and otherwise remove the guest.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0025a8c0
     * @ghidraAddress PAL: 0x00263910
     */
    bool OnMsg(BootPkt *pPkt);

    /**
     * Rate the game from the final scores, report them, and leave the game.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0025a910
     * @ghidraAddress PAL: 0x00263960
     */
    bool OnMsg(GameOverPkt *pPkt);

    /**
     * Prepare the buffer of the remix the host shares.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0025aab0
     * @ghidraAddress PAL: 0x00263b00
     */
    bool OnMsg(ShareRemixPkt *pPkt);

    /**
     * Store a block of the remix the host shares and acknowledge it.
     *
     * @param pPkt The packet.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0025abc0
     * @ghidraAddress PAL: 0x00263c10
     */
    bool OnMsg(DataBlockPkt *pPkt);

    /**
     * Take the key of the session the host sent.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0025acc0
     * @ghidraAddress PAL: 0x00263d10
     */
    bool OnMsg(ChatMsg *pMsg);

    using LaunchpadRT::OnMsg;

    int mKeyReceived;          /*!< Non-zero once the key of the session arrived. */
    ClockSyncTask *mClockSync; /*!< The task that follows the clock of the host. */
    NetGameParams mParams;     /*!< The settings of the game the host published. */
    float mClockOffset;        /*!< The clock of the host less the clock of this console. */
    int mReserved1D4;          // +0x1d4, the alignment of mRemixInfo to eight bytes.
    RemixInfo mRemixInfo;      /*!< The description of the remix the host shares. */
};

/**
 * The joined session, or null.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0acc
 * @ghidraAddress PAL: 0x0042f998
 */
extern JoinpadRT *TheJoinpadRT;
