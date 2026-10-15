#pragma once

#include <list>
#include <set>
#include <vector>

#include <librtdme.h>

#include "netflow/lobbylocation.h"
#include "netflow/lobbyplayer.h"
#include "netflow/netchatroominfo.h"
#include "netflow/netgameparams.h"
#include "netflow/netlaunchpadinfo.h"
#include "netflow/netlobby.h"
#include "netflow/netmultitask.h"
#include "netflow/netreporemix.h"
#include "netflow/periodictask.h"
#include "netflow/rtfilter.h"
#include "netflow/tasklist.h"
#include "os/string.h"

class DataArray;

/**
 * The online lobby service on the SCE-RT lobby server.
 *
 * The RTTI includes the name and records NetLobby as the base. The vtable is at `0x003d4e78` and
 * the object is 0x690 bytes. Each request of NetLobby runs as a chain of steps in a NetMultiTask,
 * and Poll() reports each finished chain to its receiver. The names of the members are inferred.
 */
class LobbyRT : public NetLobby {
public:
#ifdef VIDEO_STANDARD_PAL
    /** The identifier of the game on the lobby server, a variable in the European release. */
    static int sApplicationId;
#else
    /** The identifier of the game on the lobby server. */
    static constexpr int kApplicationId = 10164;
#endif

    /**
     * Report the identifier of the game on the lobby server.
     *
     * Inline at each use.
     *
     * @return The identifier.
     */
    static int ApplicationId() {
#ifdef VIDEO_STANDARD_PAL
        return sApplicationId;
#else
        return kApplicationId;
#endif
    }

    /** Values of mDisconnectReason, the result Poll() reports once mDisconnectTask finishes. */
    enum DisconnectReason {
        kReasonDisconnect = 0, /*!< A LobbyDisconnectResultMsg. */
        kReasonConnect = 1,    /*!< A failed LobbyConnectResultMsg. */
        kReasonLogin = 2,      /*!< A failed LoginResultMsg. */
        kReasonLost = 3,       /*!< A LobbyConnectionLostMsg. */
    };

    /** Values of mTransferMode. */
    enum TransferMode {
        kTransferNone = 0,            /*!< No transfer. */
        kTransferDirectory = 1,       /*!< Download of the remix directory. */
        kTransferRemix = 2,           /*!< Download of a remix. */
        kTransferUploadRemix = 3,     /*!< Upload of the current remix. */
        kTransferUploadDirectory = 4, /*!< Upload of the remix directory. */
        kTransferList = 5,            /*!< List of the files. */
        kTransferDelete = 6,          /*!< Deletion of a file. */
    };

    /** Values of the direction of Page(). */
    enum PageDirection {
        kPageFirst = 0,    /*!< The first page. */
        kPageNext = 1,     /*!< The next page, unless the current page is short. */
        kPageCurrent = 2,  /*!< The current page, or the first when nothing is listed. */
        kPagePrevious = 3, /*!< The previous page. */
        kPageFirstAll = 4, /*!< The first page, in the large size. */
    };

    /**
     * Construct the service, read `server.cfg`, and build the chain of each request.
     *
     * @ghidraAddress NTSC-U/C: 0x0025d7f0
     * @ghidraAddress PAL: 0x00266888
     */
    LobbyRT();

    /**
     * Release the service.
     *
     * @ghidraAddress NTSC-U/C: 0x0025ed98
     * @ghidraAddress PAL: 0x00267e78
     */
    ~LobbyRT() override;

    /**
     * Vtable slot 3. Take the name and the Freq of the local player, and forget the password.
     *
     * @param pProfile The profile of the player.
     * @ghidraAddress NTSC-U/C: 0x0025f248
     * @ghidraAddress PAL: 0x00268328
     */
    void SetProfile(Campaign *pProfile) override;

    /**
     * Vtable slot 4. Start the chain of mConnectTask.
     *
     * @param pSink The receiver of the LobbyConnectResultMsg.
     * @ghidraAddress NTSC-U/C: 0x0025f2c8
     * @ghidraAddress PAL: 0x002683a8
     */
    void Connect(MsgSink *pSink) override;

