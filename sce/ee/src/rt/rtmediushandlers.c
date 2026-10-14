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

int rtMediusOnMessage000(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage000Size;
}

int rtMediusOnMessage001(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage001Size;
}

int rtMediusOnMessage002(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage002Size;
}

int rtMediusOnMessage003(int connection, int type, int sender, rtDmeMessage *message) {
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

int rtMediusOnMessage005(int connection, int type, int sender, rtDmeMessage *message) {
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

int rtMediusOnMessage203(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage203Size;
}

int rtMediusOnMessage007(int connection, int type, int sender, rtDmeMessage *message) {
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

int rtMediusOnMessage021(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage021Size;
}

int rtMediusOnMessage022(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage022Callback != NULL) {
        g_pfnMediusMessage022Callback(connection, type, sender, response);
    }
    return kMessage022Size;
}

int rtMediusOnMessage009(int connection, int type, int sender, rtDmeMessage *message) {
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

int rtMediusOnMessage011(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage011Size;
}

int rtMediusOnMessage012(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage012Callback != NULL) {
        g_pfnMediusMessage012Callback(connection, type, sender, response);
    }
    return kMessage012Size;
}

int rtMediusOnMessage013(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage013Size;
}

int rtMediusOnMessage014(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage014Callback != NULL) {
        g_pfnMediusMessage014Callback(connection, type, sender, response);
    }
    return kMessage014Size;
}

int rtMediusOnMessage015(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage015Size;
}

int rtMediusOnChangePasswordResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusChangePasswordCallback != NULL) {
        g_pfnMediusChangePasswordCallback(connection, type, sender, response);
    }
    return kMessage016Size;
}

int rtMediusOnMessage017(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage017Size;
}

int rtMediusOnUpdateStatsResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusUpdateStatsCallback != NULL) {
        g_pfnMediusUpdateStatsCallback(connection, type, sender, response);
    }
    return kMessage018Size;
}

int rtMediusOnMessage025(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage025Size;
}

int rtMediusOnAnonymousLoginResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusLoginResponse *response = (const rtMediusLoginResponse *)message;

    if (g_pfnMediusAnonymousLoginCallback != NULL) {
        g_pfnMediusAnonymousLoginCallback(connection, type, sender, response);
    }
    return kMessage026Size;
}

int rtMediusOnMessage029(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage029Size;
}

int rtMediusOnMessage030(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage030Callback != NULL) {
        g_pfnMediusMessage030Callback(connection, type, sender, response);
    }
    return kMessage030Size;
}

int rtMediusOnMessage031(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage031Size;
}

int rtMediusOnMessage032(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage032Callback != NULL) {
        g_pfnMediusMessage032Callback(connection, type, sender, response);
    }
    return kMessage032Size;
}

int rtMediusOnMessage033(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage033Size;
}

int rtMediusOnCreateChannelResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusCreateChannelResponse *response = (const rtMediusCreateChannelResponse *)message;

    if (g_pfnMediusCreateChannelCallback != NULL) {
        g_pfnMediusCreateChannelCallback(connection, type, sender, response);
    }
    return kMessage034Size;
}

int rtMediusOnMessage035(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage035Size;
}

int rtMediusOnJoinGameResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusJoinGameResponse *response = (const rtMediusJoinGameResponse *)message;

    if (g_pfnMediusJoinGameCallback != NULL) {
        g_pfnMediusJoinGameCallback(connection, type, sender, response);
    }
    return kMessage036Size;
}

int rtMediusOnMessage037(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage037Size;
}

int rtMediusOnJoinChannelResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusJoinChannelResponse *response = (const rtMediusJoinChannelResponse *)message;

    if (g_pfnMediusJoinChannelCallback != NULL) {
        g_pfnMediusJoinChannelCallback(connection, type, sender, response);
    }
    return kMessage038Size;
}

int rtMediusOnMessage039(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage039Size;
}

int rtMediusOnMessage040(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage040Size;
}

int rtMediusOnMessage041(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage041Size;
}

int rtMediusOnGameListResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusGameListResponse *response = (const rtMediusGameListResponse *)message;

    if (g_pfnMediusGameListCallback != NULL) {
        g_pfnMediusGameListCallback(connection, type, sender, response);
    }
    return kMessage042Size;
}

int rtMediusOnMessage043(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage043Size;
}

int rtMediusOnChannelListResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusChannelListResponse *response = (const rtMediusChannelListResponse *)message;

    if (g_pfnMediusChannelListCallback != NULL) {
        g_pfnMediusChannelListCallback(connection, type, sender, response);
    }
    return kMessage044Size;
}

int rtMediusOnMessage045(int connection, int type, int sender, rtDmeMessage *message) {
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

int rtMediusOnMessage047(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage047Size;
}

int rtMediusOnGamePlayersResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusGamePlayersResponse *response = (const rtMediusGamePlayersResponse *)message;

    if (g_pfnMediusGamePlayersCallback != NULL) {
        g_pfnMediusGamePlayersCallback(connection, type, sender, response);
    }
    return kMessage048Size;
}

int rtMediusOnMessage049(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage049Size;
}

int rtMediusOnPlayerInfoResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusPlayerInfoResponse *response = (const rtMediusPlayerInfoResponse *)message;

    if (g_pfnMediusPlayerInfoCallback != NULL) {
        g_pfnMediusPlayerInfoCallback(connection, type, sender, response);
    }
    return kMessage050Size;
}

int rtMediusOnMessage051(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage051Size;
}

int rtMediusOnMessage052(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage052Callback != NULL) {
        g_pfnMediusMessage052Callback(connection, type, sender, response);
    }
    return kMessage052Size;
}

int rtMediusOnMessage053(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage053Size;
}

int rtMediusOnChannelInfoResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusChannelInfoResponse *response = (const rtMediusChannelInfoResponse *)message;

    if (g_pfnMediusChannelInfoCallback != NULL) {
        g_pfnMediusChannelInfoCallback(connection, type, sender, response);
    }
    return kMessage054Size;
}

int rtMediusOnMessage055(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage055Size;
}

int rtMediusOnMessage056(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage056Callback != NULL) {
        g_pfnMediusMessage056Callback(connection, type, sender, response);
    }
    return kMessage056Size;
}

int rtMediusOnMessage057(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage057Size;
}

int rtMediusOnFindPlayerResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusFindPlayerResponse *response = (const rtMediusFindPlayerResponse *)message;

    if (g_pfnMediusFindPlayerCallback != NULL) {
        g_pfnMediusFindPlayerCallback(connection, type, sender, response);
    }
    return kMessage058Size;
}

int rtMediusOnMessage059(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage059Size;
}

int rtMediusOnChatMessage(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusChatMessage *response = (const rtMediusChatMessage *)message;

    if (g_pfnMediusChatMessageCallback != NULL) {
        g_pfnMediusChatMessageCallback(connection, type, sender, response);
    }
    return kMessage060Size;
}

int rtMediusOnMessage061(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage061Size;
}

int rtMediusOnMessage062(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage062Callback != NULL) {
        g_pfnMediusMessage062Callback(connection, type, sender, response);
    }
    return kMessage062Size;
}

int rtMediusOnMessage063(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage063Size;
}

int rtMediusOnMessage064(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage064Callback != NULL) {
        g_pfnMediusMessage064Callback(connection, type, sender, response);
    }
    return kMessage064Size;
}

int rtMediusOnMessage065(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage065Size;
}

int rtMediusOnMessage066(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage066Callback != NULL) {
        g_pfnMediusMessage066Callback(connection, type, sender, response);
    }
    return kMessage066Size;
}

int rtMediusOnMessage067(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage067Size;
}

int rtMediusOnMessage068(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage068Callback != NULL) {
        g_pfnMediusMessage068Callback(connection, type, sender, response);
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

int rtMediusOnMessage070(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage070Size;
}

int rtMediusOnMessage192(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage192Size;
}

int rtMediusOnMessage193(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage193Callback != NULL) {
        g_pfnMediusMessage193Callback(connection, type, sender, response);
    }
    return kMessage193Size;
}

