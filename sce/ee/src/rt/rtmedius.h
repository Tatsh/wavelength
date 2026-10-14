#ifndef RTMEDIUS_H
#define RTMEDIUS_H

#include <librtdme.h>
#include <librtmedius.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The state the lobby client shares between its entry points and the receivers of the messages of
 * the lobby server. The names of the messages, the receivers, and the globals are inferred, and a
 * message without a known entry point is named by its place in the registration order.
 */

/** Non-zero once rtMediusInitialize() has run. NTSC-U/C: 0x003c25c8. */
extern int g_nMediusInitialized;

/** The connection to the lobby server, or negative. NTSC-U/C: 0x003c25cc. */
extern int g_nMediusConnection;

/** The key of the lobby session, or empty. NTSC-U/C: 0x003c25d8. */
extern char g_szMediusSessionKey[RT_MEDIUS_SESSION_KEY_SIZE];

/** The file rtMediusFileUpload() sends. NTSC-U/C: 0x004357fc. */
extern const char *g_pMediusUploadBuffer;

/** The size of the file rtMediusFileUpload() sends. NTSC-U/C: 0x00435800. */
extern int g_nMediusUploadSize;

/** The registered type of the parts of an upload. NTSC-U/C: 0x00435b14. */
extern int g_nMediusFileUploadPartType;

/** The receiver of the parts of an upload, or null. NTSC-U/C: 0x004357f8. */
extern rtMediusFileTransferCallback g_pfnMediusFileUploadCallback;

/** The receiver of message 74, or null. NTSC-U/C: 0x003c25f0. */
extern rtMediusCallback g_pfnMediusNoticeCallback;

/** The receiver of message 4, or null. NTSC-U/C: 0x003c25f4. */
extern rtMediusSessionBeginCallback g_pfnMediusSessionBeginCallback;

/** The receiver of message 6, or null. NTSC-U/C: 0x003c25f8. */
extern rtMediusCallback g_pfnMediusSessionEndCallback;

/** The receiver of message 8, or null. NTSC-U/C: 0x003c2600. */
extern rtMediusLoginCallback g_pfnMediusAccountLoginCallback;

/** The receiver of message 22, or null. NTSC-U/C: 0x003c2604. */
extern rtMediusCallback g_pfnMediusMessage022Callback;

/** The receiver of message 10, or null. NTSC-U/C: 0x003c2608. */
extern rtMediusCallback g_pfnMediusAccountRegistrationCallback;

/** The receiver of message 12, or null. NTSC-U/C: 0x003c2618. */
extern rtMediusCallback g_pfnMediusMessage012Callback;

/** The receiver of message 14, or null. NTSC-U/C: 0x003c260c. */
extern rtMediusCallback g_pfnMediusMessage014Callback;

/** The receiver of message 16, or null. NTSC-U/C: 0x003c2610. */
extern rtMediusCallback g_pfnMediusChangePasswordCallback;

/** The receiver of message 18, or null. NTSC-U/C: 0x003c2614. */
extern rtMediusCallback g_pfnMediusUpdateStatsCallback;

/** The receiver of message 26, or null. NTSC-U/C: 0x003c2620. */
extern rtMediusLoginCallback g_pfnMediusAnonymousLoginCallback;

/** The receiver of message 30, or null. NTSC-U/C: 0x003c262c. */
extern rtMediusCallback g_pfnMediusMessage030Callback;

/** The receiver of message 32, or null. NTSC-U/C: 0x003c2630. */
extern rtMediusCallback g_pfnMediusMessage032Callback;

/** The receiver of message 34, or null. NTSC-U/C: 0x003c2634. */
extern rtMediusCreateChannelCallback g_pfnMediusCreateChannelCallback;

/** The receiver of message 36, or null. NTSC-U/C: 0x003c2638. */
extern rtMediusJoinGameCallback g_pfnMediusJoinGameCallback;

