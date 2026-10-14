#include <string.h>

#include "rt/rtmedius.h"

// The size of the record of each message, in the registration order.
enum {
    kMessage000Size = 0x180,
    kMessage001Size = 0x118,
    kMessage002Size = 0x7c,
    kMessage003Size = 0x1c,
    kMessage004Size = 0x30,
    kMessage005Size = 0x26,
    kMessage006Size = 0x1c,
    kMessage007Size = 0x66,
    kMessage008Size = 0xc4,
    kMessage009Size = 0x6c,
    kMessage010Size = 0x20,
    kMessage011Size = 0x26,
    kMessage012Size = 0x1cc,
    kMessage013Size = 0x1b0,
    kMessage014Size = 0x1c,
    kMessage015Size = 0x66,
    kMessage016Size = 0x1c,
    kMessage017Size = 0x126,
    kMessage018Size = 0x1c,
    kMessage019Size = 0x46,
    kMessage020Size = 0x1c,
    kMessage021Size = 0x26,
    kMessage022Size = 0x1c,
    kMessage023Size = 0x46,
    kMessage024Size = 0x20,
    kMessage025Size = 0x146,
    kMessage026Size = 0xc4,
    kMessage027Size = 0x26,
    kMessage028Size = 0x30,
    kMessage029Size = 0xb0,
    kMessage030Size = 0x20,
    kMessage031Size = 0xec,
    kMessage032Size = 0x20,
    kMessage033Size = 0x90,
    kMessage034Size = 0x20,
    kMessage035Size = 0x80,
    kMessage036Size = 0xbc,
    kMessage037Size = 0x4c,
    kMessage038Size = 0xb8,
    kMessage039Size = 0x26,
    kMessage040Size = 0x3c,
    kMessage041Size = 0x30,
    kMessage042Size = 0x70,
    kMessage043Size = 0x2a,
    kMessage044Size = 0x68,
    kMessage045Size = 0x2c,
    kMessage046Size = 0x14c,
    kMessage047Size = 0x2c,
    kMessage048Size = 0x148,
    kMessage049Size = 0x2c,
    kMessage050Size = 0x148,
    kMessage051Size = 0x2c,
    kMessage052Size = 0x18c,
    kMessage053Size = 0x2c,
    kMessage054Size = 0x64,
    kMessage055Size = 0x6c,
    kMessage056Size = 0x90,
    kMessage057Size = 0x50,
    kMessage058Size = 0x70,
    kMessage059Size = 0x70,
    kMessage060Size = 0x80,
    kMessage061Size = 0x26,
    kMessage062Size = 0x48,
    kMessage063Size = 0x2c,
    kMessage064Size = 0x1c,
    kMessage065Size = 0x2c,
    kMessage066Size = 0x1c,
    kMessage067Size = 0x2c,
    kMessage068Size = 0x40,
    kMessage069Size = 0x3c,
    kMessage070Size = 0x34,
    kMessage071Size = 0x2c,
    kMessage072Size = 0x120,
    kMessage073Size = 0x18,
    kMessage074Size = 0x104,
    kMessage075Size = 0x2c,
    kMessage076Size = 0x2c,
    kMessage077Size = 0x40c,
    kMessage078Size = 0x34,
    kMessage079Size = 0x1c,
    kMessage080Size = 0x2c,
    kMessage081Size = 0x1c,
    kMessage082Size = 0x26,
    kMessage083Size = 0x2c,
    kMessage084Size = 0x4c,
    kMessage085Size = 0x20,
    kMessage086Size = 0x2c,
    kMessage087Size = 0x1c,
    kMessage088Size = 0x30,
    kMessage089Size = 0x168,
    kMessage090Size = 0x4c,
    kMessage091Size = 0x148,
    kMessage092Size = 0x4c,
    kMessage093Size = 0x1c,
    kMessage094Size = 0xf4,
    kMessage095Size = 0x1c,
    kMessage096Size = 0x30,
    kMessage097Size = 0x1c,
    kMessage098Size = 0xf4,
    kMessage099Size = 0x1c,
    kMessage100Size = 0x30,
    kMessage101Size = 0x118,
    kMessage102Size = 0xf8,
    kMessage103Size = 0x1c,
    kMessage104Size = 0x2c,
    kMessage105Size = 0x1c,
    kMessage106Size = 0xf4,
    kMessage107Size = 0x1c,
    kMessage108Size = 0x34,
    kMessage109Size = 0xec,
    kMessage110Size = 0xee,
    kMessage111Size = 0x1c,
    kMessage112Size = 0xf4,
    kMessage113Size = 0x1c,
    kMessage114Size = 0x2c,
    kMessage115Size = 0x1c,
    kMessage116Size = 0xf8,
    kMessage117Size = 0x1c,
    kMessage118Size = 0x2c,
    kMessage119Size = 0x1c,
    kMessage120Size = 0x38,
    kMessage121Size = 0x28,
    kMessage122Size = 0x30,
    kMessage123Size = 0x114,
    kMessage124Size = 0x30,
    kMessage125Size = 0x170,
    kMessage126Size = 0x30,
    kMessage127Size = 0xf0,
    kMessage128Size = 0x2c,
    kMessage129Size = 0x1c,
    kMessage130Size = 0x3c,
    kMessage131Size = 0x1c0,
    kMessage132Size = 0x12c,
    kMessage133Size = 0x1c,
    kMessage134Size = 0x26,
    kMessage135Size = 0x4d,
    kMessage136Size = 0x30,
    kMessage137Size = 0x28,
    kMessage138Size = 0x38,
    kMessage139Size = 0x1c,
    kMessage140Size = 0x26,
    kMessage141Size = 0x64,
    kMessage142Size = 0x2c,
    kMessage143Size = 0x1c,
    kMessage144Size = 0x2c,
    kMessage145Size = 0x44,
    kMessage146Size = 0x38,
    kMessage147Size = 0x20,
    kMessage148Size = 0x3c,
    kMessage149Size = 0x48,
    kMessage150Size = 0x2c,
    kMessage151Size = 0x1c,
    kMessage152Size = 0x6c,
    kMessage153Size = 0x5c,
    kMessage154Size = 0x8,
    kMessage155Size = 0x2c,
    kMessage156Size = 0x20,
    kMessage157Size = 0x2c,
    kMessage158Size = 0x20,
    kMessage159Size = 0x2c,
    kMessage160Size = 0x44,
    kMessage161Size = 0x2c,
    kMessage162Size = 0x24,
    kMessage163Size = 0x30,
    kMessage164Size = 0x1c,
    kMessage165Size = 0x1d8,
    kMessage166Size = 0xc8,
    kMessage167Size = 0xcc,
    kMessage168Size = 0x1fc,
    kMessage170Size = 0xc4,
    kMessage171Size = 0xc8,
    kMessage172Size = 0xc4,
    kMessage173Size = 0x1fc,
    kMessage174Size = 0xc4,
    kMessage175Size = 0x1fc,
    kMessage176Size = 0xc4,
    kMessage177Size = 0x1c,
    kMessage178Size = 0xb0,
    kMessage179Size = 0xc8,
    kMessage180Size = 0x1d8,
    kMessage181Size = 0xc8,
    kMessage182Size = 0xc4,
    kMessage183Size = 0x1dc,
    kMessage184Size = 0x204,
    kMessage185Size = 0xc8,
    kMessage186Size = 0x204,
    kMessage187Size = 0x208,
    kMessage188Size = 0x155,
    kMessage189Size = 0x208,
    kMessage190Size = 0xc4,
    kMessage191Size = 0x1c,
    kMessage192Size = 0x26,
    kMessage193Size = 0x48,
    kMessage194Size = 0x2c,
    kMessage195Size = 0x1c,
    kMessage196Size = 0x2c,
    kMessage197Size = 0x1c,
    kMessage198Size = 0x30,
    kMessage199Size = 0x1c,
    kMessage200Size = 0x24,
    kMessage201Size = 0x120,
    kMessage202Size = 0x230,
    kMessage203Size = 0xa6,
    kMessage204Size = 0x30,
    kMessage205Size = 0x20,
    kMessage169Size = 0x28,
};