int rtMediusOnMessage194(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage194Size;
}

int rtMediusOnMessage195(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage195Callback != NULL) {
        g_pfnMediusMessage195Callback(connection, type, sender, response);
    }
    return kMessage195Size;
}

int rtMediusOnMessage196(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage196Size;
}

int rtMediusOnMessage197(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage197Callback != NULL) {
        g_pfnMediusMessage197Callback(connection, type, sender, response);
    }
    return kMessage197Size;
}

int rtMediusOnMessage071(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage071Size;
}

int rtMediusOnGetPolicyResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusPolicyResponse *response = (const rtMediusPolicyResponse *)message;

    if (g_pfnMediusGetPolicyCallback != NULL) {
        g_pfnMediusGetPolicyCallback(connection, type, sender, response);
    }
    return kMessage072Size;
}

int rtMediusOnMessage073(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage073Size;
}

int rtMediusOnMessage075(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage075Size;
}

int rtMediusOnMessage076(int connection, int type, int sender, rtDmeMessage *message) {
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

int rtMediusOnMessage078(int connection, int type, int sender, rtDmeMessage *message) {
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

int rtMediusOnMessage080(int connection, int type, int sender, rtDmeMessage *message) {
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

int rtMediusOnMessage082(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage082Size;
}

int rtMediusOnMessage083(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage083Callback != NULL) {
        g_pfnMediusMessage083Callback(connection, type, sender, response);
    }
    return kMessage083Size;
}

int rtMediusOnMessage084(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage084Size;
}

int rtMediusOnMessage085(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage085Callback != NULL) {
        g_pfnMediusMessage085Callback(connection, type, sender, response);
    }
    return kMessage085Size;
}

int rtMediusOnMessage086(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage086Size;
}

int rtMediusOnMessage087(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage087Callback != NULL) {
        g_pfnMediusMessage087Callback(connection, type, sender, response);
    }
    return kMessage087Size;
}

int rtMediusOnMessage088(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage088Size;
}

int rtMediusOnMessage089(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage089Callback != NULL) {
        g_pfnMediusMessage089Callback(connection, type, sender, response);
    }
    return kMessage089Size;
}

int rtMediusOnMessage090(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage090Size;
}

int rtMediusOnMessage091(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage091Callback != NULL) {
        g_pfnMediusMessage091Callback(connection, type, sender, response);
    }
    return kMessage091Size;
}

int rtMediusOnMessage092(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage092Size;
}

int rtMediusOnMessage093(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage093Callback != NULL) {
        g_pfnMediusMessage093Callback(connection, type, sender, response);
    }
    return kMessage093Size;
}

int rtMediusOnMessage094(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage094Size;
}

int rtMediusOnMessage095(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage095Callback != NULL) {
        g_pfnMediusMessage095Callback(connection, type, sender, response);
    }
    return kMessage095Size;
}

int rtMediusOnMessage096(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage096Size;
}

int rtMediusOnMessage097(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage097Callback != NULL) {
        g_pfnMediusMessage097Callback(connection, type, sender, response);
    }
    return kMessage097Size;
}

int rtMediusOnMessage098(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage098Size;
}

int rtMediusOnMessage099(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage099Callback != NULL) {
        g_pfnMediusMessage099Callback(connection, type, sender, response);
    }
    return kMessage099Size;
}

int rtMediusOnMessage100(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage100Size;
}

int rtMediusOnMessage101(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage101Callback != NULL) {
        g_pfnMediusMessage101Callback(connection, type, sender, response);
    }
    return kMessage101Size;
}

int rtMediusOnMessage122(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage122Size;
}

int rtMediusOnMessage123(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage123Callback != NULL) {
        g_pfnMediusMessage123Callback(connection, type, sender, response);
    }
    return kMessage123Size;
}

int rtMediusOnMessage102(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage102Size;
}