    /**
     * Vtable slot 5. Reset the record of the local player and start the chain of mLoginTask.
     *
     * @param pSink The receiver of the LoginResultMsg.
     * @param nLocation The identifier of the lobby location.
     * @param pszPassword The password.
     * @ghidraAddress NTSC-U/C: 0x0025f310
     * @ghidraAddress PAL: 0x002683f0
     */
    void Login(MsgSink *pSink, int nLocation, const char *pszPassword) override;

    /**
     * Vtable slot 6. Stop every request and start the chain of mDisconnectTask.
     *
     * @param pSink The receiver of the LobbyDisconnectResultMsg.
     * @ghidraAddress NTSC-U/C: 0x0025f2a8
     * @ghidraAddress PAL: 0x00268388
     */
    void Disconnect(MsgSink *pSink) override;

    /**
     * Vtable slot 7. Report the account of the local player.
     *
     * @return The identifier of mLocalPlayer.
     * @ghidraAddress NTSC-U/C: 0x0025f4b0
     * @ghidraAddress PAL: 0x00268590
     */
    int GetAccountId() override;

    /**
     * Vtable slot 8. Start the chain of mCreateAccountTask.
     *
     * @param pSink The receiver of the CreateAccountResultMsg.
     * @param pszPassword The password of the account.
     * @ghidraAddress NTSC-U/C: 0x0025f4b8
     * @ghidraAddress PAL: 0x00268598
     */
    void CreateAccount(MsgSink *pSink, const char *pszPassword) override;

    /**
     * Vtable slot 9. Start the chain of mDeleteAccountTask.
     *
     * @param pSink The receiver of the DeleteAccountResultMsg.
     * @ghidraAddress NTSC-U/C: 0x0025f500
     * @ghidraAddress PAL: 0x002685e0
     */
    void DeleteAccount(MsgSink *pSink) override;

    /**
     * Vtable slot 10. Start the chain of mChangePasswordTask.
     *
     * @param pSink The receiver of the ChangePasswordResultMsg.
     * @param pszOldPassword The current password.
     * @param pszNewPassword The new password.
     * @ghidraAddress NTSC-U/C: 0x0025f520
     * @ghidraAddress PAL: 0x00268600
     */
    void
    ChangePassword(MsgSink *pSink, const char *pszOldPassword, const char *pszNewPassword) override;

    /**
     * Vtable slot 11. Start the chain of mLobbyPlayersTask.
     *
     * @param pSink The receiver of the LobbyPlayersMsg.
     * @ghidraAddress NTSC-U/C: 0x0025f828
     * @ghidraAddress PAL: 0x00268908
     */
    void RequestLobbyPlayers(MsgSink *pSink) override;

    /**
     * Vtable slot 12. Start the chain of mLaunchpadPlayersTask unless it is running.
     *
     * @param pSink The receiver of the LobbyPlayersMsg.
     * @param nLaunchpad The identifier of the launchpad.
     * @ghidraAddress NTSC-U/C: 0x0025f8b0
     * @ghidraAddress PAL: 0x00268990
     */
    void RequestLaunchpadPlayers(MsgSink *pSink, int nLaunchpad) override;

    /**
     * Vtable slot 13. Add a LadderPosition2Task to mTasks.
     *
     * @param pSink The receiver of the PlayerRankMsg.
     * @param nAccount The account of the player.
     * @ghidraAddress NTSC-U/C: 0x0025f848
     * @ghidraAddress PAL: 0x00268928
     */
    void AddLadderListener(MsgSink *pSink, int nAccount) override;

    /**
     * Vtable slot 14. Add an account to mMuted or remove it.
     *
     * @param nAccount The account of the player.
     * @param bMuted Whether the player is muted.
     * @ghidraAddress NTSC-U/C: 0x0025f580
     * @ghidraAddress PAL: 0x00268660
     */
    void SetMuted(int nAccount, bool bMuted) override;

