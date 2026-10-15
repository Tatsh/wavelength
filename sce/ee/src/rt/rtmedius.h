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

/** 1 once rtMediusInitialize() has run. NTSC-U/C: 0x003c25c8. */
extern short g_nMediusInitialized;

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
extern rtMediusFileTransferCallback g_pfnMediusFileUploadServerReqCallback;

/** The receiver of message 74, or null. NTSC-U/C: 0x003c25f0. */
extern rtMediusCallback g_pfnMediusErrorMessageCallback;

/** The receiver of message 4, or null. NTSC-U/C: 0x003c25f4. */
extern rtMediusSessionBeginCallback g_pfnMediusSessionBeginCallback;

/** The receiver of message 6, or null. NTSC-U/C: 0x003c25f8. */
extern rtMediusCallback g_pfnMediusSessionEndCallback;

/** The receiver of message 8, or null. NTSC-U/C: 0x003c2600. */
extern rtMediusLoginCallback g_pfnMediusAccountLoginCallback;

/** The receiver of message 22, or null. NTSC-U/C: 0x003c2604. */
extern rtMediusCallback g_pfnMediusAccountLogoutCallback;

/** The receiver of message 10, or null. NTSC-U/C: 0x003c2608. */
extern rtMediusCallback g_pfnMediusAccountRegistrationCallback;

/** The receiver of message 12, or null. NTSC-U/C: 0x003c2618. */
extern rtMediusCallback g_pfnMediusAccountGetProfileCallback;

/** The receiver of message 14, or null. NTSC-U/C: 0x003c260c. */
extern rtMediusCallback g_pfnMediusAccountUpdateProfileCallback;

/** The receiver of message 16, or null. NTSC-U/C: 0x003c2610. */
extern rtMediusCallback g_pfnMediusChangePasswordCallback;

/** The receiver of message 18, or null. NTSC-U/C: 0x003c2614. */
extern rtMediusCallback g_pfnMediusAccountUpdateStatsCallback;

/** The receiver of message 26, or null. NTSC-U/C: 0x003c2620. */
extern rtMediusLoginCallback g_pfnMediusAnonymousLoginCallback;

/** The receiver of message 30, or null. NTSC-U/C: 0x003c262c. */
extern rtMediusCallback g_pfnMediusCreateGameCallback;

/** The receiver of message 32, or null. NTSC-U/C: 0x003c2630. */
extern rtMediusCallback g_pfnMediusCreateGameOnSelfCallback;

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
extern rtMediusChannelPlayersCallback g_pfnMediusLobbyWorldPlayerListCallback;

/** The receiver of message 48, or null. NTSC-U/C: 0x003c2628. */
extern rtMediusGamePlayersCallback g_pfnMediusGameWorldPlayerListCallback;

/** The receiver of message 50, or null. NTSC-U/C: 0x003c2648. */
extern rtMediusPlayerInfoCallback g_pfnMediusPlayerInfoCallback;

/** The receiver of message 52, or null. NTSC-U/C: 0x003c264c. */
extern rtMediusCallback g_pfnMediusGameInfoResponse0Callback;

/** The receiver of message 54, or null. NTSC-U/C: 0x003c2650. */
extern rtMediusChannelInfoCallback g_pfnMediusChannelInfoCallback;

/** The receiver of message 56, or null. NTSC-U/C: 0x003c2654. */
extern rtMediusCallback g_pfnMediusFindWorldByNameCallback;

/** The receiver of message 58, or null. NTSC-U/C: 0x003c2658. */
extern rtMediusFindPlayerCallback g_pfnMediusFindPlayerCallback;

/** The receiver of message 60, or null. NTSC-U/C: 0x003c265c. */
extern rtMediusChatHandler g_pfnMediusChatFwdMessageCallback;

/** The receiver of message 62, or null. NTSC-U/C: 0x003c2660. */
extern rtMediusCallback g_pfnMediusGetBuddyListCallback;

/** The receiver of message 64, or null. NTSC-U/C: 0x003c2664. */
extern rtMediusCallback g_pfnMediusAddToBuddyListCallback;

/** The receiver of message 66, or null. NTSC-U/C: 0x003c2668. */
extern rtMediusCallback g_pfnMediusRemoveFromBuddyListCallback;

/** The receiver of message 68, or null. NTSC-U/C: 0x003c2670. */
extern rtMediusCallback g_pfnMediusAddToBuddyListConfirmationCallback;

/** The receiver of message 69, or null. NTSC-U/C: 0x003c266c. */
extern rtMediusCallback g_pfnMediusAddToBuddyListFwdConfirmationRequest0Callback;

/** The receiver of message 193, or null. NTSC-U/C: 0x003c2674. */
extern rtMediusCallback g_pfnMediusGetIgnoreListCallback;

/** The receiver of message 195, or null. NTSC-U/C: 0x003c2678. */
extern rtMediusCallback g_pfnMediusAddToIgnoreListCallback;

/** The receiver of message 197, or null. NTSC-U/C: 0x003c267c. */
extern rtMediusCallback g_pfnMediusRemoveFromIgnoreListCallback;

/** The receiver of message 72, or null. NTSC-U/C: 0x003c2680. */
extern rtMediusPolicyCallback g_pfnMediusPolicyCallback;

/** The receiver of message 77, or null. NTSC-U/C: 0x003c2684. */
extern rtMediusAnnouncementCallback g_pfnMediusGetAnnouncementsCallback;

/** The receiver of message 79, or null. NTSC-U/C: 0x003c2688. */
extern rtMediusCallback g_pfnMediusSetGameListFilterResponse0Callback;

/** The receiver of message 81, or null. NTSC-U/C: 0x003c268c. */
extern rtMediusCallback g_pfnMediusClearGameListFilterCallback;

/** The receiver of message 83, or null. NTSC-U/C: 0x003c2690. */
extern rtMediusCallback g_pfnMediusGetGameListFilterResponse0Callback;

/** The receiver of message 85, or null. NTSC-U/C: 0x00435b68. */
extern rtMediusCallback g_pfnMediusCreateClanCallback;

/** The receiver of message 87, or null. NTSC-U/C: 0x00435b6c. */
extern rtMediusCallback g_pfnMediusDisbandClanCallback;

/** The receiver of message 89, or null. NTSC-U/C: 0x00435b70. */
extern rtMediusCallback g_pfnMediusGetClanByIDCallback;

/** The receiver of message 91, or null. NTSC-U/C: 0x00435b74. */
extern rtMediusCallback g_pfnMediusGetClanByNameCallback;