/** The receiver of message 38, or null. NTSC-U/C: 0x003c263c. */
extern rtMediusJoinChannelCallback g_pfnMediusJoinChannelCallback;

/** The receiver of message 42, or null. NTSC-U/C: 0x003c2640. */
extern rtMediusGameListCallback g_pfnMediusGameListCallback;

/** The receiver of message 44, or null. NTSC-U/C: 0x003c2644. */
extern rtMediusChannelListCallback g_pfnMediusChannelListCallback;

/** The receiver of message 46, or null. NTSC-U/C: 0x003c2624. */
extern rtMediusChannelPlayersCallback g_pfnMediusChannelPlayersCallback;

/** The receiver of message 48, or null. NTSC-U/C: 0x003c2628. */
extern rtMediusGamePlayersCallback g_pfnMediusGamePlayersCallback;

/** The receiver of message 50, or null. NTSC-U/C: 0x003c2648. */
extern rtMediusPlayerInfoCallback g_pfnMediusPlayerInfoCallback;

/** The receiver of message 52, or null. NTSC-U/C: 0x003c264c. */
extern rtMediusCallback g_pfnMediusMessage052Callback;

/** The receiver of message 54, or null. NTSC-U/C: 0x003c2650. */
extern rtMediusChannelInfoCallback g_pfnMediusChannelInfoCallback;

/** The receiver of message 56, or null. NTSC-U/C: 0x003c2654. */
extern rtMediusCallback g_pfnMediusMessage056Callback;

/** The receiver of message 58, or null. NTSC-U/C: 0x003c2658. */
extern rtMediusFindPlayerCallback g_pfnMediusFindPlayerCallback;

/** The receiver of message 60, or null. NTSC-U/C: 0x003c265c. */
extern rtMediusChatHandler g_pfnMediusChatMessageCallback;

/** The receiver of message 62, or null. NTSC-U/C: 0x003c2660. */
extern rtMediusCallback g_pfnMediusMessage062Callback;

/** The receiver of message 64, or null. NTSC-U/C: 0x003c2664. */
extern rtMediusCallback g_pfnMediusMessage064Callback;

/** The receiver of message 66, or null. NTSC-U/C: 0x003c2668. */
extern rtMediusCallback g_pfnMediusMessage066Callback;

/** The receiver of message 68, or null. NTSC-U/C: 0x003c2670. */
extern rtMediusCallback g_pfnMediusMessage068Callback;

/** The receiver of message 69, or null. NTSC-U/C: 0x003c266c. */
extern rtMediusCallback g_pfnMediusInitHandler04Callback;

/** The receiver of message 193, or null. NTSC-U/C: 0x003c2674. */
extern rtMediusCallback g_pfnMediusMessage193Callback;

/** The receiver of message 195, or null. NTSC-U/C: 0x003c2678. */
extern rtMediusCallback g_pfnMediusMessage195Callback;

/** The receiver of message 197, or null. NTSC-U/C: 0x003c267c. */
extern rtMediusCallback g_pfnMediusMessage197Callback;

/** The receiver of message 72, or null. NTSC-U/C: 0x003c2680. */
extern rtMediusPolicyCallback g_pfnMediusGetPolicyCallback;

/** The receiver of message 77, or null. NTSC-U/C: 0x003c2684. */
extern rtMediusAnnouncementCallback g_pfnMediusGetAnnouncementsCallback;

/** The receiver of message 79, or null. NTSC-U/C: 0x003c2688. */
extern rtMediusCallback g_pfnMediusSetGameListFilterCallback;

/** The receiver of message 81, or null. NTSC-U/C: 0x003c268c. */
extern rtMediusCallback g_pfnMediusClearGameListFilterCallback;

/** The receiver of message 83, or null. NTSC-U/C: 0x003c2690. */
extern rtMediusCallback g_pfnMediusMessage083Callback;