    /**
     * Vtable slot 15. Build mFilters and start the chain of mLaunchpadsTask.
     *
     * @param pSink The receiver of the LobbyLaunchpadsMsg.
     * @param nStart One of PageDirection.
     * @param nMode Non-zero to list the launchpads of the current chat room only.
     * @param pszArena The song of the search, or an empty string for any.
     * @param nRuleSet The game mode of the search, or 0 for any.
     * @param nSkillLevel The skill level of the search, or -1 for any.
     * @ghidraAddress NTSC-U/C: 0x0025f918
     * @ghidraAddress PAL: 0x002689f8
     */
    void RequestLaunchpads(MsgSink *pSink,
                           int nStart,
                           int nMode,
                           const char *pszArena,
                           int nRuleSet,
                           int nSkillLevel) override;

    /**
     * Vtable slot 16. Start the chain of mFindPlayerTask.
     *
     * @param pSink The receiver of the FindPlayerMsg.
     * @param pszName The name of the player.
     * @ghidraAddress NTSC-U/C: 0x002601d0
     * @ghidraAddress PAL: 0x002692b0
     */
    void FindPlayer(MsgSink *pSink, const char *pszName) override;

    /**
     * Vtable slot 17. Start the chain of mChatroomsTask.
     *
     * @param pSink The receiver of the LobbyChatroomsMsg.
     * @ghidraAddress NTSC-U/C: 0x00260218
     * @ghidraAddress PAL: 0x002692f8
     */
    void RequestChatrooms(MsgSink *pSink) override;

    /**
     * Vtable slot 18. Start the chain of mJoinChatroomTask.
     *
     * @param pSink The receiver of the JoinChatroomResultMsg.
     * @param nChatroomId The identifier of the room.
     * @ghidraAddress NTSC-U/C: 0x00260238
     * @ghidraAddress PAL: 0x00269318
     */
    void JoinChatroom(MsgSink *pSink, int nChatroomId) override;

    /**
     * Vtable slot 19. Start the chain of mCreateChatroomTask.
     *
     * @param pSink The receiver of the JoinChatroomResultMsg.
     * @param name The name of the chat room.
     * @ghidraAddress NTSC-U/C: 0x00260258
     * @ghidraAddress PAL: 0x00269338
     */
    void CreateChatroom(MsgSink *pSink, const String &name) override;

    /**
     * Vtable slot 20. Start the chain of mRanksTask unless it is running.
     *
     * @param pSink The receiver of the LobbyPlayersMsg.
     * @param nCount The players of a page.
     * @param nDirection One of PageDirection.
     * @param nMode Non-zero to list the ranking without the details of the players.
     * @ghidraAddress NTSC-U/C: 0x002602a0
     * @ghidraAddress PAL: 0x00269380
     */
    void RequestRanks(MsgSink *pSink, int nCount, int nDirection, int nMode) override;

    /**
     * Vtable slot 21. Upload the current remix with a note, once a day.
     *
     * @param pSink The receiver of the RepoRemixStatusMsg.
     * @param pszNote The note of the remix.
     * @ghidraAddress NTSC-U/C: 0x00260368
     * @ghidraAddress PAL: 0x00269448
     */
    void UploadRemix(MsgSink *pSink, const char *pszNote) override;

    /**
     * Vtable slot 22. Start downloading the remix directory.
     *
     * @param pSink The receiver of the RepoRemixesMsg.
     * @ghidraAddress NTSC-U/C: 0x002608e8
     * @ghidraAddress PAL: 0x002699c8
     */
    void RequestRemixes(MsgSink *pSink) override;

    /**
     * Vtable slot 23. Start downloading a remix.
     *
     * @param pSink The receiver of the RepoRemixStatusMsg and the RemixDownloadMsg.
     * @param pszName The file of the remix.
     * @ghidraAddress NTSC-U/C: 0x00260938
     * @ghidraAddress PAL: 0x00269a18
     */
    void DownloadRemix(MsgSink *pSink, const char *pszName) override;

