#pragma once

/**
 * Identity that LobbyConnectionLostMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b1c
 */
extern int g_nLobbyConnectionLostMsgType;

/**
 * Identity that LaunchpadAbortedMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b68
 */
extern int g_nLaunchpadAbortedMsgType;

/**
 * Identity that GameParamsUpdateMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b6c
 */
extern int g_nGameParamsUpdateMsgType;

/**
 * Identity that ShareRemixBeginMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b70
 */
extern int g_nShareRemixBeginMsgType;

/**
 * Identity that ShareRemixProgressMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b74
 */
extern int g_nShareRemixProgressMsgType;

/**
 * Identity that LoadGameMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b78
 */
extern int g_nLoadGameMsgType;