enum {
    // The status of a part the lobby client cannot send.
    kUploadStatusOffline = -966,
    // The receiver of the messages sent to the lobby server.
    kServerReceiver = 0xffff,
};

// The request of the lobby server for the next part of an upload. 0x28 bytes.
typedef struct {
    int startByteIndex;
    int packetNumber;
    int transferStatus;
    int status;
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE];
} MediusUploadPartRequest;

int rtMediusOnNotice(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusNoticeCallback != NULL) {
        g_pfnMediusNoticeCallback(connection, type, sender, response);
    }
    return kMessage074Size;
}

int rtMediusOnWorldReport0(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage000Size;
}

int rtMediusOnPlayerReport(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage001Size;
}

int rtMediusOnEndGameReport(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage002Size;
}

int rtMediusOnSessionBegin(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage003Size;
}

int rtMediusOnSessionBeginResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusSessionBeginResponse *response = (const rtMediusSessionBeginResponse *)message;

    if (response->header.status == RT_MEDIUS_STATUS_SUCCESS) {
        strncpy(g_szMediusSessionKey, response->sessionKey, RT_MEDIUS_SESSION_KEY_SIZE);
    }
    if (g_pfnMediusSessionBeginCallback != NULL) {
        g_pfnMediusSessionBeginCallback(connection, type, sender, response);
    }
    return kMessage004Size;
}

int rtMediusOnSessionEnd(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage005Size;
}

int rtMediusOnSessionEndResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (response->status == RT_MEDIUS_STATUS_SUCCESS) {
        g_szMediusSessionKey[0] = '\0';
    }
    if (g_pfnMediusSessionEndCallback != NULL) {
        g_pfnMediusSessionEndCallback(connection, type, sender, response);
    }
    return kMessage006Size;
}

int rtMediusOnMachineSignaturePost(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage203Size;
}

int rtMediusOnAccountLogin(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage007Size;
}

int rtMediusOnAccountLoginResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusLoginResponse *response = (const rtMediusLoginResponse *)message;

    if (response->header.status == RT_MEDIUS_STATUS_SUCCESS) {
        rtMediusSetServerAddress(&response->info.addresses[1]);
    }
    if (g_pfnMediusAccountLoginCallback != NULL) {
        g_pfnMediusAccountLoginCallback(connection, type, sender, response);
    }
    return kMessage008Size;
}

int rtMediusOnAccountLogout(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage021Size;
}

int rtMediusOnAccountLogoutResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusAccountLogoutCallback != NULL) {
        g_pfnMediusAccountLogoutCallback(connection, type, sender, response);
    }
    return kMessage022Size;
}

int rtMediusOnAccountRegistration(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage009Size;
}

int rtMediusOnAccountRegistrationResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusAccountRegistrationCallback != NULL) {
        g_pfnMediusAccountRegistrationCallback(connection, type, sender, response);
    }
    return kMessage010Size;
}

int rtMediusOnAccountGetProfile(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage011Size;
}

int rtMediusOnAccountGetProfileResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusAccountGetProfileCallback != NULL) {
        g_pfnMediusAccountGetProfileCallback(connection, type, sender, response);
    }
    return kMessage012Size;
}

int rtMediusOnAccountUpdateProfile(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage013Size;
}

int rtMediusOnAccountUpdateProfileResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusAccountUpdateProfileCallback != NULL) {
        g_pfnMediusAccountUpdateProfileCallback(connection, type, sender, response);
    }
    return kMessage014Size;
}

int rtMediusOnAccountUpdatePassword(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage015Size;
}

int rtMediusOnChangePasswordResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusChangePasswordCallback != NULL) {
        g_pfnMediusChangePasswordCallback(connection, type, sender, response);
    }
    return kMessage016Size;
}

int rtMediusOnAccountUpdateStats(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage017Size;
}

int rtMediusOnUpdateStatsResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusUpdateStatsCallback != NULL) {
        g_pfnMediusUpdateStatsCallback(connection, type, sender, response);
    }
    return kMessage018Size;
}

int rtMediusOnAnonymousLogin(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage025Size;
}

int rtMediusOnAnonymousLoginResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusLoginResponse *response = (const rtMediusLoginResponse *)message;

    if (g_pfnMediusAnonymousLoginCallback != NULL) {
        g_pfnMediusAnonymousLoginCallback(connection, type, sender, response);
    }
    return kMessage026Size;
}

int rtMediusOnCreateGameRequest0(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage029Size;
}

int rtMediusOnCreateGameResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusCreateGameCallback != NULL) {
        g_pfnMediusCreateGameCallback(connection, type, sender, response);
    }
    return kMessage030Size;
}

int rtMediusOnCreateGameOnSelf(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage031Size;
}

int rtMediusOnCreateGameOnSelfResponse(int connection,
                                       int type,
                                       int sender,
                                       rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusCreateGameOnSelfCallback != NULL) {
        g_pfnMediusCreateGameOnSelfCallback(connection, type, sender, response);
    }
    return kMessage032Size;
}

int rtMediusOnCreateChannelRequest0(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage033Size;
}

int rtMediusOnCreateChannelResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusCreateChannelResponse *response = (const rtMediusCreateChannelResponse *)message;

    if (g_pfnMediusCreateChannelCallback != NULL) {
        g_pfnMediusCreateChannelCallback(connection, type, sender, response);
    }
    return kMessage034Size;
}

int rtMediusOnJoinGameRequest0(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage035Size;
}

int rtMediusOnJoinGameResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusJoinGameResponse *response = (const rtMediusJoinGameResponse *)message;

    if (g_pfnMediusJoinGameCallback != NULL) {
        g_pfnMediusJoinGameCallback(connection, type, sender, response);
    }
    return kMessage036Size;
}

int rtMediusOnJoinChannel(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage037Size;
}

int rtMediusOnJoinChannelResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusJoinChannelResponse *response = (const rtMediusJoinChannelResponse *)message;

    if (g_pfnMediusJoinChannelCallback != NULL) {
        g_pfnMediusJoinChannelCallback(connection, type, sender, response);
    }
    return kMessage038Size;
}

int rtMediusOnJoinChannelFwd(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage039Size;
}

int rtMediusOnJoinChannelFwdResponse(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage040Size;
}

int rtMediusOnGameList(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage041Size;
}

int rtMediusOnGameListResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusGameListResponse *response = (const rtMediusGameListResponse *)message;

    if (g_pfnMediusGameListCallback != NULL) {
        g_pfnMediusGameListCallback(connection, type, sender, response);
    }
    return kMessage042Size;
}

int rtMediusOnChannelList(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage043Size;
}

int rtMediusOnChannelListResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusChannelListResponse *response = (const rtMediusChannelListResponse *)message;

    if (g_pfnMediusChannelListCallback != NULL) {
        g_pfnMediusChannelListCallback(connection, type, sender, response);
    }
    return kMessage044Size;
}

int rtMediusOnLobbyWorldPlayerList(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage045Size;
}

int rtMediusOnChannelPlayersResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusChannelPlayersResponse *response =
        (const rtMediusChannelPlayersResponse *)message;

    if (g_pfnMediusChannelPlayersCallback != NULL) {
        g_pfnMediusChannelPlayersCallback(connection, type, sender, response);
    }
    return kMessage046Size;
}

int rtMediusOnGameWorldPlayerList(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage047Size;
}

int rtMediusOnGamePlayersResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusGamePlayersResponse *response = (const rtMediusGamePlayersResponse *)message;

    if (g_pfnMediusGamePlayersCallback != NULL) {
        g_pfnMediusGamePlayersCallback(connection, type, sender, response);
    }
    return kMessage048Size;
}

int rtMediusOnPlayerInfo(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage049Size;
}

int rtMediusOnPlayerInfoResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusPlayerInfoResponse *response = (const rtMediusPlayerInfoResponse *)message;

    if (g_pfnMediusPlayerInfoCallback != NULL) {
        g_pfnMediusPlayerInfoCallback(connection, type, sender, response);
    }
    return kMessage050Size;
}

int rtMediusOnGameInfo0(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage051Size;
}