    /**
     * Vtable slot 24. Start the chain of mFileListTask.
     *
     * @param pSink The receiver of the FileListMsg.
     * @param nRemixes Non-zero to list only the files whose names begin with `r_`.
     * @ghidraAddress NTSC-U/C: 0x00260d18
     * @ghidraAddress PAL: 0x00269df8
     */
    void RequestFiles(MsgSink *pSink, int nRemixes) override;

    /**
     * Vtable slot 25. Write the remix directory to a buffer and start uploading it.
     *
     * @param pSink The receiver of the RepoRemixStatusMsg.
     * @param pRemixes The remixes of the directory.
     * @ghidraAddress NTSC-U/C: 0x00260650
     * @ghidraAddress PAL: 0x00269730
     */
    void UploadDirectory(MsgSink *pSink, std::list<NetRepoRemix> *pRemixes) override;

    /**
     * Vtable slot 26. Start the chain of mDeleteFileTask.
     *
     * @param pSink The receiver of the result.
     * @param pszName The file.
     * @ghidraAddress NTSC-U/C: 0x00260e60
     * @ghidraAddress PAL: 0x00269f50
     */
    void DeleteFile(MsgSink *pSink, const char *pszName) override;

    /**
     * Service the library and every request, and report the finished chains.
     *
     * @ghidraAddress NTSC-U/C: 0x00260f28
     * @ghidraAddress PAL: 0x0026a038
     */
    void Poll();

    /**
     * Stop every request, forget the filters and the muted players, and start the chain of
     * mDisconnectTask, which reports nResult as nReason requires.
     *
     * @param pSink The receiver of the report.
     * @param nResult The error reported.
     * @param nReason One of DisconnectReason.
     * @ghidraAddress NTSC-U/C: 0x00262708
     * @ghidraAddress PAL: 0x0026b860
     */
    void Fail(MsgSink *pSink, int nResult, int nReason);

    /**
     * Take the account, the chat channel, and the connection a login or a join reports.
     *
     * @param nAccount The account of the local player.
     * @param nChannel The chat channel.
     * @param pInfo The connection to the lobby server.
     * @ghidraAddress NTSC-U/C: 0x002628b8
     * @ghidraAddress PAL: 0x0026ba10
     */
    void SetAccount(int nAccount, int nChannel, const rtNetConnectionInfo *pInfo);

    /**
     * Report the name of the local player.
     *
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x002629a0
     * @ghidraAddress PAL: 0x0026baf8
     */
    const char *GetName();

    /**
     * Report the password of the local player.
     *
     * @return mPassword.
     * @ghidraAddress NTSC-U/C: 0x002629a8
     * @ghidraAddress PAL: 0x0026bb00
     */
    const char *GetPassword();

    /**
     * Encode the record of the local player into mStatsText.
     *
     * @return mStatsText.
     * @ghidraAddress NTSC-U/C: 0x002629b0
     * @ghidraAddress PAL: 0x0026bb08
     */
    const char *GetPlayerStats();

    /**
     * Take the rating, the games, the record, and the rank of the local player after a game, and
     * add an AccountUpdateStatsTask to mTasks when any of them changed.
     *
     * @param pPlayer The player after the game.
     * @ghidraAddress NTSC-U/C: 0x002629e0
     * @ghidraAddress PAL: 0x0026bb38
     */
    void UpdateStats(const LobbyPlayer *pPlayer);

    /**
     * Report whether an account is in mMuted.
     *
     * @param nAccount The account.
     * @return Whether the account is muted.
     * @ghidraAddress NTSC-U/C: 0x00262ae8
     * @ghidraAddress PAL: 0x0026bc40
     */
    bool IsMuted(int nAccount);

    /**
     * Record the result of a call to the peer-to-peer layer. The body is empty.
     *
     * @param nLine The line of the original source that made the call.
     * @param nError The result.
     * @ghidraAddress NTSC-U/C: 0x00262b60
     * @ghidraAddress PAL: 0x0026bcb8
     */
    void LogNetError(int nLine, int nError);

