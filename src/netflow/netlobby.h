#pragma once

#include "app/msgsink.h"
#include "game/campaign.h"
#include "os/string.h"

/**
 * The online lobby service: accounts, chat rooms, launchpads, and the remix repository.
 *
 * The RTTI includes the name. LobbyRT implements it, and TheNetLobby points at the one instance.
 * Each request reports its result to the receiver it is given, as a message such as
 * LobbyConnectResultMsg or LoginResultMsg. The member names other than SetSink() are inferred.
 */
class NetLobby {
public:
    /** Release the service. */
    virtual ~NetLobby();

    /**
     * Make a sink the receiver of the lobby's result messages. Vtable slot 2.
     *
     * @param pSink The sink.
     * @ghidraAddress NTSC-U/C: 0x0039afb0
     * @ghidraAddress PAL: 0x00409bf8
     */
    virtual void SetSink(MsgSink *pSink);

    /**
     * Take the name, the password, and the Freq of the local player.
     *
     * @param pProfile The profile of the player.
     */
    virtual void SetProfile(Campaign *pProfile) = 0;

    /**
     * Start connecting to the lobby server.
     *
     * @param pSink The receiver of the LobbyConnectResultMsg.
     */
    virtual void Connect(MsgSink *pSink) = 0;

    /**
     * Start logging in the local player.
     *
     * @param pSink The receiver of the LoginResultMsg.
     * @param nLocation The identifier of the lobby location the player chose.
     * @param pszPassword The password.
     */
    virtual void Login(MsgSink *pSink, int nLocation, const char *pszPassword) = 0;

    /**
     * Start disconnecting from the lobby server.
     *
     * @param pSink The receiver of the LobbyDisconnectResultMsg.
     */
    virtual void Disconnect(MsgSink *pSink) = 0;

    /**
     * Report the account of the local player.
     *
     * @return The account identifier.
     */
    virtual int GetAccountId() = 0;

    /**
     * Start creating an account for the local player.
     *
     * @param pSink The receiver of the CreateAccountResultMsg.
     * @param pszPassword The password of the account.
     */
    virtual void CreateAccount(MsgSink *pSink, const char *pszPassword) = 0;

    /**
     * Start deleting the account of the local player.
     *
     * @param pSink The receiver of the DeleteAccountResultMsg.
     */
    virtual void DeleteAccount(MsgSink *pSink) = 0;

    /**
     * Start changing the password of the local player.
     *
     * @param pSink The receiver of the ChangePasswordResultMsg.
     * @param pszOldPassword The current password.
     * @param pszNewPassword The new password.
     */
    virtual void
    ChangePassword(MsgSink *pSink, const char *pszOldPassword, const char *pszNewPassword) = 0;

    /**
     * Start listing the players of the current chat room.
     *
     * @param pSink The receiver of the LobbyPlayersMsg.
     */
    virtual void RequestLobbyPlayers(MsgSink *pSink) = 0;

    /**
     * Start listing the players of a launchpad.
     *
     * The name is inferred.
     *
     * @param pSink The receiver of the LobbyPlayersMsg.
     * @param nLaunchpad The identifier of the launchpad.
     */
    virtual void RequestLaunchpadPlayers(MsgSink *pSink, int nLaunchpad) = 0;

    /**
     * Add a listener the service reports ladder positions to.
     *
     * @param pSink The listener.
     * @param nAccount The account of the player.
     */
    virtual void AddLadderListener(MsgSink *pSink, int nAccount) = 0;

    /**
     * Mute or unmute the chat of a player.
     *
     * @param nAccount The account of the player.
     * @param bMuted Whether the player is muted.
     */
    virtual void SetMuted(int nAccount, bool bMuted) = 0;

    /**
     * Start listing the launchpads of the lobby.
     *
     * The main lobby passes 4, 1, an empty arena, 0, and -1.
     *
     * @param pSink The receiver of the LobbyLaunchpadsMsg.
     * @param nStart The position of the list, or a count.
     * @param nMode Not yet identified.
     * @param pszArena The arena of the search, or an empty string for any.
     * @param nRuleSet The game mode of the search.
     * @param nSkillLevel The skill level of the search, or -1 for any.
     */
    virtual void RequestLaunchpads(MsgSink *pSink,
                                   int nStart,
                                   int nMode,
                                   const char *pszArena,
                                   int nRuleSet,
                                   int nSkillLevel) = 0;

    /**
     * Start finding a player by name.
     *
     * @param pSink The receiver of the FindPlayerMsg.
     * @param pszName The name of the player.
     */
    virtual void FindPlayer(MsgSink *pSink, const char *pszName) = 0;

    /**
     * Start listing the chat rooms of the lobby.
     *
     * @param pSink The receiver of the LobbyChatroomsMsg.
     */
    virtual void RequestChatrooms(MsgSink *pSink) = 0;

    /**
     * Join a chat room. Vtable slot 18.
     *
     * The result arrives at pSink as a JoinChatroomResultMsg. The name is inferred.
     *
     * @param pSink The sink that receives the result.
     * @param nChatroomId The identifier of the room.
     * @ghidraAddress NTSC-U/C: 0x00260238
     * @ghidraAddress PAL: 0x00269318
     */
    virtual void JoinChatroom(MsgSink *pSink, int nChatroomId);

    /**
     * Start creating a chat room and joining it.
     *
     * @param pSink The receiver of the JoinChatroomResultMsg.
     * @param name The name of the chat room.
     */
    virtual void CreateChatroom(MsgSink *pSink, const String &name) = 0;

    /**
     * Start listing a page of the ranking.
     *
     * @param pSink The receiver of the LobbyPlayersMsg.
     * @param nCount The most players to list.
     * @param nDirection 0 for the top of the ranking, 1 for the next page, or 3 for the previous
     * page.
     * @param nMode Not yet identified. The ten best players are listed with 1, the ranking screen
     * with 0.
     */
    virtual void RequestRanks(MsgSink *pSink, int nCount, int nDirection, int nMode) = 0;

    /**
     * Start uploading the current remix to the repository.
     *
     * @param pSink The receiver of the RepoRemixStatusMsg.
     * @param pszNote The note of the remix.
     */
    virtual void UploadRemix(MsgSink *pSink, const char *pszNote) = 0;

    /**
     * Start listing the remixes of the repository.
     *
     * @param pSink The receiver of the RepoRemixesMsg.
     */
    virtual void RequestRemixes(MsgSink *pSink) = 0;

    /**
     * Start downloading a remix from the repository.
     *
     * @param pSink The receiver of the RemixDownloadMsg.
     * @param pszName The name of the remix.
     */
    virtual void DownloadRemix(MsgSink *pSink, const char *pszName) = 0;

    /**
     * Start listing the files of the repository.
     *
     * @param pSink The receiver of the FileListMsg.
     * @param nOwn Non-zero to list only the files of the local player.
     */
    virtual void RequestFiles(MsgSink *pSink, int nOwn) = 0;

    /**
     * Start uploading the remix directory to the repository.
     *
     * @param pSink The receiver of the RepoRemixStatusMsg.
     * @param pszName The name of the remix.
     */
    virtual void UploadDirectory(MsgSink *pSink, const char *pszName) = 0;

    /**
     * Start deleting a file of the repository.
     *
     * @param pSink The receiver of the RepoRemixStatusMsg.
     * @param pszName The name of the file.
     */
    virtual void DeleteFile(MsgSink *pSink, const char *pszName) = 0;

    MsgSink *mSink; /*!< The receiver of the result messages. +0x04 */
};

/**
 * The lobby service.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0aec
 */
extern NetLobby *TheNetLobby;
