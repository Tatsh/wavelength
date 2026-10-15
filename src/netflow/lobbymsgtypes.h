#pragma once

/**
 * Identity that InetConnectStatusMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b08
 * @ghidraAddress PAL: 0x0042f9d4
 */
extern int g_nInetConnectStatusMsgType;

/**
 * Identity that InetConnectResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b0c
 * @ghidraAddress PAL: 0x0042f9d8
 */
extern int g_nInetConnectResultMsgType;

/**
 * Identity that InetDisconnectResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b10
 * @ghidraAddress PAL: 0x0042f9dc
 */
extern int g_nInetDisconnectResultMsgType;

/**
 * Identity that InetDNSLookupResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b14
 * @ghidraAddress PAL: 0x0042f9e0
 */
extern int g_nInetDNSLookupResultMsgType;

/**
 * Identity that InetCheckConnectionResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b18
 * @ghidraAddress PAL: 0x0042f9e4
 */
extern int g_nInetCheckConnectionResultMsgType;

/**
 * Identity that LobbyConnectionLostMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b1c
 * @ghidraAddress PAL: 0x0042f9e8
 */
extern int g_nLobbyConnectionLostMsgType;

/**
 * Identity that LobbyConnectResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b20
 * @ghidraAddress PAL: 0x0042f9ec
 */
extern int g_nLobbyConnectResultMsgType;

#ifdef VIDEO_STANDARD_PAL
/**
 * Identity that DNASStatusMsg::Type() reports.
 *
 * @ghidraAddress PAL: 0x0042f9f0
 */
extern int g_nDNASStatusMsgType;
#endif

/**
 * Identity that CreateAccountResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b24
 * @ghidraAddress PAL: 0x0042f9f4
 */
extern int g_nCreateAccountResultMsgType;

/**
 * Identity that DeleteAccountResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b28
 * @ghidraAddress PAL: 0x0042f9f8
 */
extern int g_nDeleteAccountResultMsgType;

/**
 * Identity that ChangePasswordResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b2c
 * @ghidraAddress PAL: 0x0042f9fc
 */
extern int g_nChangePasswordResultMsgType;

/**
 * Identity that LoginResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b30
 * @ghidraAddress PAL: 0x0042fa00
 */
extern int g_nLoginResultMsgType;

/**
 * Identity that LobbyDisconnectResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b34
 * @ghidraAddress PAL: 0x0042fa04
 */
extern int g_nLobbyDisconnectResultMsgType;

/**
 * Identity that PlayerRankMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b3c
 * @ghidraAddress PAL: 0x0042fa0c
 */
extern int g_nPlayerRankMsgType;

/**
 * Identity that FindPlayerMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b40
 * @ghidraAddress PAL: 0x0042fa10
 */
extern int g_nFindPlayerMsgType;

/**
 * Identity that LobbyChatroomsMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b44
 * @ghidraAddress PAL: 0x0042fa14
 */
extern int g_nLobbyChatroomsMsgType;

/**
 * Identity that LobbyLaunchpadsMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b48
 * @ghidraAddress PAL: 0x0042fa18
 */
extern int g_nLobbyLaunchpadsMsgType;

/**
 * Identity that RepoRemixStatusMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b50
 * @ghidraAddress PAL: 0x0042fa20
 */
extern int g_nRepoRemixStatusMsgType;

/**
 * Identity that RepoRemixesMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b54
 * @ghidraAddress PAL: 0x0042fa24
 */
extern int g_nRepoRemixesMsgType;

/**
 * Identity that FileListMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b58
 * @ghidraAddress PAL: 0x0042fa28
 */
extern int g_nFileListMsgType;

/**
 * Identity that RemixDownloadMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b5c
 * @ghidraAddress PAL: 0x0042fa2c
 */
extern int g_nRemixDownloadMsgType;

/**
 * Identity that LaunchpadAbortedMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b68
 * @ghidraAddress PAL: 0x0042fa38
 */
extern int g_nLaunchpadAbortedMsgType;

/**
 * Identity that GameParamsUpdateMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b6c
 * @ghidraAddress PAL: 0x0042fa3c
 */
extern int g_nGameParamsUpdateMsgType;

/**
 * Identity that ShareRemixBeginMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b70
 * @ghidraAddress PAL: 0x0042fa40
 */
extern int g_nShareRemixBeginMsgType;

/**
 * Identity that ShareRemixProgressMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b74
 * @ghidraAddress PAL: 0x0042fa44
 */
extern int g_nShareRemixProgressMsgType;

/**
 * Identity that LoadGameMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b78
 * @ghidraAddress PAL: 0x0042fa48
 */
extern int g_nLoadGameMsgType;