/** The receiver of message 85, or null. NTSC-U/C: 0x00435b68. */
extern rtMediusCallback g_pfnMediusMessage085Callback;

/** The receiver of message 87, or null. NTSC-U/C: 0x00435b6c. */
extern rtMediusCallback g_pfnMediusMessage087Callback;

/** The receiver of message 89, or null. NTSC-U/C: 0x00435b70. */
extern rtMediusCallback g_pfnMediusMessage089Callback;

/** The receiver of message 91, or null. NTSC-U/C: 0x00435b74. */
extern rtMediusCallback g_pfnMediusMessage091Callback;

/** The receiver of message 93, or null. NTSC-U/C: 0x00435b78. */
extern rtMediusCallback g_pfnMediusMessage093Callback;

/** The receiver of message 95, or null. NTSC-U/C: 0x00435b7c. */
extern rtMediusCallback g_pfnMediusMessage095Callback;

/** The receiver of message 97, or null. NTSC-U/C: 0x00435b80. */
extern rtMediusCallback g_pfnMediusMessage097Callback;

/** The receiver of message 99, or null. NTSC-U/C: 0x00435b84. */
extern rtMediusCallback g_pfnMediusMessage099Callback;

/** The receiver of message 101, or null. NTSC-U/C: 0x00435b88. */
extern rtMediusCallback g_pfnMediusMessage101Callback;

/** The receiver of message 123, or null. NTSC-U/C: 0x00435b8c. */
extern rtMediusCallback g_pfnMediusMessage123Callback;

/** The receiver of message 103, or null. NTSC-U/C: 0x00435b90. */
extern rtMediusCallback g_pfnMediusMessage103Callback;

/** The receiver of message 105, or null. NTSC-U/C: 0x00435b94. */
extern rtMediusCallback g_pfnMediusMessage105Callback;

/** The receiver of message 107, or null. NTSC-U/C: 0x00435b98. */
extern rtMediusCallback g_pfnMediusMessage107Callback;

/** The receiver of message 109, or null. NTSC-U/C: 0x00435b9c. */
extern rtMediusCallback g_pfnMediusMessage109Callback;

/** The receiver of message 111, or null. NTSC-U/C: 0x00435ba0. */
extern rtMediusCallback g_pfnMediusMessage111Callback;

/** The receiver of message 113, or null. NTSC-U/C: 0x00435ba4. */
extern rtMediusCallback g_pfnMediusMessage113Callback;

/** The receiver of message 115, or null. NTSC-U/C: 0x00435ba8. */
extern rtMediusCallback g_pfnMediusMessage115Callback;

/** The receiver of message 117, or null. NTSC-U/C: 0x00435bac. */
extern rtMediusCallback g_pfnMediusMessage117Callback;

/** The receiver of message 119, or null. NTSC-U/C: 0x00435bb0. */
extern rtMediusCallback g_pfnMediusMessage119Callback;

/** The receiver of message 125, or null. NTSC-U/C: 0x00435bb4. */
extern rtMediusCallback g_pfnMediusMessage125Callback;

/** The receiver of message 131, or null. NTSC-U/C: 0x00435bb8. */
extern rtMediusCallback g_pfnMediusMessage131Callback;

/** The receiver of message 129, or null. NTSC-U/C: 0x00435bbc. */
extern rtMediusCallback g_pfnMediusMessage129Callback;

/** The receiver of message 127, or null. NTSC-U/C: 0x00435bc0. */
extern rtMediusCallback g_pfnMediusMessage127Callback;

/** The receiver of message 121, or null. NTSC-U/C: 0x00435bc4. */
extern rtMediusCallback g_pfnMediusMessage121Callback;

/** The receiver of message 133, or null. NTSC-U/C: 0x00435bc8. */
extern rtMediusCallback g_pfnMediusMessage133Callback;

/** The receiver of message 28, or null. NTSC-U/C: 0x003c2698. */
extern rtMediusCallback g_pfnMediusMessage028Callback;

