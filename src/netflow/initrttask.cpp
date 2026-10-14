#include "netflow/initrttask.h"

#include <cstring>

#include <librtdme.h>

#include "msg/chatmsg.h"
#include "msg/lobbyconnectresultmsg.h"
#include "netflow/launchpadrt.h"
#include "netflow/lobbyrt.h"
#include "os/mem.h"

namespace {

// The alignment of the blocks of the library.
constexpr int kBlockAlignment = 16;

// The sizes of the send and receive queues of the datagram service in bytes.
constexpr int kSendBufferSize = 65536;
constexpr int kReceiveBufferSize = 32768;

// The wait for an answer of the lobby server, in milliseconds.
constexpr int kMediusTimeoutMs = 15000;

// The size of the buffer the version of the lobby client is copied into.
constexpr int kVersionBufferSize = 128;

} // namespace

// NTSC-U/C: 0x003b0af8
InitRTTask *InitRTTask::sTask;

void InitRTTask::OnStart() {
    int nError = rtNetSetMemoryFunctions(AllocBlock, FreeBlock);
    LobbyRT::ReportNetError(__LINE__, nError);
    if (nError == 0) {
        rtNetSettings settings;
        memset(&settings, 0, sizeof(settings));
        (void)rtNetDefaultSettings(&settings);
        settings.applicationId = LobbyRT::kApplicationId;
        if (TheLobbyRT->mSecure != 0) {
            settings.keyPair = &TheLobbyRT->mKeyPair;
        }
        rtNetInitStatus status;
        memset(&status, 0, sizeof(status));
        nError = rtNetInitialize(&settings, &status);
        LobbyRT::ReportNetError(__LINE__, nError);
        if (nError == 0) {
            nError = rtNetSetBufferSizes(kSendBufferSize, kReceiveBufferSize);
            LobbyRT::ReportNetError(__LINE__, nError); // Yes, the binary goes on after an error.
            if (LaunchpadRT::RegisterMessages() == 0) {
                rtMediusInitParams params;
                memset(&params, 0, sizeof(params));
                params.timeoutMs = kMediusTimeoutMs;
                params.onChat = ChatMsg::OnLobbyChat;
                params.onNotice = OnNotice;
                params.onMessage069 = nullptr;
                int nStatus;
                memset(&nStatus, 0, sizeof(nStatus));
                nError = rtMediusInitialize(&params, &nStatus);
                LobbyRT::ReportMediusError(__LINE__, nError);
                if (nError == 0) {
                    char szVersion[kVersionBufferSize];
                    szVersion[0] = '\0';
                    (void)rtMediusGetVersion(szVersion); // Yes, the binary discards the version.
                    TheLobbyRT->mRtReady = 1;
                    Finish(true, 0);
                    return;
                }
            }
        }
    }
    Finish(false, LobbyConnectResultMsg::kResultFailed);
}

void InitRTTask::OnStop() {
    sTask = nullptr;
}

void *InitRTTask::AllocBlock(int nSize) {
    return PoolMemAlloc(nSize, "rtime", kBlockAlignment);
}

void InitRTTask::FreeBlock(void *pBlock) {
    PoolMemFree(pBlock);
}

void InitRTTask::OnNotice(int nConnection, int nType, int nSize, const rtMediusResponse *pNotice) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    (void)pNotice;
}