int rtMediusOnGameInfoResponse0(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGameInfoResponse0Callback != NULL) {
        g_pfnMediusGameInfoResponse0Callback(connection, type, sender, response);
    }
    return kMessage052Size;
}

int rtMediusOnChannelInfo(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage053Size;
}

int rtMediusOnChannelInfoResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusChannelInfoResponse *response = (const rtMediusChannelInfoResponse *)message;

    if (g_pfnMediusChannelInfoCallback != NULL) {
        g_pfnMediusChannelInfoCallback(connection, type, sender, response);
    }
    return kMessage054Size;
}

int rtMediusOnFindWorldByName(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage055Size;
}

int rtMediusOnFindWorldByNameResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusFindWorldByNameCallback != NULL) {
        g_pfnMediusFindWorldByNameCallback(connection, type, sender, response);
    }
    return kMessage056Size;
}

int rtMediusOnFindPlayer(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage057Size;
}

int rtMediusOnFindPlayerResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusFindPlayerResponse *response = (const rtMediusFindPlayerResponse *)message;

    if (g_pfnMediusFindPlayerCallback != NULL) {
        g_pfnMediusFindPlayerCallback(connection, type, sender, response);
    }
    return kMessage058Size;
}

int rtMediusOnChatMessage(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage059Size;
}

int rtMediusOnChatFwdMessage(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusChatMessage *response = (const rtMediusChatMessage *)message;

    if (g_pfnMediusChatFwdMessageCallback != NULL) {
        g_pfnMediusChatFwdMessageCallback(connection, type, sender, response);
    }
    return kMessage060Size;
}

int rtMediusOnGetBuddyList(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage061Size;
}

int rtMediusOnGetBuddyListResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetBuddyListCallback != NULL) {
        g_pfnMediusGetBuddyListCallback(connection, type, sender, response);
    }
    return kMessage062Size;
}

int rtMediusOnAddToBuddyList(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage063Size;
}

int rtMediusOnAddToBuddyListResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusAddToBuddyListCallback != NULL) {
        g_pfnMediusAddToBuddyListCallback(connection, type, sender, response);
    }
    return kMessage064Size;
}

int rtMediusOnRemoveFromBuddyList(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage065Size;
}

int rtMediusOnRemoveFromBuddyListResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusRemoveFromBuddyListCallback != NULL) {
        g_pfnMediusRemoveFromBuddyListCallback(connection, type, sender, response);
    }
    return kMessage066Size;
}

int rtMediusOnAddToBuddyListConfirmationRequest0(int connection,
                                                 int type,
                                                 int sender,
                                                 rtDmeMessage *message) {
    return kMessage067Size;
}

int rtMediusOnAddToBuddyListConfirmationResponse(int connection,
                                                 int type,
                                                 int sender,
                                                 rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusAddToBuddyListConfirmationCallback != NULL) {
        g_pfnMediusAddToBuddyListConfirmationCallback(connection, type, sender, response);
    }
    return kMessage068Size;
}

int rtMediusOnInitHandler04(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusInitHandler04Callback != NULL) {
        g_pfnMediusInitHandler04Callback(connection, type, sender, response);
    }
    return kMessage069Size;
}

int rtMediusOnAddToBuddyListFwdConfirmationResponse0(int connection,
                                                     int type,
                                                     int sender,
                                                     rtDmeMessage *message) {
    return kMessage070Size;
}

int rtMediusOnGetIgnoreList(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage192Size;
}

int rtMediusOnGetIgnoreListResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetIgnoreListCallback != NULL) {
        g_pfnMediusGetIgnoreListCallback(connection, type, sender, response);
    }
    return kMessage193Size;
}

int rtMediusOnAddToIgnoreList(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage194Size;
}

int rtMediusOnAddToIgnoreListResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusAddToIgnoreListCallback != NULL) {
        g_pfnMediusAddToIgnoreListCallback(connection, type, sender, response);
    }
    return kMessage195Size;
}

int rtMediusOnRemoveFromIgnoreList(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage196Size;
}

int rtMediusOnRemoveFromIgnoreListResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusRemoveFromIgnoreListCallback != NULL) {
        g_pfnMediusRemoveFromIgnoreListCallback(connection, type, sender, response);
    }
    return kMessage197Size;
}

int rtMediusOnPolicy(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage071Size;
}

int rtMediusOnGetPolicyResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusPolicyResponse *response = (const rtMediusPolicyResponse *)message;

    if (g_pfnMediusGetPolicyCallback != NULL) {
        g_pfnMediusGetPolicyCallback(connection, type, sender, response);
    }
    return kMessage072Size;
}

int rtMediusOnUpdateUserState(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage073Size;
}

int rtMediusOnGetAnnouncements(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage075Size;
}

int rtMediusOnGetAllAnnouncements(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage076Size;
}

int rtMediusOnGetAnnouncementsResponse(int connection,
                                       int type,
                                       int sender,
                                       rtDmeMessage *message) {
    rtMediusAnnouncementResponse *response = (rtMediusAnnouncementResponse *)message;

    if (g_pfnMediusGetAnnouncementsCallback != NULL) {
        g_pfnMediusGetAnnouncementsCallback(connection, type, sender, response);
    }
    return kMessage077Size;
}

int rtMediusOnSetGameListFilter0(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage078Size;
}

int rtMediusOnSetGameListFilterResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusSetGameListFilterCallback != NULL) {
        g_pfnMediusSetGameListFilterCallback(connection, type, sender, response);
    }
    return kMessage079Size;
}

int rtMediusOnClearGameListFilter0(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage080Size;
}

int rtMediusOnClearGameListFilterResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusClearGameListFilterCallback != NULL) {
        g_pfnMediusClearGameListFilterCallback(connection, type, sender, response);
    }
    return kMessage081Size;
}

int rtMediusOnGetGameListFilter(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage082Size;
}

int rtMediusOnGetGameListFilterResponse0(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetGameListFilterResponse0Callback != NULL) {
        g_pfnMediusGetGameListFilterResponse0Callback(connection, type, sender, response);
    }
    return kMessage083Size;
}

int rtMediusOnCreateClan(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage084Size;
}

int rtMediusOnCreateClanResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusCreateClanCallback != NULL) {
        g_pfnMediusCreateClanCallback(connection, type, sender, response);
    }
    return kMessage085Size;
}

int rtMediusOnDisbandClan(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage086Size;
}

int rtMediusOnDisbandClanResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusDisbandClanCallback != NULL) {
        g_pfnMediusDisbandClanCallback(connection, type, sender, response);
    }
    return kMessage087Size;
}

int rtMediusOnGetClanByID(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage088Size;
}

int rtMediusOnGetClanByIDResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetClanByIDCallback != NULL) {
        g_pfnMediusGetClanByIDCallback(connection, type, sender, response);
    }
    return kMessage089Size;
}

int rtMediusOnGetClanByName(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage090Size;
}

int rtMediusOnGetClanByNameResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetClanByNameCallback != NULL) {
        g_pfnMediusGetClanByNameCallback(connection, type, sender, response);
    }
    return kMessage091Size;
}

int rtMediusOnTransferClanLeadership(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage092Size;
}

int rtMediusOnTransferClanLeadershipResponse(int connection,
                                             int type,
                                             int sender,
                                             rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusTransferClanLeadershipCallback != NULL) {
        g_pfnMediusTransferClanLeadershipCallback(connection, type, sender, response);
    }
    return kMessage093Size;
}

int rtMediusOnAddPlayerToClan(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage094Size;
}

int rtMediusOnAddPlayerToClanResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusAddPlayerToClanCallback != NULL) {
        g_pfnMediusAddPlayerToClanCallback(connection, type, sender, response);
    }
    return kMessage095Size;
}

int rtMediusOnRemovePlayerFromClan(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage096Size;
}

int rtMediusOnRemovePlayerFromClanResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusRemovePlayerFromClanCallback != NULL) {
        g_pfnMediusRemovePlayerFromClanCallback(connection, type, sender, response);
    }
    return kMessage097Size;
}

int rtMediusOnInvitePlayerToClan(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage098Size;
}

int rtMediusOnInvitePlayerToClanResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusInvitePlayerToClanCallback != NULL) {
        g_pfnMediusInvitePlayerToClanCallback(connection, type, sender, response);
    }
    return kMessage099Size;
}

int rtMediusOnCheckMyClanInvitations(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage100Size;
}

int rtMediusOnCheckMyClanInvitationsResponse(int connection,
                                             int type,
                                             int sender,
                                             rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusCheckMyClanInvitationsCallback != NULL) {
        g_pfnMediusCheckMyClanInvitationsCallback(connection, type, sender, response);
    }
    return kMessage101Size;
}

int rtMediusOnGetClanInvitationsSent(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage122Size;
}

int rtMediusOnGetClanInvitationsSentResponse(int connection,
                                             int type,
                                             int sender,
                                             rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetClanInvitationsSentCallback != NULL) {
        g_pfnMediusGetClanInvitationsSentCallback(connection, type, sender, response);
    }
    return kMessage123Size;
}

int rtMediusOnRespondToClanInvitation(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage102Size;
}

int rtMediusOnRespondToClanInvitationResponse(int connection,
                                              int type,
                                              int sender,
                                              rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusRespondToClanInvitationCallback != NULL) {
        g_pfnMediusRespondToClanInvitationCallback(connection, type, sender, response);
    }
    return kMessage103Size;
}

int rtMediusOnRevokeClanInvitation(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage104Size;
}

int rtMediusOnRevokeClanInvitationResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusRevokeClanInvitationCallback != NULL) {
        g_pfnMediusRevokeClanInvitationCallback(connection, type, sender, response);
    }
    return kMessage105Size;
}

int rtMediusOnRequestClanTeamChallenge(int connection,
                                       int type,
                                       int sender,
                                       rtDmeMessage *message) {
    return kMessage106Size;
}

int rtMediusOnRequestClanTeamChallengeResponse(int connection,
                                               int type,
                                               int sender,
                                               rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusRequestClanTeamChallengeCallback != NULL) {
        g_pfnMediusRequestClanTeamChallengeCallback(connection, type, sender, response);
    }
    return kMessage107Size;
}

int rtMediusOnGetMyClanMessages(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage108Size;
}

int rtMediusOnGetMyClanMessagesResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetMyClanMessagesCallback != NULL) {
        g_pfnMediusGetMyClanMessagesCallback(connection, type, sender, response);
    }
    return kMessage109Size;
}

int rtMediusOnSendClanMessage(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage110Size;
}

int rtMediusOnSendClanMessageResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusSendClanMessageCallback != NULL) {
        g_pfnMediusSendClanMessageCallback(connection, type, sender, response);
    }
    return kMessage111Size;
}

int rtMediusOnModifyClanMessage(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage112Size;
}

int rtMediusOnModifyClanMessageResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusModifyClanMessageCallback != NULL) {
        g_pfnMediusModifyClanMessageCallback(connection, type, sender, response);
    }
    return kMessage113Size;
}

int rtMediusOnDeleteClanMessage(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage114Size;
}

int rtMediusOnDeleteClanMessageResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusDeleteClanMessageCallback != NULL) {
        g_pfnMediusDeleteClanMessageCallback(connection, type, sender, response);
    }
    return kMessage115Size;
}

int rtMediusOnRespondToClanTeamChallenge(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message) {
    return kMessage116Size;
}

int rtMediusOnRespondToClanTeamChallengeResponse(int connection,
                                                 int type,
                                                 int sender,
                                                 rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusRespondToClanTeamChallengeCallback != NULL) {
        g_pfnMediusRespondToClanTeamChallengeCallback(connection, type, sender, response);
    }
    return kMessage117Size;
}

int rtMediusOnRevokeClanTeamChallenge(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage118Size;
}

int rtMediusOnRevokeClanTeamChallengeResponse(int connection,
                                              int type,
                                              int sender,
                                              rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusRevokeClanTeamChallengeCallback != NULL) {
        g_pfnMediusRevokeClanTeamChallengeCallback(connection, type, sender, response);
    }
    return kMessage119Size;
}

int rtMediusOnGetMyClans(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage124Size;
}

int rtMediusOnGetMyClansResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetMyClansCallback != NULL) {
        g_pfnMediusGetMyClansCallback(connection, type, sender, response);
    }
    return kMessage125Size;
}

int rtMediusOnGetClanTeamChallenges(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage130Size;
}

int rtMediusOnGetClanTeamChallengesResponse(int connection,
                                            int type,
                                            int sender,
                                            rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetClanTeamChallengesCallback != NULL) {
        g_pfnMediusGetClanTeamChallengesCallback(connection, type, sender, response);
    }
    return kMessage131Size;
}

int rtMediusOnConfirmClanTeamChallenge(int connection,
                                       int type,
                                       int sender,
                                       rtDmeMessage *message) {
    return kMessage128Size;
}

int rtMediusOnConfirmClanTeamChallengeResponse(int connection,
                                               int type,
                                               int sender,
                                               rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusConfirmClanTeamChallengeCallback != NULL) {
        g_pfnMediusConfirmClanTeamChallengeCallback(connection, type, sender, response);
    }
    return kMessage129Size;
}

int rtMediusOnGetAllClanMessages(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage126Size;
}

int rtMediusOnGetAllClanMessagesResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetAllClanMessagesCallback != NULL) {
        g_pfnMediusGetAllClanMessagesCallback(connection, type, sender, response);
    }
    return kMessage127Size;
}

int rtMediusOnGetClanTeamChallengeHistory(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message) {
    return kMessage120Size;
}

int rtMediusOnGetClanTeamChallengeHistoryResponse(int connection,
                                                  int type,
                                                  int sender,
                                                  rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetClanTeamChallengeHistoryCallback != NULL) {
        g_pfnMediusGetClanTeamChallengeHistoryCallback(connection, type, sender, response);
    }
    return kMessage121Size;
}

int rtMediusOnUpdateClanStats(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage132Size;
}

int rtMediusOnUpdateClanStatsResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusUpdateClanStatsCallback != NULL) {
        g_pfnMediusUpdateClanStatsCallback(connection, type, sender, response);
    }
    return kMessage133Size;
}

int rtMediusOnGetMyIP(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage027Size;
}

int rtMediusOnGetMyIPResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetMyIPCallback != NULL) {
        g_pfnMediusGetMyIPCallback(connection, type, sender, response);
    }
    return kMessage028Size;
}

int rtMediusOnVersionServer(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage134Size;
}

int rtMediusOnVersionServerResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusVersionResponse *response = (const rtMediusVersionResponse *)message;

    if (g_pfnMediusVersionServerCallback != NULL) {
        g_pfnMediusVersionServerCallback(connection, type, sender, response);
    }
    return kMessage135Size;
}

int rtMediusOnGetWorldSecurityLevel(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage136Size;
}

int rtMediusOnGetWorldSecurityLevelResponse(int connection,
                                            int type,
                                            int sender,
                                            rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetWorldSecurityLevelCallback != NULL) {
        g_pfnMediusGetWorldSecurityLevelCallback(connection, type, sender, response);
    }
    return kMessage137Size;
}

int rtMediusOnBanPlayer(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage138Size;
}

int rtMediusOnBanPlayerResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusBanPlayerCallback != NULL) {
        g_pfnMediusBanPlayerCallback(connection, type, sender, response);
    }
    return kMessage139Size;
}

int rtMediusOnAccountDelete(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage019Size;
}

int rtMediusOnAccountDeleteResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusAccountDeleteCallback != NULL) {
        g_pfnMediusAccountDeleteCallback(connection, type, sender, response);
    }
    return kMessage020Size;
}

int rtMediusOnAccountGetId(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage023Size;
}

int rtMediusOnAccountGetIdResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusAccountGetIdResponse *response = (const rtMediusAccountGetIdResponse *)message;

    if (g_pfnMediusAccountGetIdCallback != NULL) {
        g_pfnMediusAccountGetIdCallback(connection, type, sender, response);
    }
    return kMessage024Size;
}

int rtMediusOnGetLocations(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage140Size;
}

int rtMediusOnGetLocationsResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetLocationsCallback != NULL) {
        g_pfnMediusGetLocationsCallback(connection, type, sender, response);
    }
    return kMessage141Size;
}