/** The receiver of message 135, or null. NTSC-U/C: 0x003c2694. */
extern rtMediusVersionCallback g_pfnMediusVersionServerCallback;

/** The receiver of message 137, or null. NTSC-U/C: 0x003c269c. */
extern rtMediusCallback g_pfnMediusMessage137Callback;

/** The receiver of message 139, or null. NTSC-U/C: 0x003c26a0. */
extern rtMediusCallback g_pfnMediusBanPlayerCallback;

/** The receiver of message 20, or null. NTSC-U/C: 0x003c261c. */
extern rtMediusCallback g_pfnMediusAccountDeleteCallback;

/** The receiver of message 24, or null. NTSC-U/C: 0x003c25fc. */
extern rtMediusAccountGetIdCallback g_pfnMediusAccountGetIdCallback;

/** The receiver of message 141, or null. NTSC-U/C: 0x003c26a4. */
extern rtMediusCallback g_pfnMediusMessage141Callback;

/** The receiver of message 143, or null. NTSC-U/C: 0x003c26a8. */
extern rtMediusCallback g_pfnMediusMessage143Callback;

/** The receiver of message 145, or null. NTSC-U/C: 0x00435bcc. */
extern rtMediusCallback g_pfnMediusMessage145Callback;

/** The receiver of message 147, or null. NTSC-U/C: 0x003c26ac. */
extern rtMediusLadderPositionCallback g_pfnMediusLadderPositionCallback;

/** The receiver of message 205, or null. NTSC-U/C: 0x003c26b0. */
extern rtMediusLadderPositionCallback g_pfnMediusLadderPositionFastCallback;

/** The receiver of message 149, or null. NTSC-U/C: 0x003c26b4. */
extern rtMediusLadderListCallback g_pfnMediusLadderListCallback;

/** The receiver of message 151, or null. NTSC-U/C: 0x003c26b8. */
extern rtMediusCallback g_pfnMediusMessage151Callback;

/** The receiver of message 153, or null. NTSC-U/C: 0x003c26bc. */
extern rtMediusTextFilterCallback g_pfnMediusTextFilterCallback;

/** The receiver of message 154, or null. NTSC-U/C: 0x003c26c0. */
extern rtMediusCallback g_pfnMediusInitHandler0CCallback;

/** The receiver of message 156, or null. NTSC-U/C: 0x003c26c4. */
extern rtMediusCallback g_pfnMediusMessage156Callback;

/** The receiver of message 158, or null. NTSC-U/C: 0x003c26c8. */
extern rtMediusCallback g_pfnMediusMessage158Callback;

/** The receiver of message 160, or null. NTSC-U/C: 0x003c26cc. */
extern rtMediusCallback g_pfnMediusMessage160Callback;

/** The receiver of message 162, or null. NTSC-U/C: 0x003c26d0. */
extern rtMediusCallback g_pfnMediusMessage162Callback;

/** The receiver of message 164, or null. NTSC-U/C: 0x003c26d4. */
extern rtMediusCallback g_pfnMediusMessage164Callback;

/** The receiver of message 199, or null. NTSC-U/C: 0x003c26d8. */
extern rtMediusCallback g_pfnMediusMessage199Callback;

/** The receiver of message 166, or null. NTSC-U/C: 0x004357f4. */
extern rtMediusFileCallback g_pfnMediusFileCreateCallback;

/** The receiver of message 171, or null. NTSC-U/C: 0x00435804. */
extern rtMediusFileCallback g_pfnMediusFileCloseCallback;

/** The receiver of message 173, or null. NTSC-U/C: 0x00435808. */
extern rtMediusFileTransferCallback g_pfnMediusFileDownloadCallback;

/** The receiver of message 175, or null. NTSC-U/C: 0x0043580c. */
extern rtMediusCallback g_pfnMediusMessage175Callback;