/** The receiver of message 93, or null. NTSC-U/C: 0x00435b78. */
extern rtMediusCallback g_pfnMediusTransferClanLeadershipCallback;

/** The receiver of message 95, or null. NTSC-U/C: 0x00435b7c. */
extern rtMediusCallback g_pfnMediusAddPlayerToClanCallback;

/** The receiver of message 97, or null. NTSC-U/C: 0x00435b80. */
extern rtMediusCallback g_pfnMediusRemovePlayerFromClanCallback;

/** The receiver of message 99, or null. NTSC-U/C: 0x00435b84. */
extern rtMediusCallback g_pfnMediusInvitePlayerToClanCallback;

/** The receiver of message 101, or null. NTSC-U/C: 0x00435b88. */
extern rtMediusCallback g_pfnMediusCheckMyClanInvitationsCallback;

/** The receiver of message 123, or null. NTSC-U/C: 0x00435b8c. */
extern rtMediusCallback g_pfnMediusGetClanInvitationsSentCallback;

/** The receiver of message 103, or null. NTSC-U/C: 0x00435b90. */
extern rtMediusCallback g_pfnMediusRespondToClanInvitationCallback;

/** The receiver of message 105, or null. NTSC-U/C: 0x00435b94. */
extern rtMediusCallback g_pfnMediusRevokeClanInvitationCallback;

/** The receiver of message 107, or null. NTSC-U/C: 0x00435b98. */
extern rtMediusCallback g_pfnMediusRequestClanTeamChallengeCallback;

/** The receiver of message 109, or null. NTSC-U/C: 0x00435b9c. */
extern rtMediusCallback g_pfnMediusGetMyClanMessagesCallback;

/** The receiver of message 111, or null. NTSC-U/C: 0x00435ba0. */
extern rtMediusCallback g_pfnMediusSendClanMessageCallback;

/** The receiver of message 113, or null. NTSC-U/C: 0x00435ba4. */
extern rtMediusCallback g_pfnMediusModifyClanMessageCallback;

/** The receiver of message 115, or null. NTSC-U/C: 0x00435ba8. */
extern rtMediusCallback g_pfnMediusDeleteClanMessageCallback;

/** The receiver of message 117, or null. NTSC-U/C: 0x00435bac. */
extern rtMediusCallback g_pfnMediusRespondToClanTeamChallengeCallback;

/** The receiver of message 119, or null. NTSC-U/C: 0x00435bb0. */
extern rtMediusCallback g_pfnMediusRevokeClanTeamChallengeCallback;

/** The receiver of message 125, or null. NTSC-U/C: 0x00435bb4. */
extern rtMediusCallback g_pfnMediusGetMyClansCallback;

/** The receiver of message 131, or null. NTSC-U/C: 0x00435bb8. */
extern rtMediusCallback g_pfnMediusGetClanTeamChallengesCallback;

/** The receiver of message 129, or null. NTSC-U/C: 0x00435bbc. */
extern rtMediusCallback g_pfnMediusConfirmClanTeamChallengeCallback;

/** The receiver of message 127, or null. NTSC-U/C: 0x00435bc0. */
extern rtMediusCallback g_pfnMediusGetAllClanMessagesCallback;

/** The receiver of message 121, or null. NTSC-U/C: 0x00435bc4. */
extern rtMediusCallback g_pfnMediusGetClanTeamChallengeHistoryCallback;

/** The receiver of message 133, or null. NTSC-U/C: 0x00435bc8. */
extern rtMediusCallback g_pfnMediusUpdateClanStatsCallback;

/** The receiver of message 28, or null. NTSC-U/C: 0x003c2698. */
extern rtMediusCallback g_pfnMediusGetMyIPCallback;

/** The receiver of message 135, or null. NTSC-U/C: 0x003c2694. */
extern rtMediusVersionCallback g_pfnMediusVersionServerCallback;

/** The receiver of message 137, or null. NTSC-U/C: 0x003c269c. */
extern rtMediusCallback g_pfnMediusGetWorldSecurityLevelCallback;

/** The receiver of message 139, or null. NTSC-U/C: 0x003c26a0. */
extern rtMediusCallback g_pfnMediusBanPlayerCallback;

/** The receiver of message 20, or null. NTSC-U/C: 0x003c261c. */
extern rtMediusCallback g_pfnMediusAccountDeleteCallback;

/** The receiver of message 24, or null. NTSC-U/C: 0x003c25fc. */
extern rtMediusAccountGetIdCallback g_pfnMediusAccountGetIdCallback;

/** The receiver of message 141, or null. NTSC-U/C: 0x003c26a4. */
extern rtMediusCallback g_pfnMediusGetLocationsCallback;

/** The receiver of message 143, or null. NTSC-U/C: 0x003c26a8. */
extern rtMediusCallback g_pfnMediusPickLocationCallback;

/** The receiver of message 145, or null. NTSC-U/C: 0x00435bcc. */
extern rtMediusCallback g_pfnMediusGetClanMemberListCallback;

/** The receiver of message 147, or null. NTSC-U/C: 0x003c26ac. */
extern rtMediusLadderPositionCallback g_pfnMediusLadderPositionCallback;

/** The receiver of message 205, or null. NTSC-U/C: 0x003c26b0. */
extern rtMediusLadderPositionCallback g_pfnMediusLadderPositionFastCallback;

/** The receiver of message 149, or null. NTSC-U/C: 0x003c26b4. */
extern rtMediusLadderListCallback g_pfnMediusLadderListCallback;

/** The receiver of message 151, or null. NTSC-U/C: 0x003c26b8. */
extern rtMediusCallback g_pfnMediusChatToggleCallback;

/** The receiver of message 153, or null. NTSC-U/C: 0x003c26bc. */
extern rtMediusTextFilterCallback g_pfnMediusTextFilterCallback;

/** The receiver of message 154, or null. NTSC-U/C: 0x003c26c0. */
extern rtMediusCallback g_pfnMediusServerReassignGameMediusWorldIDCallback;

/** The receiver of message 156, or null. NTSC-U/C: 0x003c26c4. */
extern rtMediusCallback g_pfnMediusGetTotalGamesCallback;

/** The receiver of message 158, or null. NTSC-U/C: 0x003c26c8. */
extern rtMediusCallback g_pfnMediusGetTotalChannelsCallback;

/** The receiver of message 160, or null. NTSC-U/C: 0x003c26cc. */
extern rtMediusCallback g_pfnMediusGetLobbyPlayerNamesCallback;