int rtMediusOnPickLocation(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage142Size;
}

int rtMediusOnPickLocationResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusPickLocationCallback != NULL) {
        g_pfnMediusPickLocationCallback(connection, type, sender, response);
    }
    return kMessage143Size;
}

int rtMediusOnGetClanMemberList(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage144Size;
}

int rtMediusOnGetClanMemberListResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetClanMemberListCallback != NULL) {
        g_pfnMediusGetClanMemberListCallback(connection, type, sender, response);
    }
    return kMessage145Size;
}

int rtMediusOnLadderPosition(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage146Size;
}

int rtMediusOnLadderPositionResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusLadderPositionResponse *response =
        (const rtMediusLadderPositionResponse *)message;

    if (g_pfnMediusLadderPositionCallback != NULL) {
        g_pfnMediusLadderPositionCallback(connection, type, sender, response);
    }
    return kMessage147Size;
}

int rtMediusOnLadderPositionFast(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage204Size;
}

int rtMediusOnLadderPositionFastResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message) {
    const rtMediusLadderPositionResponse *response =
        (const rtMediusLadderPositionResponse *)message;

    if (g_pfnMediusLadderPositionFastCallback != NULL) {
        g_pfnMediusLadderPositionFastCallback(connection, type, sender, response);
    }
    return kMessage205Size;
}

int rtMediusOnLadderList(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage148Size;
}

int rtMediusOnLadderListResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusLadderListResponse *response = (const rtMediusLadderListResponse *)message;

    if (g_pfnMediusLadderListCallback != NULL) {
        g_pfnMediusLadderListCallback(connection, type, sender, response);
    }
    return kMessage149Size;
}

int rtMediusOnChatToggle(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage150Size;
}

int rtMediusOnChatToggleResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusChatToggleCallback != NULL) {
        g_pfnMediusChatToggleCallback(connection, type, sender, response);
    }
    return kMessage151Size;
}

int rtMediusOnTextFilter(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage152Size;
}

int rtMediusOnTextFilterResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusTextFilterResponse *response = (const rtMediusTextFilterResponse *)message;

    if (g_pfnMediusTextFilterCallback != NULL) {
        g_pfnMediusTextFilterCallback(connection, type, sender, response);
    }
    return kMessage153Size;
}

int rtMediusOnInitHandler0C(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusInitHandler0CCallback != NULL) {
        g_pfnMediusInitHandler0CCallback(connection, type, sender, response);
    }
    return kMessage154Size;
}

int rtMediusOnGetTotalGames(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage155Size;
}

int rtMediusOnGetTotalGamesResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetTotalGamesCallback != NULL) {
        g_pfnMediusGetTotalGamesCallback(connection, type, sender, response);
    }
    return kMessage156Size;
}

int rtMediusOnGetTotalChannels(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage157Size;
}

int rtMediusOnGetTotalChannelsResponse(int connection,
                                       int type,
                                       int sender,
                                       rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetTotalChannelsCallback != NULL) {
        g_pfnMediusGetTotalChannelsCallback(connection, type, sender, response);
    }
    return kMessage158Size;
}

int rtMediusOnGetLobbyPlayerNames(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage159Size;
}

int rtMediusOnGetLobbyPlayerNamesResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetLobbyPlayerNamesCallback != NULL) {
        g_pfnMediusGetLobbyPlayerNamesCallback(connection, type, sender, response);
    }
    return kMessage160Size;
}

int rtMediusOnGetTotalUsers(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage161Size;
}

int rtMediusOnGetTotalUsersResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetTotalUsersCallback != NULL) {
        g_pfnMediusGetTotalUsersCallback(connection, type, sender, response);
    }
    return kMessage162Size;
}

int rtMediusOnSetLocalizationParams(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage163Size;
}

int rtMediusOnSetLocalizationParamsResponse(int connection,
                                            int type,
                                            int sender,
                                            rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusSetLocalizationParamsCallback != NULL) {
        g_pfnMediusSetLocalizationParamsCallback(connection, type, sender, response);
    }
    return kMessage164Size;
}

int rtMediusOnSetMessageAsRead(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage198Size;
}

int rtMediusOnSetMessageAsReadResponse(int connection,
                                       int type,
                                       int sender,
                                       rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusSetMessageAsReadCallback != NULL) {
        g_pfnMediusSetMessageAsReadCallback(connection, type, sender, response);
    }
    return kMessage199Size;
}

int rtMediusOnFileCreate(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage165Size;
}

int rtMediusOnFileCreateResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusFileResponse *response = (const rtMediusFileResponse *)message;

    if (g_pfnMediusFileCreateCallback != NULL) {
        g_pfnMediusFileCreateCallback(connection, type, sender, response);
    }
    return kMessage166Size;
}

int rtMediusOnFileUpload(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage167Size;
}

int rtMediusOnFileUploadResponse(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage168Size;
}

int rtMediusOnFileClose(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage170Size;
}

int rtMediusOnFileCloseResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusFileResponse *response = (const rtMediusFileResponse *)message;

    if (g_pfnMediusFileCloseCallback != NULL) {
        g_pfnMediusFileCloseCallback(connection, type, sender, response);
    }
    return kMessage171Size;
}

int rtMediusOnFileDownload(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage172Size;
}

int rtMediusOnFileDownloadResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusFileTransferResponse *response = (const rtMediusFileTransferResponse *)message;

    if (g_pfnMediusFileDownloadCallback != NULL) {
        g_pfnMediusFileDownloadCallback(connection, type, sender, response);
    }
    return kMessage173Size;
}