    /**
     * Record the result of a call to the lobby client. The body is empty.
     *
     * @param nLine The line of the original source that made the call.
     * @param nError The result.
     * @ghidraAddress NTSC-U/C: 0x00262b68
     * @ghidraAddress PAL: 0x0026bcc0
     */
    void LogMediusError(int nLine, int nError);

    /**
     * Pass the result of a call to the peer-to-peer layer to LogNetError() of TheLobbyRT.
     *
     * Inline at each call.
     *
     * @param nLine The line of the call.
     * @param nError The result.
     */
    static void ReportNetError(int nLine, int nError);

    /**
     * Pass the result of a call to the lobby client to LogMediusError() of TheLobbyRT.
     *
     * Inline at each call.
     *
     * @param nLine The line of the call.
     * @param nError The result.
     */
    static void ReportMediusError(int nLine, int nError);

    /**
     * Send a RepoRemixStatusMsg with the progress to the running transfer.
     *
     * @param fProgress The part transferred, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x00260eb8
     * @ghidraAddress PAL: 0x00269fa8
     */
    void ReportProgress(float fProgress);

    /**
     * The `unlimited_upload` script command, which lifts the limit of one upload a day.
     *
     * @param pCommand The command.
     * @param pUserData Not read.
     * @ghidraAddress NTSC-U/C: 0x0025ccd8
     * @ghidraAddress PAL: 0x00265d38
     */
    static void AllowUnlimitedUpload(DataArray *pCommand, void *pUserData);

    /**
     * Read a number of a fixed count of digits from encoded text.
     *
     * @param pnValue Receives the number.
     * @param ppszText The text, advanced past the digits.
     * @param pnRemaining The characters left, reduced by the digits.
     * @param nDigits The count of digits.
     * @return False, with nothing read, when fewer characters remain.
     * @ghidraAddress NTSC-U/C: 0x0025cce8
     * @ghidraAddress PAL: 0x00265d48
     */
    static bool ReadNumber(int *pnValue, const char **ppszText, int *pnRemaining, int nDigits);

    /**
     * Convert a connection class of the lobby server to a LobbyPlayer::ConnectionType.
     *
     * @param nClass The connection class.
     * @return The type.
     * @ghidraAddress NTSC-U/C: 0x0025cd80
     * @ghidraAddress PAL: 0x00265de0
     */
    static int ConnectionTypeOf(int nClass);

    /**
     * Convert a NetInet::GetConnectionType() value to a connection class of the lobby server.
     *
     * @param nType The kind of connection.
     * @return The connection class.
     * @ghidraAddress NTSC-U/C: 0x0025cdc8
     * @ghidraAddress PAL: 0x00265e28
     */
    static int ConnectionClassOf(int nType);

    /**
     * Encode the settings of a game as the name the lobby server lists it by.
     *
     * @param pName Receives the name.
     * @param pParams The settings.
     * @param nPlayers The players in the game.
     * @param nConnectionType The kind of connection of the host.
     * @param nAccountId The account of the host.
     * @ghidraAddress NTSC-U/C: 0x0025ce00
     * @ghidraAddress PAL: 0x00265e60
     */
    static void EncodeGameName(String *pName,
                               const NetGameParams *pParams,
                               int nPlayers,
                               int nConnectionType,
                               int nAccountId);

    /**
     * Decode the name EncodeGameName() writes.
     *
     * Each output starts at its default, and decoding stops at the first field missing.
     *
     * @param pParams Receives the settings.
     * @param pnPlayers Receives the players in the game.
     * @param pnConnectionType Receives the kind of connection of the host.
     * @param pnAccountId Receives the account of the host.
     * @param pszName The name.
     * @ghidraAddress NTSC-U/C: 0x0025cec8
     * @ghidraAddress PAL: 0x00265f28
     */
    static void DecodeGameName(NetGameParams *pParams,
                               int *pnPlayers,
                               int *pnConnectionType,
                               int *pnAccountId,
                               const char *pszName);