/** The receiver of message 162, or null. NTSC-U/C: 0x003c26d0. */
extern rtMediusCallback g_pfnMediusGetTotalUsersCallback;

/** The receiver of message 164, or null. NTSC-U/C: 0x003c26d4. */
extern rtMediusCallback g_pfnMediusSetLocalizationParamsCallback;

/** The receiver of message 199, or null. NTSC-U/C: 0x003c26d8. */
extern rtMediusCallback g_pfnMediusSetMessageAsReadCallback;

/** The receiver of message 166, or null. NTSC-U/C: 0x004357f4. */
extern rtMediusFileCallback g_pfnMediusFileCreateCallback;

/** The receiver of message 171, or null. NTSC-U/C: 0x00435804. */
extern rtMediusFileCallback g_pfnMediusFileCloseCallback;

/** The receiver of message 173, or null. NTSC-U/C: 0x00435808. */
extern rtMediusFileTransferCallback g_pfnMediusFileDownloadCallback;

/** The receiver of message 175, or null. NTSC-U/C: 0x0043580c. */
extern rtMediusCallback g_pfnMediusFileDownloadStreamCallback;

/** The receiver of message 177, or null. NTSC-U/C: 0x00435810. */
extern rtMediusFileDeleteCallback g_pfnMediusFileDeleteCallback;

/** The receiver of message 179, or null. NTSC-U/C: 0x00435814. */
extern rtMediusFileCallback g_pfnMediusFileListFilesCallback;

/** The receiver of message 181, or null. NTSC-U/C: 0x00435818. */
extern rtMediusCallback g_pfnMediusFileUpdateAttributesCallback;

/** The receiver of message 183, or null. NTSC-U/C: 0x0043581c. */
extern rtMediusFileAttributesCallback g_pfnMediusFileGetAttributesCallback;

/** The receiver of message 185, or null. NTSC-U/C: 0x00435820. */
extern rtMediusCallback g_pfnMediusFileUpdateMetaDataCallback;

/** The receiver of message 187, or null. NTSC-U/C: 0x00435824. */
extern rtMediusCallback g_pfnMediusFileGetMetaDataCallback;

/** The receiver of message 189, or null. NTSC-U/C: 0x00435828. */
extern rtMediusCallback g_pfnMediusFileSearchByMetaDataCallback;

/** The receiver of message 191, or null. NTSC-U/C: 0x0043582c. */
extern rtMediusCallback g_pfnMediusFileCancelOperationCallback;

/** The receiver of message 200, or null. NTSC-U/C: 0x003c26dc. */
extern rtMediusCallback g_pfnMediusGetUniverseInformationCallback;

/** The receiver of message 201, or null. NTSC-U/C: 0x003c26e0. */
extern rtMediusCallback g_pfnMediusUniverseNewsCallback;

/** The receiver of message 202, or null. NTSC-U/C: 0x003c26e4. */
extern rtMediusCallback g_pfnMediusUniverseStatusListCallback;

/** The registered type of message 0. NTSC-U/C: 0x00435900. */
extern int g_nMediusWorldReportType;

/** The registered type of message 1. NTSC-U/C: 0x004358fc. */
extern int g_nMediusPlayerReportType;

/** The registered type of message 2. NTSC-U/C: 0x00435904. */
extern int g_nMediusEndGameReportType;

/** The registered type of message 3. NTSC-U/C: 0x00435874. */
extern int g_nMediusSessionBeginType;

/** The registered type of message 5. NTSC-U/C: 0x00435878. */
extern int g_nMediusSessionEndType;

/** The registered type of message 7. NTSC-U/C: 0x0043587c. */
extern int g_nMediusAccountLoginType;

/** The registered type of message 9. NTSC-U/C: 0x00435888. */
extern int g_nMediusAccountRegistrationType;

/** The registered type of message 15. NTSC-U/C: 0x0043588c. */
extern int g_nMediusChangePasswordType;

/** The registered type of message 17. NTSC-U/C: 0x00435890. */
extern int g_nMediusUpdateStatsType;

/** The registered type of message 19. NTSC-U/C: 0x00435898. */
extern int g_nMediusAccountDeleteType;

/** The registered type of message 23. NTSC-U/C: 0x00435928. */
extern int g_nMediusAccountGetIdType;

/** The registered type of message 25. NTSC-U/C: 0x0043589c. */
extern int g_nMediusAnonymousLoginType;

/** The registered type of message 31. NTSC-U/C: 0x00435868. */
extern int g_nMediusMessage031Type;

/** The registered type of message 33. NTSC-U/C: 0x004358a8. */
extern int g_nMediusCreateChannelType;

/** The registered type of message 35. NTSC-U/C: 0x00435864. */
extern int g_nMediusJoinGameType;

/** The registered type of message 37. NTSC-U/C: 0x004358ac. */
extern int g_nMediusJoinChannelType;

/** The registered type of message 41. NTSC-U/C: 0x004358b0. */
extern int g_nMediusGameListType;

/** The registered type of message 43. NTSC-U/C: 0x004358b4. */
extern int g_nMediusChannelListType;

/** The registered type of message 45. NTSC-U/C: 0x004358b8. */
extern int g_nMediusChannelPlayersType;

/** The registered type of message 47. NTSC-U/C: 0x004358bc. */
extern int g_nMediusGamePlayersType;

/** The registered type of message 49. NTSC-U/C: 0x004358c0. */
extern int g_nMediusPlayerInfoType;

/** The registered type of message 53. NTSC-U/C: 0x004358c8. */
extern int g_nMediusChannelInfoType;

/** The registered type of message 57. NTSC-U/C: 0x004358cc. */
extern int g_nMediusFindPlayerType;

/** The registered type of message 59. NTSC-U/C: 0x004358d4. */
extern int g_nMediusChatMessageType;

/** The registered type of message 71. NTSC-U/C: 0x004358f8. */
extern int g_nMediusGetPolicyType;

/** The registered type of message 73. NTSC-U/C: 0x00435870. */
extern int g_nMediusUpdateUserStateType;

/** The registered type of message 75. NTSC-U/C: 0x00435908. */
extern int g_nMediusGetAnnouncementsType;

/** The registered type of message 78. NTSC-U/C: 0x00435910. */
extern int g_nMediusSetGameListFilterType;

/** The registered type of message 80. NTSC-U/C: 0x00435914. */
extern int g_nMediusClearGameListFilterType;