int rtMediusOnMessage103(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage103Callback != NULL) {
        g_pfnMediusMessage103Callback(connection, type, sender, response);
    }
    return kMessage103Size;
}

int rtMediusOnMessage104(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage104Size;
}

int rtMediusOnMessage105(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage105Callback != NULL) {
        g_pfnMediusMessage105Callback(connection, type, sender, response);
    }
    return kMessage105Size;
}

int rtMediusOnMessage106(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage106Size;
}

int rtMediusOnMessage107(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage107Callback != NULL) {
        g_pfnMediusMessage107Callback(connection, type, sender, response);
    }
    return kMessage107Size;
}

int rtMediusOnMessage108(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage108Size;
}

int rtMediusOnMessage109(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage109Callback != NULL) {
        g_pfnMediusMessage109Callback(connection, type, sender, response);
    }
    return kMessage109Size;
}

int rtMediusOnMessage110(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage110Size;
}

int rtMediusOnMessage111(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage111Callback != NULL) {
        g_pfnMediusMessage111Callback(connection, type, sender, response);
    }
    return kMessage111Size;
}

int rtMediusOnMessage112(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage112Size;
}

int rtMediusOnMessage113(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage113Callback != NULL) {
        g_pfnMediusMessage113Callback(connection, type, sender, response);
    }
    return kMessage113Size;
}

int rtMediusOnMessage114(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage114Size;
}

int rtMediusOnMessage115(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage115Callback != NULL) {
        g_pfnMediusMessage115Callback(connection, type, sender, response);
    }
    return kMessage115Size;
}

int rtMediusOnMessage116(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage116Size;
}

int rtMediusOnMessage117(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage117Callback != NULL) {
        g_pfnMediusMessage117Callback(connection, type, sender, response);
    }
    return kMessage117Size;
}

int rtMediusOnMessage118(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage118Size;
}

int rtMediusOnMessage119(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage119Callback != NULL) {
        g_pfnMediusMessage119Callback(connection, type, sender, response);
    }
    return kMessage119Size;
}

int rtMediusOnMessage124(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage124Size;
}

int rtMediusOnMessage125(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage125Callback != NULL) {
        g_pfnMediusMessage125Callback(connection, type, sender, response);
    }
    return kMessage125Size;
}

int rtMediusOnMessage130(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage130Size;
}

int rtMediusOnMessage131(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage131Callback != NULL) {
        g_pfnMediusMessage131Callback(connection, type, sender, response);
    }
    return kMessage131Size;
}

int rtMediusOnMessage128(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage128Size;
}

int rtMediusOnMessage129(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage129Callback != NULL) {
        g_pfnMediusMessage129Callback(connection, type, sender, response);
    }
    return kMessage129Size;
}

int rtMediusOnMessage126(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage126Size;
}

int rtMediusOnMessage127(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage127Callback != NULL) {
        g_pfnMediusMessage127Callback(connection, type, sender, response);
    }
    return kMessage127Size;
}

int rtMediusOnMessage120(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage120Size;
}

int rtMediusOnMessage121(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage121Callback != NULL) {
        g_pfnMediusMessage121Callback(connection, type, sender, response);
    }
    return kMessage121Size;
}

int rtMediusOnMessage132(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage132Size;
}

int rtMediusOnMessage133(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage133Callback != NULL) {
        g_pfnMediusMessage133Callback(connection, type, sender, response);
    }
    return kMessage133Size;
}

int rtMediusOnMessage027(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage027Size;
}

int rtMediusOnMessage028(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage028Callback != NULL) {
        g_pfnMediusMessage028Callback(connection, type, sender, response);
    }
    return kMessage028Size;
}

int rtMediusOnMessage134(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage134Size;
}

int rtMediusOnVersionServerResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusVersionResponse *response = (const rtMediusVersionResponse *)message;

    if (g_pfnMediusVersionServerCallback != NULL) {
        g_pfnMediusVersionServerCallback(connection, type, sender, response);
    }
    return kMessage135Size;
}

int rtMediusOnMessage136(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage136Size;
}