    /**
     * Encode the rating, the record, and the Freq of a player as the statistics text of the lobby
     * server.
     *
     * @param pText Receives the text.
     * @param pPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x0025d100
     * @ghidraAddress PAL: 0x00266160
     */
    static void EncodePlayer(String *pText, const LobbyPlayer *pPlayer);

    /**
     * Decode the text EncodePlayer() writes.
     *
     * @param pPlayer Receives the rating, the record, and the Freq.
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x0025d288
     * @ghidraAddress PAL: 0x002662e8
     */
    static void DecodePlayer(LobbyPlayer *pPlayer, const char *pszText);

    /**
     * Encode a game mode and a skill level as a value of the rules field and its comparison.
     *
     * Another game mode writes nothing.
     *
     * @param pnValue Receives the value.
     * @param pnComparison Receives one of RTFilter::Comparison.
     * @param nRuleSet The game mode.
     * @param nSkillLevel The skill level, or a negative value for any.
     * @ghidraAddress NTSC-U/C: 0x0025d5a0
     * @ghidraAddress PAL: 0x00266638
     */
    static void EncodeRules(int *pnValue, int *pnComparison, int nRuleSet, int nSkillLevel);

    /**
     * Encode a song as its number.
     *
     * @param pnLevel Receives the number.
     * @param pszSong The song.
     * @ghidraAddress NTSC-U/C: 0x0025d690
     * @ghidraAddress PAL: 0x00266728
     */
    static void EncodeSong(int *pnLevel, const char *pszSong);

    /**
     * Set the song of the settings from its number, or clear it for an unknown number.
     *
     * @param pParams The settings.
     * @param nLevel The number.
     * @ghidraAddress NTSC-U/C: 0x0025d6d0
     * @ghidraAddress PAL: 0x00266768
     */
    static void DecodeSong(NetGameParams *pParams, int nLevel);

    /**
     * Convert a state of a game on the lobby server to a NetLaunchpadInfo::Status.
     *
     * @param nWorldStatus The state.
     * @return kStatusOpen for an open game, otherwise kStatusBusy.
     * @ghidraAddress NTSC-U/C: 0x0025d738
     * @ghidraAddress PAL: 0x002667d0
     */
    static int LaunchpadStatusOf(int nWorldStatus);

    /**
     * Move a page of a list.
     *
     * @param pnPage The page, from 0.
     * @param pnPageSize Receives the entries of a page.
     * @param nDirection One of PageDirection. Another value changes nothing.
     * @param nPageSize The entries of a page.
     * @param nLargePageSize The entries of a page for kPageFirstAll.
     * @param nCount The entries of the current page.
     * @ghidraAddress NTSC-U/C: 0x0025d768
     * @ghidraAddress PAL: 0x00266800
     */
    static void Page(int *pnPage,
                     int *pnPageSize,
                     int nDirection,
                     int nPageSize,
                     int nLargePageSize,
                     int nCount);

    /**
     * Non-zero once the `unlimited_upload` script command ran.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0af4
     * @ghidraAddress PAL: 0x0042f9c0
     */
    static int sUnlimitedUpload;