/** The registered type of message 134. NTSC-U/C: 0x0043586c. */
extern int g_nMediusVersionServerType;

/** The registered type of message 138. NTSC-U/C: 0x00435924. */
extern int g_nMediusBanPlayerType;

/** The registered type of message 146. NTSC-U/C: 0x00435934. */
extern int g_nMediusLadderPositionType;

/** The registered type of message 148. NTSC-U/C: 0x0043593c. */
extern int g_nMediusLadderListType;

/** The registered type of message 152. NTSC-U/C: 0x004358a4. */
extern int g_nMediusTextFilterType;

/** The registered type of message 165. NTSC-U/C: 0x00435830. */
extern int g_nMediusFileCreateType;

/** The registered type of message 167. NTSC-U/C: 0x00435834. */
extern int g_nMediusFileUploadType;

/** The registered type of message 170. NTSC-U/C: 0x00435838. */
extern int g_nMediusFileCloseType;

/** The registered type of message 172. NTSC-U/C: 0x0043583c. */
extern int g_nMediusFileDownloadType;

/** The registered type of message 176. NTSC-U/C: 0x00435844. */
extern int g_nMediusFileDeleteType;

/** The registered type of message 178. NTSC-U/C: 0x00435848. */
extern int g_nMediusFileListType;

/** The registered type of message 182. NTSC-U/C: 0x00435850. */
extern int g_nMediusFileGetAttributesType;

/** The registered type of message 204. NTSC-U/C: 0x00435938. */
extern int g_nMediusLadderPositionFastType;

/**
 * Register the message types of the lobby server with their receivers.
 *
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002fbef8
 * @ghidraAddress PAL: 0x00305f90
 */
int rtMediusRegisterMessages(void);

/**
 * Store the address of the lobby server a login reports.
 *
 * @param address The address.
 * @ghidraAddress NTSC-U/C: 0x002f98b8
 * @ghidraAddress PAL: 0x003038b8
 */
void rtMediusSetServerAddress(const rtNetAddress *address);

/**
 * Receive message 74 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301938
 * @ghidraAddress PAL: 0x0030b9d0
 */