/** The receiver of message 177, or null. NTSC-U/C: 0x00435810. */
extern rtMediusFileDeleteCallback g_pfnMediusFileDeleteCallback;

/** The receiver of message 179, or null. NTSC-U/C: 0x00435814. */
extern rtMediusFileCallback g_pfnMediusFileListCallback;

/** The receiver of message 181, or null. NTSC-U/C: 0x00435818. */
extern rtMediusCallback g_pfnMediusMessage181Callback;

/** The receiver of message 183, or null. NTSC-U/C: 0x0043581c. */
extern rtMediusFileAttributesCallback g_pfnMediusFileGetAttributesCallback;

/** The receiver of message 185, or null. NTSC-U/C: 0x00435820. */
extern rtMediusCallback g_pfnMediusMessage185Callback;

/** The receiver of message 187, or null. NTSC-U/C: 0x00435824. */
extern rtMediusCallback g_pfnMediusMessage187Callback;

/** The receiver of message 189, or null. NTSC-U/C: 0x00435828. */
extern rtMediusCallback g_pfnMediusMessage189Callback;

/** The receiver of message 191, or null. NTSC-U/C: 0x0043582c. */
extern rtMediusCallback g_pfnMediusMessage191Callback;

/** The receiver of message 200, or null. NTSC-U/C: 0x003c26dc. */
extern rtMediusCallback g_pfnMediusMessage200Callback;

/** The receiver of message 201, or null. NTSC-U/C: 0x003c26e0. */
extern rtMediusCallback g_pfnMediusMessage201Callback;

/** The receiver of message 202, or null. NTSC-U/C: 0x003c26e4. */
extern rtMediusCallback g_pfnMediusMessage202Callback;

/**
 * Store the address of the lobby server a login reports.
 *
 * @param address The address.
 * @ghidraAddress NTSC-U/C: 0x002f98b8
 */
void rtMediusSetServerAddress(const rtNetAddress *address);

/**
 * Receive message 74 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301938
 */