    int mDisconnectReason;                /*!< One of DisconnectReason. */
    int mDisconnected;                    /*!< Non-zero once the connection failed. */
    NetMultiTask mConnectTask;            /*!< The chain of Connect(). */
    NetMultiTask mDisconnectTask;         /*!< The chain of Disconnect() and Fail(). */
    NetMultiTask mCreateAccountTask;      /*!< The chain of CreateAccount(). */
    NetMultiTask mDeleteAccountTask;      /*!< The chain of DeleteAccount(). */
    NetMultiTask mChangePasswordTask;     /*!< The chain of ChangePassword(). */
    NetMultiTask mLoginTask;              /*!< The chain of Login(). */
    NetMultiTask mChatroomsTask;          /*!< The chain of RequestChatrooms(). */
    NetMultiTask mJoinChatroomTask;       /*!< The chain of JoinChatroom(). */
    NetMultiTask mCreateChatroomTask;     /*!< The chain of CreateChatroom(). */
    NetMultiTask mLobbyPlayersTask;       /*!< The chain of RequestLobbyPlayers(). */
    NetMultiTask mLaunchpadPlayersTask;   /*!< The chain of RequestLaunchpadPlayers(). */
    NetMultiTask mLaunchpadsTask;         /*!< The chain of RequestLaunchpads(). */
    NetMultiTask mFindPlayerTask;         /*!< The chain of FindPlayer(). */
    NetMultiTask mRanksTask;              /*!< The chain of RequestRanks(). */
    NetMultiTask mDownloadTask;           /*!< The chain of the downloads. */
    NetMultiTask mUploadTask;             /*!< The chain of the uploads. */
    NetMultiTask mFileListTask;           /*!< The chain of RequestFiles(). */
    NetMultiTask mDeleteFileTask;         /*!< The chain of DeleteFile(). */
    TaskList mTasks;                      /*!< The steps that run on their own. */
    PeriodicTask mCheckInet;              /*!< The check of the network connection each second. */
    int mRtReady;                         /*!< Non-zero while the library is started. */
    int mMediusConnected;                 /*!< Non-zero while the lobby server is connected. */
    String mSessionKey;                   /*!< The key of the lobby session. */
    rtNetKeyPair mKeyPair;                /*!< The key pair of the console. */
    int mSecure;                          /*!< Non-zero when the connection is encrypted. */
    const char *mServerName;              /*!< The host name of the lobby server. */
    int mServerPort;                      /*!< The port of the lobby server. */
    String mServerAddress;                /*!< The address the name of the server resolves to. */
    std::list<LobbyLocation> mLocations;  /*!< The locations of the server. */
    int mLocationId;                      /*!< The location of the login. */
    int mJoinChatroomId;                  /*!< The room JoinChatroom() joins. */
    String mNewChatroomName;              /*!< The name of the room CreateChatroom() creates. */
    int mLaunchpadsPage;                  /*!< The page of the launchpads, from 0. */
    int mLaunchpadsPageSize;              /*!< The launchpads of a page. */
    int mLaunchpadId;                     /*!< The launchpad RequestLaunchpadPlayers() lists. */
    String mReserved398;                  // +0x398, never read or written after construction.
    String mNewPassword;                  /*!< The password ChangePassword() sets. */
    std::vector<RTFilter> mFilters;       /*!< The filters of the next list of launchpads. */
    int mRanksPage;                       /*!< The page of the ranking, from 0. */
    int mRanksPageSize;                   /*!< The players of a page of the ranking. */
    int mRanksMode;                       /*!< The mode of RequestRanks(). */
    int mUploadAttempts;                  /*!< The uploads of the current remix tried. */
    int mTransferSize;                    /*!< The size of mTransferBuffer in bytes. */
    String mTransferName;                 /*!< The file of the transfer. */
    char *mTransferBuffer;                /*!< The bytes of the transfer, or null. */
    int mTransferMode;                    /*!< One of TransferMode. */
    int mDeleteBeforeUpload;              /*!< Non-zero to delete the file before the upload. */
    std::list<String> mFiles;             /*!< The files the last list found. */
    LobbyPlayer mLocalPlayer;             /*!< The local player. */
    String mPassword;                     /*!< The password of the local player. */
    String mStatsText;                    /*!< The text GetPlayerStats() encodes. */
    rtNetConnectionInfo mLobbyInfo;       /*!< The connection to the lobby server. */
    NetChatroomInfo mChatroom;            /*!< The chat room of the local player. */
    std::set<int> mMuted;                 /*!< The muted accounts. */
    std::list<LobbyPlayer> mLobbyPlayers; /*!< The players of the chat room. */
    std::list<LobbyPlayer> mLaunchpadPlayers; /*!< The players of a launchpad. */
    std::list<LobbyPlayer> mRankedPlayers;    /*!< The page of the ranking. */
    std::list<NetLaunchpadInfo> mLaunchpads;  /*!< The page of the launchpads. */
    std::list<NetChatroomInfo> mChatrooms;    /*!< The chat rooms. */
    LobbyPlayer mFoundPlayer;                 /*!< The player FindPlayer() finds. */
    NetChatroomInfo mFoundChatroom;           /*!< The chat room of mFoundPlayer. */
    String mAnnouncement;                     /*!< The news of the lobby server. */
    String mPolicy;                           /*!< The licence agreement of the lobby server. */
    std::vector<RTFilter> mActiveFilters;     /*!< The filters the server applies. */

private:
    /**
     * Read the name, the port, and the security of the lobby server from `server.cfg`.
     *
     * @ghidraAddress NTSC-U/C: 0x0025ec20
     * @ghidraAddress PAL: 0x00267d00
     */
    void LoadConfig();