int rtMediusOnFileDownloadStream(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage174Size;
}

int rtMediusOnFileDownloadStreamResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusFileDownloadStreamCallback != NULL) {
        g_pfnMediusFileDownloadStreamCallback(connection, type, sender, response);
    }
    return kMessage175Size;
}

int rtMediusOnFileDelete(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage176Size;
}

int rtMediusOnFileDeleteResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusFileDeleteResponse *response = (const rtMediusFileDeleteResponse *)message;

    if (g_pfnMediusFileDeleteCallback != NULL) {
        g_pfnMediusFileDeleteCallback(connection, type, sender, response);
    }
    return kMessage177Size;
}

int rtMediusOnFileListFiles(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage178Size;
}

int rtMediusOnFileListResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusFileResponse *response = (const rtMediusFileResponse *)message;

    if (g_pfnMediusFileListCallback != NULL) {
        g_pfnMediusFileListCallback(connection, type, sender, response);
    }
    return kMessage179Size;
}

int rtMediusOnFileUpdateAttributes(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage180Size;
}

int rtMediusOnFileUpdateAttributesResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusFileUpdateAttributesCallback != NULL) {
        g_pfnMediusFileUpdateAttributesCallback(connection, type, sender, response);
    }
    return kMessage181Size;
}

int rtMediusOnFileGetAttributes(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage182Size;
}

int rtMediusOnFileGetAttributesResponse(int connection,
                                        int type,
                                        int sender,
                                        rtDmeMessage *message) {
    const rtMediusFileAttributesResponse *response =
        (const rtMediusFileAttributesResponse *)message;

    if (g_pfnMediusFileGetAttributesCallback != NULL) {
        g_pfnMediusFileGetAttributesCallback(connection, type, sender, response);
    }
    return kMessage183Size;
}

int rtMediusOnFileUpdateMetaData(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage184Size;
}

int rtMediusOnFileUpdateMetaDataResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusFileUpdateMetaDataCallback != NULL) {
        g_pfnMediusFileUpdateMetaDataCallback(connection, type, sender, response);
    }
    return kMessage185Size;
}

int rtMediusOnFileGetMetaData(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage186Size;
}

int rtMediusOnFileGetMetaDataResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusFileGetMetaDataCallback != NULL) {
        g_pfnMediusFileGetMetaDataCallback(connection, type, sender, response);
    }
    return kMessage187Size;
}

int rtMediusOnFileSearchByMetaData(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage188Size;
}

int rtMediusOnFileSearchByMetaDataResponse(int connection,
                                           int type,
                                           int sender,
                                           rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusFileSearchByMetaDataCallback != NULL) {
        g_pfnMediusFileSearchByMetaDataCallback(connection, type, sender, response);
    }
    return kMessage189Size;
}

int rtMediusOnFileCancelOperation(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage190Size;
}

int rtMediusOnFileCancelOperationResponse(int connection,
                                          int type,
                                          int sender,
                                          rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusFileCancelOperationCallback != NULL) {
        g_pfnMediusFileCancelOperationCallback(connection, type, sender, response);
    }
    return kMessage191Size;
}

int rtMediusOnGetUniverseInformation(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusGetUniverseInformationCallback != NULL) {
        g_pfnMediusGetUniverseInformationCallback(connection, type, sender, response);
    }
    return kMessage200Size;
}

int rtMediusOnUniverseNewsResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusUniverseNewsCallback != NULL) {
        g_pfnMediusUniverseNewsCallback(connection, type, sender, response);
    }
    return kMessage201Size;
}

int rtMediusOnUniverseStatusListResponse(int connection,
                                         int type,
                                         int sender,
                                         rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusUniverseStatusListCallback != NULL) {
        g_pfnMediusUniverseStatusListCallback(connection, type, sender, response);
    }
    return kMessage202Size;
}

int rtMediusOnFileUploadRequest(int connection, int type, int sender, rtDmeMessage *message) {
    const MediusUploadPartRequest *request = (const MediusUploadPartRequest *)message;
    rtMediusFileTransferResponse part;

    part.dataSize = 0;
    part.status = request->status;
    part.transferStatus = request->transferStatus;
    part.startByteIndex = request->startByteIndex;
    part.packetNumber = request->packetNumber;
    memcpy(part.messageId, request->messageId, sizeof(part.messageId));
    if (part.transferStatus != 0) {
        int nSize = g_nMediusUploadSize - part.startByteIndex;
        if (nSize > RT_MEDIUS_FILE_DATA_SIZE) {
            nSize = RT_MEDIUS_FILE_DATA_SIZE;
        }
        part.dataSize = nSize;
        memcpy(part.data, &g_pMediusUploadBuffer[part.startByteIndex], nSize);
    }
    if (g_nMediusInitialized != 1) {
        part.status = kUploadStatusOffline;
    }
    if (g_nMediusConnection < 0) {
        part.status = kUploadStatusOffline;
    }
    if (g_pfnMediusFileUploadCallback != NULL) {
        g_pfnMediusFileUploadCallback(connection, type, sender, &part);
    }
    rtDmeSendClassMessage(RT_DME_SEND_GUARANTEED,
                          g_nMediusConnection,
                          kServerReceiver,
                          RT_DME_CLASS_LOBBY,
                          g_nMediusFileUploadPartType,
                          sizeof(part),
                          &part);
    return kMessage169Size;
}