int rtMediusOnErrorMessage(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 0 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301968
 * @ghidraAddress PAL: 0x0030ba00
 */
int rtMediusOnWorldReport0(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 1 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301970
 * @ghidraAddress PAL: 0x0030ba08
 */
int rtMediusOnPlayerReport(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 2 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301978
 * @ghidraAddress PAL: 0x0030ba10
 */
int rtMediusOnEndGameReport(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 3 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301980
 * @ghidraAddress PAL: 0x0030ba18
 */
int rtMediusOnSessionBegin(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 4 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301988
 * @ghidraAddress PAL: 0x0030ba20
 */
int rtMediusOnSessionBeginResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 5 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301a10
 * @ghidraAddress PAL: 0x0030baa8
 */
int rtMediusOnSessionEnd(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 6 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301a18
 * @ghidraAddress PAL: 0x0030bab0
 */
int rtMediusOnSessionEndResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 203 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301a60
 * @ghidraAddress PAL: 0x0030baf8
 */
int rtMediusOnMachineSignaturePost(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 7 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301a68
 * @ghidraAddress PAL: 0x0030bb00
 */
int rtMediusOnAccountLogin(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 8 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301a70
 * @ghidraAddress PAL: 0x0030bb08
 */
int rtMediusOnAccountLoginResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 21 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301af0
 * @ghidraAddress PAL: 0x0030bb88
 */
int rtMediusOnAccountLogout(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 22 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301af8
 * @ghidraAddress PAL: 0x0030bb90
 */
int rtMediusOnAccountLogoutResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 9 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301b28
 * @ghidraAddress PAL: 0x0030bbc0
 */
int rtMediusOnAccountRegistration(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 10 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301b30
 * @ghidraAddress PAL: 0x0030bbc8
 */
int rtMediusOnAccountRegistrationResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message);

/**
 * Receive message 11 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301b60
 * @ghidraAddress PAL: 0x0030bbf8
 */
int rtMediusOnAccountGetProfile(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 12 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301b68
 * @ghidraAddress PAL: 0x0030bc00
 */
int rtMediusOnAccountGetProfileResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message);

/**
 * Receive message 13 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301b98
 * @ghidraAddress PAL: 0x0030bc30
 */
int rtMediusOnAccountUpdateProfile(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 14 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301ba0
 * @ghidraAddress PAL: 0x0030bc38
 */
int rtMediusOnAccountUpdateProfileResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message);

/**
 * Receive message 15 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301bd0
 * @ghidraAddress PAL: 0x0030bc68
 */
int rtMediusOnAccountUpdatePassword(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 16 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301bd8
 * @ghidraAddress PAL: 0x0030bc70
 */
int rtMediusOnChangePasswordResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 17 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301c08
 * @ghidraAddress PAL: 0x0030bca0
 */
int rtMediusOnAccountUpdateStats(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 18 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301c10
 * @ghidraAddress PAL: 0x0030bca8
 */
int rtMediusOnAccountUpdateStatsResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message);

/**
 * Receive message 25 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301c40
 * @ghidraAddress PAL: 0x0030bcd8
 */
int rtMediusOnAnonymousLogin(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 26 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301c48
 * @ghidraAddress PAL: 0x0030bce0
 */
int rtMediusOnAnonymousLoginResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 29 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301c78
 * @ghidraAddress PAL: 0x0030bd10
 */
int rtMediusOnCreateGameRequest0(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 30 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301c80
 * @ghidraAddress PAL: 0x0030bd18
 */
int rtMediusOnCreateGameResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 31 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301cb0
 * @ghidraAddress PAL: 0x0030bd48
 */
int rtMediusOnCreateGameOnSelf(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 32 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301cb8
 * @ghidraAddress PAL: 0x0030bd50
 */
int rtMediusOnCreateGameOnSelfResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 33 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301ce8
 * @ghidraAddress PAL: 0x0030bd80
 */
int rtMediusOnCreateChannelRequest0(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 34 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301cf0
 * @ghidraAddress PAL: 0x0030bd88
 */
int rtMediusOnCreateChannelResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 35 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301d20
 * @ghidraAddress PAL: 0x0030bdb8
 */
int rtMediusOnJoinGameRequest0(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 36 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301d28
 * @ghidraAddress PAL: 0x0030bdc0
 */
int rtMediusOnJoinGameResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 37 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301d58
 * @ghidraAddress PAL: 0x0030bdf0
 */
int rtMediusOnJoinChannel(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 38 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301d60
 * @ghidraAddress PAL: 0x0030bdf8
 */
int rtMediusOnJoinChannelResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 39 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301d90
 * @ghidraAddress PAL: 0x0030be28
 */
int rtMediusOnJoinChannelFwd(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 40 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301d98
 * @ghidraAddress PAL: 0x0030be30
 */
int rtMediusOnJoinChannelFwdResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 41 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301da0
 * @ghidraAddress PAL: 0x0030be38
 */
int rtMediusOnGameList(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 42 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301da8
 * @ghidraAddress PAL: 0x0030be40
 */
int rtMediusOnGameListResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 43 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301dd8
 * @ghidraAddress PAL: 0x0030be70
 */
int rtMediusOnChannelList(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 44 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301de0
 * @ghidraAddress PAL: 0x0030be78
 */
int rtMediusOnChannelListResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 45 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301e10
 * @ghidraAddress PAL: 0x0030bea8
 */
int rtMediusOnLobbyWorldPlayerList(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 46 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301e18
 * @ghidraAddress PAL: 0x0030beb0
 */
int rtMediusOnLobbyWorldPlayerListResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message);

/**
 * Receive message 47 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301e48
 * @ghidraAddress PAL: 0x0030bee0
 */
int rtMediusOnGameWorldPlayerList(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 48 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301e50
 * @ghidraAddress PAL: 0x0030bee8
 */
int rtMediusOnGameWorldPlayerListResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message);

/**
 * Receive message 49 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301e80
 * @ghidraAddress PAL: 0x0030bf18
 */
int rtMediusOnPlayerInfo(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 50 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301e88
 * @ghidraAddress PAL: 0x0030bf20
 */
int rtMediusOnPlayerInfoResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 51 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301eb8
 * @ghidraAddress PAL: 0x0030bf50
 */
int rtMediusOnGameInfo0(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 52 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301ec0
 * @ghidraAddress PAL: 0x0030bf58
 */
int rtMediusOnGameInfoResponse0(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 53 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301ef0
 * @ghidraAddress PAL: 0x0030bf88
 */
int rtMediusOnChannelInfo(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 54 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301ef8
 * @ghidraAddress PAL: 0x0030bf90
 */
int rtMediusOnChannelInfoResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 55 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301f28
 * @ghidraAddress PAL: 0x0030bfc0
 */
int rtMediusOnFindWorldByName(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 56 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301f30
 * @ghidraAddress PAL: 0x0030bfc8
 */
int rtMediusOnFindWorldByNameResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 57 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301f60
 * @ghidraAddress PAL: 0x0030bff8
 */
int rtMediusOnFindPlayer(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 58 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301f68
 * @ghidraAddress PAL: 0x0030c000
 */
int rtMediusOnFindPlayerResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 59 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301f98
 * @ghidraAddress PAL: 0x0030c030
 */
int rtMediusOnChatMessage(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 60 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301fa0
 * @ghidraAddress PAL: 0x0030c038
 */
int rtMediusOnChatFwdMessage(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 61 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301fd0
 * @ghidraAddress PAL: 0x0030c068
 */
int rtMediusOnGetBuddyList(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 62 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00301fd8
 * @ghidraAddress PAL: 0x0030c070
 */
int rtMediusOnGetBuddyListResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 63 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302008
 * @ghidraAddress PAL: 0x0030c0a0
 */
int rtMediusOnAddToBuddyList(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 64 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302010
 * @ghidraAddress PAL: 0x0030c0a8
 */
int rtMediusOnAddToBuddyListResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 65 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302040
 * @ghidraAddress PAL: 0x0030c0d8
 */
int rtMediusOnRemoveFromBuddyList(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 66 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302048
 * @ghidraAddress PAL: 0x0030c0e0
 */
int rtMediusOnRemoveFromBuddyListResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message);

/**
 * Receive message 67 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302078
 * @ghidraAddress PAL: 0x0030c110
 */
int rtMediusOnAddToBuddyListConfirmationRequest0(int connection,
                                                 int type,
                                                 int sender,
                                                 rtDmeMessage *message);

/**
 * Receive message 68 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302080
 * @ghidraAddress PAL: 0x0030c118
 */
int rtMediusOnAddToBuddyListConfirmationResponse(int connection,
                                                 int type,
                                                 int sender,
                                                 rtDmeMessage *message);

/**
 * Receive message 69 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003020b0
 * @ghidraAddress PAL: 0x0030c148
 */
int rtMediusOnAddToBuddyListFwdConfirmationRequest0(int connection,
                                                    int type,
                                                    int sender,
                                                    rtDmeMessage *message);

/**
 * Receive message 70 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003020e0
 * @ghidraAddress PAL: 0x0030c178
 */
int rtMediusOnAddToBuddyListFwdConfirmationResponse0(int connection,
                                                     int type,
                                                     int sender,
                                                     rtDmeMessage *message);

/**
 * Receive message 192 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003020e8
 * @ghidraAddress PAL: 0x0030c180
 */
int rtMediusOnGetIgnoreList(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 193 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003020f0
 * @ghidraAddress PAL: 0x0030c188
 */
int rtMediusOnGetIgnoreListResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 194 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302120
 * @ghidraAddress PAL: 0x0030c1b8
 */
int rtMediusOnAddToIgnoreList(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 195 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302128
 * @ghidraAddress PAL: 0x0030c1c0
 */
int rtMediusOnAddToIgnoreListResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 196 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302158
 * @ghidraAddress PAL: 0x0030c1f0
 */
int rtMediusOnRemoveFromIgnoreList(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 197 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302160
 * @ghidraAddress PAL: 0x0030c1f8
 */
int rtMediusOnRemoveFromIgnoreListResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message);

/**
 * Receive message 71 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302190
 * @ghidraAddress PAL: 0x0030c228
 */
int rtMediusOnPolicy(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 72 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302198
 * @ghidraAddress PAL: 0x0030c230
 */
int rtMediusOnPolicyResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 73 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003021c8
 * @ghidraAddress PAL: 0x0030c260
 */
int rtMediusOnUpdateUserState(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 75 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003021d0
 * @ghidraAddress PAL: 0x0030c268
 */
int rtMediusOnGetAnnouncements(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 76 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003021d8
 * @ghidraAddress PAL: 0x0030c270
 */
int rtMediusOnGetAllAnnouncements(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 77 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003021e0
 * @ghidraAddress PAL: 0x0030c278
 */
int rtMediusOnGetAnnouncementsResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 78 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302210
 * @ghidraAddress PAL: 0x0030c2a8
 */
int rtMediusOnSetGameListFilter0(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 79 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302218
 * @ghidraAddress PAL: 0x0030c2b0
 */
int rtMediusOnSetGameListFilterResponse0(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message);

/**
 * Receive message 80 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302248
 * @ghidraAddress PAL: 0x0030c2e0
 */
int rtMediusOnClearGameListFilter0(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 81 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302250
 * @ghidraAddress PAL: 0x0030c2e8
 */
int rtMediusOnClearGameListFilterResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message);

/**
 * Receive message 82 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302280
 * @ghidraAddress PAL: 0x0030c318
 */
int rtMediusOnGetGameListFilter(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 83 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302288
 * @ghidraAddress PAL: 0x0030c320
 */
int rtMediusOnGetGameListFilterResponse0(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message);

/**
 * Receive message 84 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003022b8
 * @ghidraAddress PAL: 0x0030c350
 */
int rtMediusOnCreateClan(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 85 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003022c0
 * @ghidraAddress PAL: 0x0030c358
 */
int rtMediusOnCreateClanResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 86 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003022f0
 * @ghidraAddress PAL: 0x0030c388
 */
int rtMediusOnDisbandClan(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 87 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003022f8
 * @ghidraAddress PAL: 0x0030c390
 */
int rtMediusOnDisbandClanResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 88 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302328
 * @ghidraAddress PAL: 0x0030c3c0
 */
int rtMediusOnGetClanByID(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 89 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302330
 * @ghidraAddress PAL: 0x0030c3c8
 */
int rtMediusOnGetClanByIDResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 90 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302360
 * @ghidraAddress PAL: 0x0030c3f8
 */
int rtMediusOnGetClanByName(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 91 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302368
 * @ghidraAddress PAL: 0x0030c400
 */
int rtMediusOnGetClanByNameResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 92 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302398
 * @ghidraAddress PAL: 0x0030c430
 */
int rtMediusOnTransferClanLeadership(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 93 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003023a0
 * @ghidraAddress PAL: 0x0030c438
 */
int rtMediusOnTransferClanLeadershipResponse(int connection,
                                             int type,
                                             int sender,
                                             rtDmeMessage *message);

/**
 * Receive message 94 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003023d0
 * @ghidraAddress PAL: 0x0030c468
 */
int rtMediusOnAddPlayerToClan(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 95 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003023d8
 * @ghidraAddress PAL: 0x0030c470
 */
int rtMediusOnAddPlayerToClanResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 96 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302408
 * @ghidraAddress PAL: 0x0030c4a0
 */
int rtMediusOnRemovePlayerFromClan(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 97 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302410
 * @ghidraAddress PAL: 0x0030c4a8
 */
int rtMediusOnRemovePlayerFromClanResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message);

/**
 * Receive message 98 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302440
 * @ghidraAddress PAL: 0x0030c4d8
 */
int rtMediusOnInvitePlayerToClan(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 99 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302448
 * @ghidraAddress PAL: 0x0030c4e0
 */
int rtMediusOnInvitePlayerToClanResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message);

/**
 * Receive message 100 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302478
 * @ghidraAddress PAL: 0x0030c510
 */
int rtMediusOnCheckMyClanInvitations(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 101 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302480
 * @ghidraAddress PAL: 0x0030c518
 */
int rtMediusOnCheckMyClanInvitationsResponse(int connection,
                                             int type,
                                             int sender,
                                             rtDmeMessage *message);

/**
 * Receive message 122 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003024b0
 * @ghidraAddress PAL: 0x0030c548
 */
int rtMediusOnGetClanInvitationsSent(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 123 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003024b8
 * @ghidraAddress PAL: 0x0030c550
 */
int rtMediusOnGetClanInvitationsSentResponse(int connection,
                                             int type,
                                             int sender,
                                             rtDmeMessage *message);

/**
 * Receive message 102 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003024e8
 * @ghidraAddress PAL: 0x0030c580
 */
int rtMediusOnRespondToClanInvitation(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 103 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003024f0
 * @ghidraAddress PAL: 0x0030c588
 */
int rtMediusOnRespondToClanInvitationResponse(int connection,
                                              int type,
                                              int sender,
                                              rtDmeMessage *message);

/**
 * Receive message 104 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302520
 * @ghidraAddress PAL: 0x0030c5b8
 */
int rtMediusOnRevokeClanInvitation(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 105 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302528
 * @ghidraAddress PAL: 0x0030c5c0
 */
int rtMediusOnRevokeClanInvitationResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message);

/**
 * Receive message 106 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302558
 * @ghidraAddress PAL: 0x0030c5f0
 */
int rtMediusOnRequestClanTeamChallenge(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 107 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302560
 * @ghidraAddress PAL: 0x0030c5f8
 */
int rtMediusOnRequestClanTeamChallengeResponse(int connection,
                                               int type,
                                               int sender,
                                               rtDmeMessage *message);

/**
 * Receive message 108 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302590
 * @ghidraAddress PAL: 0x0030c628
 */
int rtMediusOnGetMyClanMessages(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 109 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302598
 * @ghidraAddress PAL: 0x0030c630
 */
int rtMediusOnGetMyClanMessagesResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message);

/**
 * Receive message 110 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003025c8
 * @ghidraAddress PAL: 0x0030c660
 */
int rtMediusOnSendClanMessage(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 111 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003025d0
 * @ghidraAddress PAL: 0x0030c668
 */
int rtMediusOnSendClanMessageResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 112 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302600
 * @ghidraAddress PAL: 0x0030c698
 */
int rtMediusOnModifyClanMessage(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 113 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302608
 * @ghidraAddress PAL: 0x0030c6a0
 */
int rtMediusOnModifyClanMessageResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message);

/**
 * Receive message 114 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302638
 * @ghidraAddress PAL: 0x0030c6d0
 */
int rtMediusOnDeleteClanMessage(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 115 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302640
 * @ghidraAddress PAL: 0x0030c6d8
 */
int rtMediusOnDeleteClanMessageResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message);

/**
 * Receive message 116 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302670
 * @ghidraAddress PAL: 0x0030c708
 */
int rtMediusOnRespondToClanTeamChallenge(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message);

/**
 * Receive message 117 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302678
 * @ghidraAddress PAL: 0x0030c710
 */
int rtMediusOnRespondToClanTeamChallengeResponse(int connection,
                                                 int type,
                                                 int sender,
                                                 rtDmeMessage *message);

/**
 * Receive message 118 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003026a8
 * @ghidraAddress PAL: 0x0030c740
 */
int rtMediusOnRevokeClanTeamChallenge(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 119 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003026b0
 * @ghidraAddress PAL: 0x0030c748
 */
int rtMediusOnRevokeClanTeamChallengeResponse(int connection,
                                              int type,
                                              int sender,
                                              rtDmeMessage *message);

/**
 * Receive message 124 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003026e0
 * @ghidraAddress PAL: 0x0030c778
 */
int rtMediusOnGetMyClans(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 125 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003026e8
 * @ghidraAddress PAL: 0x0030c780
 */
int rtMediusOnGetMyClansResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 130 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302718
 * @ghidraAddress PAL: 0x0030c7b0
 */
int rtMediusOnGetClanTeamChallenges(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 131 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302720
 * @ghidraAddress PAL: 0x0030c7b8
 */
int rtMediusOnGetClanTeamChallengesResponse(int connection,
                                            int type,
                                            int sender,
                                            rtDmeMessage *message);

/**
 * Receive message 128 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302750
 * @ghidraAddress PAL: 0x0030c7e8
 */
int rtMediusOnConfirmClanTeamChallenge(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 129 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302758
 * @ghidraAddress PAL: 0x0030c7f0
 */
int rtMediusOnConfirmClanTeamChallengeResponse(int connection,
                                               int type,
                                               int sender,
                                               rtDmeMessage *message);

/**
 * Receive message 126 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302788
 * @ghidraAddress PAL: 0x0030c820
 */
int rtMediusOnGetAllClanMessages(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 127 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302790
 * @ghidraAddress PAL: 0x0030c828
 */
int rtMediusOnGetAllClanMessagesResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message);

/**
 * Receive message 120 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003027c0
 * @ghidraAddress PAL: 0x0030c858
 */
int rtMediusOnGetClanTeamChallengeHistory(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message);

/**
 * Receive message 121 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003027c8
 * @ghidraAddress PAL: 0x0030c860
 */
int rtMediusOnGetClanTeamChallengeHistoryResponse(int connection,
                                                  int type,
                                                  int sender,
                                                  rtDmeMessage *message);

/**
 * Receive message 132 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003027f8
 * @ghidraAddress PAL: 0x0030c890
 */
int rtMediusOnUpdateClanStats(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 133 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302800
 * @ghidraAddress PAL: 0x0030c898
 */
int rtMediusOnUpdateClanStatsResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 27 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302830
 * @ghidraAddress PAL: 0x0030c8c8
 */
int rtMediusOnGetMyIP(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 28 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302838
 * @ghidraAddress PAL: 0x0030c8d0
 */
int rtMediusOnGetMyIPResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 134 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302868
 * @ghidraAddress PAL: 0x0030c900
 */
int rtMediusOnVersionServer(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 135 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302870
 * @ghidraAddress PAL: 0x0030c908
 */
int rtMediusOnVersionServerResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 136 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003028a0
 * @ghidraAddress PAL: 0x0030c938
 */
int rtMediusOnGetWorldSecurityLevel(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 137 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003028a8
 * @ghidraAddress PAL: 0x0030c940
 */
int rtMediusOnGetWorldSecurityLevelResponse(int connection,
                                            int type,
                                            int sender,
                                            rtDmeMessage *message);

/**
 * Receive message 138 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003028d8
 * @ghidraAddress PAL: 0x0030c970
 */
int rtMediusOnBanPlayer(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 139 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003028e0
 * @ghidraAddress PAL: 0x0030c978
 */
int rtMediusOnBanPlayerResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 19 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302910
 * @ghidraAddress PAL: 0x0030c9a8
 */
int rtMediusOnAccountDelete(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 20 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302918
 * @ghidraAddress PAL: 0x0030c9b0
 */
int rtMediusOnAccountDeleteResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 23 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302948
 * @ghidraAddress PAL: 0x0030c9e0
 */
int rtMediusOnAccountGetId(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 24 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302950
 * @ghidraAddress PAL: 0x0030c9e8
 */
int rtMediusOnAccountGetIdResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 140 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302980
 * @ghidraAddress PAL: 0x0030ca18
 */
int rtMediusOnGetLocations(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 141 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302988
 * @ghidraAddress PAL: 0x0030ca20
 */
int rtMediusOnGetLocationsResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 142 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003029b8
 * @ghidraAddress PAL: 0x0030ca50
 */
int rtMediusOnPickLocation(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 143 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003029c0
 * @ghidraAddress PAL: 0x0030ca58
 */
int rtMediusOnPickLocationResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 144 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003029f0
 * @ghidraAddress PAL: 0x0030ca88
 */
int rtMediusOnGetClanMemberList(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 145 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003029f8
 * @ghidraAddress PAL: 0x0030ca90
 */
int rtMediusOnGetClanMemberListResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message);

/**
 * Receive message 146 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302a28
 * @ghidraAddress PAL: 0x0030cac0
 */
int rtMediusOnLadderPosition(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 147 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302a30
 * @ghidraAddress PAL: 0x0030cac8
 */
int rtMediusOnLadderPositionResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 204 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302a60
 * @ghidraAddress PAL: 0x0030caf8
 */
int rtMediusOnLadderPositionFast(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 205 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302a68
 * @ghidraAddress PAL: 0x0030cb00
 */
int rtMediusOnLadderPositionFastResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message);

/**
 * Receive message 148 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302a98
 * @ghidraAddress PAL: 0x0030cb30
 */
int rtMediusOnLadderList(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 149 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302aa0
 * @ghidraAddress PAL: 0x0030cb38
 */
int rtMediusOnLadderListResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 150 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302ad0
 * @ghidraAddress PAL: 0x0030cb68
 */
int rtMediusOnChatToggle(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 151 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302ad8
 * @ghidraAddress PAL: 0x0030cb70
 */
int rtMediusOnChatToggleResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 152 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302b08
 * @ghidraAddress PAL: 0x0030cba0
 */
int rtMediusOnTextFilter(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 153 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302b10
 * @ghidraAddress PAL: 0x0030cba8
 */
int rtMediusOnTextFilterResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 154 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302b40
 * @ghidraAddress PAL: 0x0030cbd8
 */
int rtMediusOnServerReassignGameMediusWorldID(int connection,
                                              int type,
                                              int sender,
                                              rtDmeMessage *message);

/**
 * Receive message 155 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302b70
 * @ghidraAddress PAL: 0x0030cc08
 */
int rtMediusOnGetTotalGames(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 156 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302b78
 * @ghidraAddress PAL: 0x0030cc10
 */
int rtMediusOnGetTotalGamesResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 157 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302ba8
 * @ghidraAddress PAL: 0x0030cc40
 */
int rtMediusOnGetTotalChannels(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 158 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302bb0
 * @ghidraAddress PAL: 0x0030cc48
 */
int rtMediusOnGetTotalChannelsResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 159 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302be0
 * @ghidraAddress PAL: 0x0030cc78
 */
int rtMediusOnGetLobbyPlayerNames(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 160 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302be8
 * @ghidraAddress PAL: 0x0030cc80
 */
int rtMediusOnGetLobbyPlayerNamesResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message);

/**
 * Receive message 161 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302c18
 * @ghidraAddress PAL: 0x0030ccb0
 */
int rtMediusOnGetTotalUsers(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 162 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302c20
 * @ghidraAddress PAL: 0x0030ccb8
 */
int rtMediusOnGetTotalUsersResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 163 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302c50
 * @ghidraAddress PAL: 0x0030cce8
 */
int rtMediusOnSetLocalizationParams(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 164 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302c58
 * @ghidraAddress PAL: 0x0030ccf0
 */
int rtMediusOnSetLocalizationParamsResponse(int connection,
                                            int type,
                                            int sender,
                                            rtDmeMessage *message);

/**
 * Receive message 198 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302c88
 * @ghidraAddress PAL: 0x0030cd20
 */
int rtMediusOnSetMessageAsRead(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 199 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302c90
 * @ghidraAddress PAL: 0x0030cd28
 */
int rtMediusOnSetMessageAsReadResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 165 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302cc0
 * @ghidraAddress PAL: 0x0030cd58
 */
int rtMediusOnFileCreate(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 166 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302cc8
 * @ghidraAddress PAL: 0x0030cd60
 */
int rtMediusOnFileCreateResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 167 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302cf8
 * @ghidraAddress PAL: 0x0030cd90
 */
int rtMediusOnFileUpload(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 168 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302d00
 * @ghidraAddress PAL: 0x0030cd98
 */
int rtMediusOnFileUploadResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 170 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302e58
 * @ghidraAddress PAL: 0x0030cef0
 */
int rtMediusOnFileClose(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 171 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302e60
 * @ghidraAddress PAL: 0x0030cef8
 */
int rtMediusOnFileCloseResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 172 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302e90
 * @ghidraAddress PAL: 0x0030cf28
 */
int rtMediusOnFileDownload(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 173 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302e98
 * @ghidraAddress PAL: 0x0030cf30
 */
int rtMediusOnFileDownloadResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 174 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302ec8
 * @ghidraAddress PAL: 0x0030cf60
 */
int rtMediusOnFileDownloadStream(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 175 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302ed0
 * @ghidraAddress PAL: 0x0030cf68
 */
int rtMediusOnFileDownloadStreamResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message);

/**
 * Receive message 176 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302f00
 * @ghidraAddress PAL: 0x0030cf98
 */
int rtMediusOnFileDelete(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 177 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302f08
 * @ghidraAddress PAL: 0x0030cfa0
 */
int rtMediusOnFileDeleteResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 178 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302f38
 * @ghidraAddress PAL: 0x0030cfd0
 */
int rtMediusOnFileListFiles(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 179 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302f40
 * @ghidraAddress PAL: 0x0030cfd8
 */
int rtMediusOnFileListFilesResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 180 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302f70
 * @ghidraAddress PAL: 0x0030d008
 */
int rtMediusOnFileUpdateAttributes(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 181 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302f78
 * @ghidraAddress PAL: 0x0030d010
 */
int rtMediusOnFileUpdateAttributesResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message);

/**
 * Receive message 182 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302fa8
 * @ghidraAddress PAL: 0x0030d040
 */
int rtMediusOnFileGetAttributes(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 183 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302fb0
 * @ghidraAddress PAL: 0x0030d048
 */
int rtMediusOnFileGetAttributesResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message);

/**
 * Receive message 184 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302fe0
 * @ghidraAddress PAL: 0x0030d078
 */
int rtMediusOnFileUpdateMetaData(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 185 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00302fe8
 * @ghidraAddress PAL: 0x0030d080
 */
int rtMediusOnFileUpdateMetaDataResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message);

/**
 * Receive message 186 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303018
 * @ghidraAddress PAL: 0x0030d0b0
 */
int rtMediusOnFileGetMetaData(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 187 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303020
 * @ghidraAddress PAL: 0x0030d0b8
 */
int rtMediusOnFileGetMetaDataResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 188 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303050
 * @ghidraAddress PAL: 0x0030d0e8
 */
int rtMediusOnFileSearchByMetaData(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 189 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303058
 * @ghidraAddress PAL: 0x0030d0f0
 */
int rtMediusOnFileSearchByMetaDataResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message);

/**
 * Receive message 190 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303088
 * @ghidraAddress PAL: 0x0030d120
 */
int rtMediusOnFileCancelOperation(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 191 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303090
 * @ghidraAddress PAL: 0x0030d128
 */
int rtMediusOnFileCancelOperationResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message);

/**
 * Receive message 200 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003030c0
 * @ghidraAddress PAL: 0x0030d158
 */
int rtMediusOnGetUniverseInformation(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 201 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x003030f0
 * @ghidraAddress PAL: 0x0030d188
 */
int rtMediusOnUniverseNewsResponse(int connection, int type, int sender, rtDmeMessage *message);

/**
 * Receive message 202 of the lobby server and report the size of its record.
 *
 * @ghidraAddress NTSC-U/C: 0x00303120
 * @ghidraAddress PAL: 0x0030d1b8
 */
int rtMediusOnUniverseStatusListResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message);

/**
 * Send the lobby server the next part of the file of rtMediusFileUpload() and report the size of
 * the request.
 *
 * @ghidraAddress NTSC-U/C: 0x00302d08
 * @ghidraAddress PAL: 0x0030cda0
 */
int rtMediusOnFileUploadServerReq(int connection, int type, int sender, rtDmeMessage *message);

#ifdef __cplusplus
}
#endif

#endif