    /**
     * Mark each player of a list muted or not.
     *
     * @param pPlayers The players.
     * @ghidraAddress NTSC-U/C: 0x0025f660
     * @ghidraAddress PAL: 0x00268740
     */
    void ApplyMuted(std::list<LobbyPlayer> *pPlayers);

    /**
     * Move the local player to the front of a list.
     *
     * @param pPlayers The players.
     * @ghidraAddress NTSC-U/C: 0x0025f6d8
     * @ghidraAddress PAL: 0x002687b8
     */
    void MoveLocalPlayerToFront(std::list<LobbyPlayer> *pPlayers);

    /**
     * Move mChatroom to the front of mChatrooms.
     *
     * @ghidraAddress NTSC-U/C: 0x0025f798
     * @ghidraAddress PAL: 0x00268878
     */
    void MoveChatroomToFront();

    /**
     * Name the next attempt of the upload of the current remix and start mUploadTask.
     *
     * @param pSink The receiver of the RepoRemixStatusMsg.
     * @return True, with nothing started, after the tenth attempt.
     * @ghidraAddress NTSC-U/C: 0x00260538
     * @ghidraAddress PAL: 0x00269618
     */
    bool StartUpload(MsgSink *pSink);

    /**
     * Report a finished upload, or retry a failed upload of a remix.
     *
     * @param pTask mUploadTask.
     * @ghidraAddress NTSC-U/C: 0x00260760
     * @ghidraAddress PAL: 0x00269840
     */
    void OnUploadDone(NetMultiTask *pTask);

    /**
     * Decode and report a finished download.
     *
     * @param pTask mDownloadTask.
     * @ghidraAddress NTSC-U/C: 0x002609d8
     * @ghidraAddress PAL: 0x00269ab8
     */
    void OnDownloadDone(NetMultiTask *pTask);

    /**
     * Report the files a finished list found.
     *
     * @param pTask mFileListTask.
     * @ghidraAddress NTSC-U/C: 0x00260d80
     * @ghidraAddress PAL: 0x00269e60
     */
    void OnFileListDone(NetMultiTask *pTask);

    /**
     * End the transfer of a finished deletion.
     *
     * @param pTask mDeleteFileTask, not read.
     * @ghidraAddress NTSC-U/C: 0x00260eb0
     * @ghidraAddress PAL: 0x00269fa0
     */
    void OnDeleteFileDone(NetMultiTask *pTask);
};

/**
 * The lobby service.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0af0
 * @ghidraAddress PAL: 0x0042f9bc
 */
extern LobbyRT *TheLobbyRT;

inline void LobbyRT::ReportNetError(int nLine, int nError) {
    if (TheLobbyRT != nullptr) {
        TheLobbyRT->LogNetError(nLine, nError);
    }
}

inline void LobbyRT::ReportMediusError(int nLine, int nError) {
    if (TheLobbyRT != nullptr) {
        TheLobbyRT->LogMediusError(nLine, nError);
    }
}