int rtMediusOnNotice(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 0 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301968
 */
int rtMediusOnMessage000(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 1 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301970
 */
int rtMediusOnMessage001(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 2 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301978
 */
int rtMediusOnMessage002(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 3 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301980
 */
int rtMediusOnMessage003(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 4 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301988
 */
int rtMediusOnSessionBeginResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 5 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301a10
 */
int rtMediusOnMessage005(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 6 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301a18
 */
int rtMediusOnSessionEndResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 203 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301a60
 */
int rtMediusOnMessage203(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 7 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301a68
 */
int rtMediusOnMessage007(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 8 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301a70
 */
int rtMediusOnAccountLoginResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 21 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301af0
 */
int rtMediusOnMessage021(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 22 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301af8
 */
int rtMediusOnMessage022(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 9 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301b28
 */
int rtMediusOnMessage009(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 10 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301b30
 */
int rtMediusOnAccountRegistrationResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message);

/**
 * Receive message 11 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301b60
 */
int rtMediusOnMessage011(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 12 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301b68
 */
int rtMediusOnMessage012(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 13 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301b98
 */
int rtMediusOnMessage013(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 14 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301ba0
 */
int rtMediusOnMessage014(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 15 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301bd0
 */
int rtMediusOnMessage015(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 16 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301bd8
 */
int rtMediusOnChangePasswordResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 17 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301c08
 */
int rtMediusOnMessage017(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 18 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301c10
 */
int rtMediusOnUpdateStatsResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 25 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301c40
 */
int rtMediusOnMessage025(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 26 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301c48
 */
int rtMediusOnAnonymousLoginResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 29 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301c78
 */
int rtMediusOnMessage029(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 30 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301c80
 */
int rtMediusOnMessage030(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 31 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301cb0
 */
int rtMediusOnMessage031(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 32 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301cb8
 */
int rtMediusOnMessage032(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 33 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301ce8
 */
int rtMediusOnMessage033(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 34 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301cf0
 */
int rtMediusOnCreateChannelResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 35 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301d20
 */
int rtMediusOnMessage035(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 36 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301d28
 */
int rtMediusOnJoinGameResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 37 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301d58
 */
int rtMediusOnMessage037(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 38 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301d60
 */
int rtMediusOnJoinChannelResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 39 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301d90
 */
int rtMediusOnMessage039(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 40 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301d98
 */
int rtMediusOnMessage040(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 41 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301da0
 */
int rtMediusOnMessage041(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 42 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301da8
 */
int rtMediusOnGameListResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 43 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301dd8
 */
int rtMediusOnMessage043(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 44 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301de0
 */
int rtMediusOnChannelListResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 45 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301e10
 */
int rtMediusOnMessage045(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 46 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301e18
 */
int rtMediusOnChannelPlayersResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 47 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301e48
 */
int rtMediusOnMessage047(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 48 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301e50
 */
int rtMediusOnGamePlayersResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 49 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301e80
 */
int rtMediusOnMessage049(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 50 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301e88
 */
int rtMediusOnPlayerInfoResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 51 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301eb8
 */
int rtMediusOnMessage051(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 52 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301ec0
 */
int rtMediusOnMessage052(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 53 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301ef0
 */
int rtMediusOnMessage053(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 54 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301ef8
 */
int rtMediusOnChannelInfoResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 55 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301f28
 */
int rtMediusOnMessage055(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 56 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301f30
 */
int rtMediusOnMessage056(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 57 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301f60
 */
int rtMediusOnMessage057(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 58 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301f68
 */
int rtMediusOnFindPlayerResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 59 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301f98
 */
int rtMediusOnMessage059(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 60 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301fa0
 */
int rtMediusOnChatMessage(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 61 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301fd0
 */
int rtMediusOnMessage061(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 62 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301fd8
 */
int rtMediusOnMessage062(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 63 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302008
 */
int rtMediusOnMessage063(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 64 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302010
 */
int rtMediusOnMessage064(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 65 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302040
 */
int rtMediusOnMessage065(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 66 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302048
 */
int rtMediusOnMessage066(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 67 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302078
 */
int rtMediusOnMessage067(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 68 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302080
 */
int rtMediusOnMessage068(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 69 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003020b0
 */
int rtMediusOnInitHandler04(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 70 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003020e0
 */
int rtMediusOnMessage070(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 192 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003020e8
 */
int rtMediusOnMessage192(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 193 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003020f0
 */
int rtMediusOnMessage193(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 194 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302120
 */
int rtMediusOnMessage194(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 195 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302128
 */
int rtMediusOnMessage195(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 196 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302158
 */
int rtMediusOnMessage196(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 197 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302160
 */
int rtMediusOnMessage197(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 71 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302190
 */
int rtMediusOnMessage071(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 72 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302198
 */
int rtMediusOnGetPolicyResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 73 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003021c8
 */
int rtMediusOnMessage073(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 75 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003021d0
 */
int rtMediusOnMessage075(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 76 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003021d8
 */
int rtMediusOnMessage076(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 77 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003021e0
 */
int rtMediusOnGetAnnouncementsResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 78 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302210
 */
int rtMediusOnMessage078(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 79 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302218
 */
int rtMediusOnSetGameListFilterResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message);

/**
 * Receive message 80 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302248
 */
int rtMediusOnMessage080(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 81 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302250
 */
int rtMediusOnClearGameListFilterResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message);

/**
 * Receive message 82 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302280
 */
int rtMediusOnMessage082(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 83 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302288
 */
int rtMediusOnMessage083(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 84 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003022b8
 */
int rtMediusOnMessage084(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 85 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003022c0
 */
int rtMediusOnMessage085(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 86 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003022f0
 */
int rtMediusOnMessage086(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 87 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003022f8
 */
int rtMediusOnMessage087(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 88 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302328
 */
int rtMediusOnMessage088(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 89 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302330
 */
int rtMediusOnMessage089(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 90 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302360
 */
int rtMediusOnMessage090(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 91 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302368
 */
int rtMediusOnMessage091(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 92 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302398
 */
int rtMediusOnMessage092(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 93 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003023a0
 */
int rtMediusOnMessage093(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 94 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003023d0
 */
int rtMediusOnMessage094(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 95 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003023d8
 */
int rtMediusOnMessage095(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 96 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302408
 */
int rtMediusOnMessage096(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 97 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302410
 */
int rtMediusOnMessage097(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 98 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302440
 */
int rtMediusOnMessage098(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 99 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302448
 */
int rtMediusOnMessage099(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 100 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302478
 */
int rtMediusOnMessage100(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 101 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302480
 */
int rtMediusOnMessage101(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 122 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003024b0
 */
int rtMediusOnMessage122(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 123 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003024b8
 */
int rtMediusOnMessage123(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 102 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003024e8
 */
int rtMediusOnMessage102(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 103 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003024f0
 */
int rtMediusOnMessage103(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 104 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302520
 */
int rtMediusOnMessage104(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 105 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302528
 */
int rtMediusOnMessage105(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 106 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302558
 */
int rtMediusOnMessage106(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 107 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302560
 */
int rtMediusOnMessage107(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 108 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302590
 */
int rtMediusOnMessage108(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 109 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302598
 */
int rtMediusOnMessage109(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 110 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003025c8
 */
int rtMediusOnMessage110(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 111 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003025d0
 */
int rtMediusOnMessage111(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 112 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302600
 */
int rtMediusOnMessage112(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 113 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302608
 */
int rtMediusOnMessage113(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 114 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302638
 */
int rtMediusOnMessage114(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 115 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302640
 */
int rtMediusOnMessage115(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 116 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302670
 */
int rtMediusOnMessage116(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 117 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302678
 */
int rtMediusOnMessage117(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 118 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003026a8
 */
int rtMediusOnMessage118(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 119 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003026b0
 */
int rtMediusOnMessage119(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 124 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003026e0
 */
int rtMediusOnMessage124(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 125 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003026e8
 */
int rtMediusOnMessage125(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 130 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302718
 */
int rtMediusOnMessage130(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 131 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302720
 */
int rtMediusOnMessage131(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 128 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302750
 */
int rtMediusOnMessage128(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 129 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302758
 */
int rtMediusOnMessage129(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 126 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302788
 */
int rtMediusOnMessage126(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 127 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302790
 */
int rtMediusOnMessage127(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 120 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003027c0
 */
int rtMediusOnMessage120(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 121 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003027c8
 */
int rtMediusOnMessage121(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 132 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003027f8
 */
int rtMediusOnMessage132(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 133 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302800
 */
int rtMediusOnMessage133(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 27 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302830
 */
int rtMediusOnMessage027(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 28 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302838
 */
int rtMediusOnMessage028(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 134 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302868
 */
int rtMediusOnMessage134(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 135 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302870
 */
int rtMediusOnVersionServerResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 136 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003028a0
 */
int rtMediusOnMessage136(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 137 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003028a8
 */
int rtMediusOnMessage137(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 138 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003028d8
 */
int rtMediusOnMessage138(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 139 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003028e0
 */
int rtMediusOnBanPlayerResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 19 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302910
 */
int rtMediusOnMessage019(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 20 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302918
 */
int rtMediusOnAccountDeleteResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 23 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302948
 */
int rtMediusOnMessage023(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 24 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302950
 */
int rtMediusOnAccountGetIdResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 140 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302980
 */
int rtMediusOnMessage140(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 141 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302988
 */
int rtMediusOnMessage141(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 142 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003029b8
 */
int rtMediusOnMessage142(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 143 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003029c0
 */
int rtMediusOnMessage143(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 144 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003029f0
 */
int rtMediusOnMessage144(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 145 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003029f8
 */
int rtMediusOnMessage145(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 146 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302a28
 */
int rtMediusOnMessage146(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 147 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302a30
 */
int rtMediusOnLadderPositionResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 204 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302a60
 */
int rtMediusOnMessage204(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 205 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302a68
 */
int rtMediusOnLadderPositionFastResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message);

/**
 * Receive message 148 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302a98
 */
int rtMediusOnMessage148(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 149 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302aa0
 */
int rtMediusOnLadderListResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 150 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302ad0
 */
int rtMediusOnMessage150(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 151 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302ad8
 */
int rtMediusOnMessage151(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 152 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302b08
 */
int rtMediusOnMessage152(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 153 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302b10
 */
int rtMediusOnTextFilterResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 154 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302b40
 */
int rtMediusOnInitHandler0C(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 155 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302b70
 */
int rtMediusOnMessage155(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 156 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302b78
 */
int rtMediusOnMessage156(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 157 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302ba8
 */
int rtMediusOnMessage157(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 158 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302bb0
 */
int rtMediusOnMessage158(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 159 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302be0
 */
int rtMediusOnMessage159(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 160 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302be8
 */
int rtMediusOnMessage160(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 161 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302c18
 */
int rtMediusOnMessage161(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 162 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302c20
 */
int rtMediusOnMessage162(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 163 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302c50
 */
int rtMediusOnMessage163(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 164 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302c58
 */
int rtMediusOnMessage164(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 198 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302c88
 */
int rtMediusOnMessage198(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 199 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302c90
 */
int rtMediusOnMessage199(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 165 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302cc0
 */
int rtMediusOnMessage165(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 166 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302cc8
 */
int rtMediusOnFileCreateResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 167 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302cf8
 */
int rtMediusOnMessage167(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 168 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302d00
 */
int rtMediusOnMessage168(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 170 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302e58
 */
int rtMediusOnMessage170(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 171 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302e60
 */
int rtMediusOnFileCloseResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 172 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302e90
 */
int rtMediusOnMessage172(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 173 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302e98
 */
int rtMediusOnFileDownloadResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 174 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302ec8
 */
int rtMediusOnMessage174(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 175 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302ed0
 */
int rtMediusOnMessage175(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 176 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302f00
 */
int rtMediusOnMessage176(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 177 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302f08
 */
int rtMediusOnFileDeleteResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 178 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302f38
 */
int rtMediusOnMessage178(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 179 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302f40
 */
int rtMediusOnFileListResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 180 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302f70
 */
int rtMediusOnMessage180(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 181 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302f78
 */
int rtMediusOnMessage181(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 182 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302fa8
 */
int rtMediusOnMessage182(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 183 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302fb0
 */
int rtMediusOnFileGetAttributesResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message);

/**
 * Receive message 184 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302fe0
 */
int rtMediusOnMessage184(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 185 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302fe8
 */
int rtMediusOnMessage185(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 186 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303018
 */
int rtMediusOnMessage186(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 187 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303020
 */
int rtMediusOnMessage187(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 188 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303050
 */
int rtMediusOnMessage188(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 189 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303058
 */
int rtMediusOnMessage189(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 190 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303088
 */
int rtMediusOnMessage190(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 191 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303090
 */
int rtMediusOnMessage191(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 200 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003030c0
 */
int rtMediusOnMessage200(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 201 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003030f0
 */
int rtMediusOnMessage201(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 202 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303120
 */
int rtMediusOnMessage202(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Send the lobby server the next part of the file of rtMediusFileUpload() and report the size of
 * the request.
 *
 * @ghidraAddress NTSC-U/C: 0x00302d08
 */
int rtMediusOnFileUploadRequest(int connection, int type, int sender, rtDmeMessage *message);

#ifdef __cplusplus
}
#endif

#endif