int rtMediusOnMessage137(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage137Callback != NULL) {
        g_pfnMediusMessage137Callback(connection, type, sender, response);
    }
    return kMessage137Size;
}

int rtMediusOnMessage138(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage138Size;
}

int rtMediusOnBanPlayerResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusBanPlayerCallback != NULL) {
        g_pfnMediusBanPlayerCallback(connection, type, sender, response);
    }
    return kMessage139Size;
}

int rtMediusOnMessage019(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage019Size;
}

int rtMediusOnAccountDeleteResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusAccountDeleteCallback != NULL) {
        g_pfnMediusAccountDeleteCallback(connection, type, sender, response);
    }
    return kMessage020Size;
}

int rtMediusOnMessage023(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage023Size;
}

int rtMediusOnAccountGetIdResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusAccountGetIdResponse *response = (const rtMediusAccountGetIdResponse *)message;

    if (g_pfnMediusAccountGetIdCallback != NULL) {
        g_pfnMediusAccountGetIdCallback(connection, type, sender, response);
    }
    return kMessage024Size;
}

int rtMediusOnMessage140(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage140Size;
}

int rtMediusOnMessage141(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage141Callback != NULL) {
        g_pfnMediusMessage141Callback(connection, type, sender, response);
    }
    return kMessage141Size;
}

int rtMediusOnMessage142(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage142Size;
}

int rtMediusOnMessage143(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage143Callback != NULL) {
        g_pfnMediusMessage143Callback(connection, type, sender, response);
    }
    return kMessage143Size;
}

int rtMediusOnMessage144(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage144Size;
}

int rtMediusOnMessage145(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage145Callback != NULL) {
        g_pfnMediusMessage145Callback(connection, type, sender, response);
    }
    return kMessage145Size;
}

int rtMediusOnMessage146(int connection, int type, int sender, rtDmeMessage *message) {
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

int rtMediusOnMessage204(int connection, int type, int sender, rtDmeMessage *message) {
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

int rtMediusOnMessage148(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage148Size;
}

int rtMediusOnLadderListResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusLadderListResponse *response = (const rtMediusLadderListResponse *)message;

    if (g_pfnMediusLadderListCallback != NULL) {
        g_pfnMediusLadderListCallback(connection, type, sender, response);
    }
    return kMessage149Size;
}

int rtMediusOnMessage150(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage150Size;
}

int rtMediusOnMessage151(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage151Callback != NULL) {
        g_pfnMediusMessage151Callback(connection, type, sender, response);
    }
    return kMessage151Size;
}

int rtMediusOnMessage152(int connection, int type, int sender, rtDmeMessage *message) {
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

int rtMediusOnMessage155(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage155Size;
}

int rtMediusOnMessage156(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage156Callback != NULL) {
        g_pfnMediusMessage156Callback(connection, type, sender, response);
    }
    return kMessage156Size;
}

int rtMediusOnMessage157(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage157Size;
}

int rtMediusOnMessage158(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage158Callback != NULL) {
        g_pfnMediusMessage158Callback(connection, type, sender, response);
    }
    return kMessage158Size;
}

int rtMediusOnMessage159(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage159Size;
}

int rtMediusOnMessage160(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage160Callback != NULL) {
        g_pfnMediusMessage160Callback(connection, type, sender, response);
    }
    return kMessage160Size;
}

int rtMediusOnMessage161(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage161Size;
}

int rtMediusOnMessage162(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage162Callback != NULL) {
        g_pfnMediusMessage162Callback(connection, type, sender, response);
    }
    return kMessage162Size;
}

int rtMediusOnMessage163(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage163Size;
}

int rtMediusOnMessage164(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage164Callback != NULL) {
        g_pfnMediusMessage164Callback(connection, type, sender, response);
    }
    return kMessage164Size;
}

int rtMediusOnMessage198(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage198Size;
}

int rtMediusOnMessage199(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage199Callback != NULL) {
        g_pfnMediusMessage199Callback(connection, type, sender, response);
    }
    return kMessage199Size;
}

int rtMediusOnMessage165(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage165Size;
}

int rtMediusOnFileCreateResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusFileResponse *response = (const rtMediusFileResponse *)message;

    if (g_pfnMediusFileCreateCallback != NULL) {
        g_pfnMediusFileCreateCallback(connection, type, sender, response);
    }
    return kMessage166Size;
}

int rtMediusOnMessage167(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage167Size;
}

int rtMediusOnMessage168(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage168Size;
}

int rtMediusOnMessage170(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage170Size;
}

int rtMediusOnFileCloseResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusFileResponse *response = (const rtMediusFileResponse *)message;

    if (g_pfnMediusFileCloseCallback != NULL) {
        g_pfnMediusFileCloseCallback(connection, type, sender, response);
    }
    return kMessage171Size;
}

int rtMediusOnMessage172(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage172Size;
}

int rtMediusOnFileDownloadResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusFileTransferResponse *response = (const rtMediusFileTransferResponse *)message;

    if (g_pfnMediusFileDownloadCallback != NULL) {
        g_pfnMediusFileDownloadCallback(connection, type, sender, response);
    }
    return kMessage173Size;
}

int rtMediusOnMessage174(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage174Size;
}

int rtMediusOnMessage175(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage175Callback != NULL) {
        g_pfnMediusMessage175Callback(connection, type, sender, response);
    }
    return kMessage175Size;
}

int rtMediusOnMessage176(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage176Size;
}

int rtMediusOnFileDeleteResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusFileDeleteResponse *response = (const rtMediusFileDeleteResponse *)message;

    if (g_pfnMediusFileDeleteCallback != NULL) {
        g_pfnMediusFileDeleteCallback(connection, type, sender, response);
    }
    return kMessage177Size;
}

int rtMediusOnMessage178(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage178Size;
}

int rtMediusOnFileListResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusFileResponse *response = (const rtMediusFileResponse *)message;

    if (g_pfnMediusFileListCallback != NULL) {
        g_pfnMediusFileListCallback(connection, type, sender, response);
    }
    return kMessage179Size;
}

int rtMediusOnMessage180(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage180Size;
}

int rtMediusOnMessage181(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage181Callback != NULL) {
        g_pfnMediusMessage181Callback(connection, type, sender, response);
    }
    return kMessage181Size;
}

int rtMediusOnMessage182(int connection, int type, int sender, rtDmeMessage *message) {
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

int rtMediusOnMessage184(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage184Size;
}

int rtMediusOnMessage185(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage185Callback != NULL) {
        g_pfnMediusMessage185Callback(connection, type, sender, response);
    }
    return kMessage185Size;
}

int rtMediusOnMessage186(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage186Size;
}

int rtMediusOnMessage187(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage187Callback != NULL) {
        g_pfnMediusMessage187Callback(connection, type, sender, response);
    }
    return kMessage187Size;
}

int rtMediusOnMessage188(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage188Size;
}

int rtMediusOnMessage189(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage189Callback != NULL) {
        g_pfnMediusMessage189Callback(connection, type, sender, response);
    }
    return kMessage189Size;
}

int rtMediusOnMessage190(int connection, int type, int sender, rtDmeMessage *message) {
    return kMessage190Size;
}

int rtMediusOnMessage191(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage191Callback != NULL) {
        g_pfnMediusMessage191Callback(connection, type, sender, response);
    }
    return kMessage191Size;
}

int rtMediusOnMessage200(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage200Callback != NULL) {
        g_pfnMediusMessage200Callback(connection, type, sender, response);
    }
    return kMessage200Size;
}

int rtMediusOnMessage201(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage201Callback != NULL) {
        g_pfnMediusMessage201Callback(connection, type, sender, response);
    }
    return kMessage201Size;
}

int rtMediusOnMessage202(int connection, int type, int sender, rtDmeMessage *message) {
    const rtMediusResponse *response = (const rtMediusResponse *)message;

    if (g_pfnMediusMessage202Callback != NULL) {
        g_pfnMediusMessage202Callback(connection, type, sender, response);
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
