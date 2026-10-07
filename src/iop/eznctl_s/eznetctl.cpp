#include "eznctl_s/eznetctl.h"

#include <stdio.h>

#include <inet.h>
#include <kernel.h>
#include <sysclib.h>

#include "eznctl_s/netctlrequest.h"
#include "eznctl_s/netctlstatus.h"

namespace {

constexpr unsigned short kModuleVersion = 0x0109;
constexpr char kUnloadToken[] = "other";
constexpr int kRpcThreadPriority = 121;
constexpr int kRpcThreadStackSize = 0x800;

constexpr unsigned int kConnectTimeoutMicroseconds = 2000000;
constexpr int kLookUpTimeout = 5000;
constexpr int kLookUpRetries = 1;
// The resolved address is written over the name in the request.
constexpr int kAddressTextSize = 32;

constexpr char kMessagePrefix[] = "%s> ";
constexpr char kCreateEventFlagErrorMessage[] = "CreateEventFlag failed with %d\n";
constexpr char kLookUpMessage[] = "DNSLookup(%s, %d)\n";
constexpr char kName2AddressErrorMessage[] = "sceInetName2Address: error %d\n";
constexpr char kAddress2StringErrorMessage[] = "sceInetAddress2String: error %d\n";
constexpr char kResultMessage[] = "...result =%s\n";
constexpr char kNetCheckMessage[] = "net check...\n";
constexpr char kUnknownFunctionMessage[] = "unrecognized function id %d\n";

} // namespace

// NTSC-U/C: 0x00002140
ModuleInfo Module = {"eznetctl", kModuleVersion};

// NTSC-U/C: 0x00002130
int EzNetCtl::sThread = KE_ERROR;

#ifdef VIDEO_STANDARD_PAL
// PAL: 0x0000215c
int EzNetCtl::sNetChecked = 0;
#endif

// NTSC-U/C: 0x0000214c
int EzNetCtl::sEventFlag = 0;

// NTSC-U/C: 0x00002150
int EzNetCtl::sEthernetId = 0;

// NTSC-U/C: 0x00002154
int EzNetCtl::sPppId = 0;

// NTSC-U/C: 0x00002158
sceInetCtlEventHandler EzNetCtl::sEventHandler = {};

// NTSC-U/C: 0x00002190
sceSifQueueData EzNetCtl::sQueue;

// NTSC-U/C: 0x000021a8
sceSifServeData EzNetCtl::sServer;

// NTSC-U/C: 0x000021f0
alignas(16) unsigned char EzNetCtl::sBuffer[kRpcBufferSize];

// NTSC-U/C: 0x00002330
NetcnfifEnv EzNetCtl::sEnv;

// NTSC-U/C: 0x00002ba0
alignas(16) NetcnfifData EzNetCtl::sData;

int EzNetCtl::ModuleStart() {
    ThreadParam param;
    param.attr = TH_C;
    param.option = 0;
    param.entry = RpcServerThread;
    param.stackSize = kRpcThreadStackSize;
    param.initPriority = kRpcThreadPriority;
    sceSifInitRpc(0);
    sThread = CreateThread(&param);
    if (sThread <= 0) {
        return NO_RESIDENT_END;
    }
    if (StartThread(sThread, nullptr) != KE_OK) {
        return NO_RESIDENT_END;
    }
    if (Initialize() < 0) {
        return NO_RESIDENT_END;
    }
    return REMOVABLE_RESIDENT_END;
}

int EzNetCtl::ModuleStop([[maybe_unused]] int argc, char **argv) {
    RemoveRpcServer();
    if (strcmp(argv[0], kUnloadToken) != 0) {
        return Finalize() < 0 ? REMOVABLE_RESIDENT_END : NO_RESIDENT_END;
    }
    if (Finalize() < 0) {
        return REMOVABLE_RESIDENT_END;
    }
    if (TerminateThread(sThread) != KE_OK) {
        return REMOVABLE_RESIDENT_END;
    }
    if (DeleteThread(sThread) == KE_OK) {
        return NO_RESIDENT_END;
    }
    return REMOVABLE_RESIDENT_END;
}

void EzNetCtl::RpcServerThread() {
    sceSifInitRpc(0);
    sceSifSetRpcQueue(&sQueue, GetThreadId());
    sceSifRegisterRpc(&sServer, kRpcServerId, RpcHandler, sBuffer, nullptr, nullptr, &sQueue);
    sceSifRpcLoop(&sQueue);
}

void EzNetCtl::RemoveRpcServer() {
    sceSifRemoveRpc(&sServer, &sQueue);
    sceSifRemoveRpcQueue(&sQueue);
}

int EzNetCtl::Initialize() {
    EventFlagParam param;
    memset(&param, 0, sizeof(param));
    sEventHandler.handler = EventHandler;
    sceInetCtlRegisterEventHandler(&sEventHandler);
    sEventFlag = CreateEventFlag(&param);
    if (sEventFlag > 0) {
        return 0;
    }
    printf(kMessagePrefix, __FILE__);
    printf(kCreateEventFlagErrorMessage, sEventFlag);
    sEventFlag = 0;
    return -1;
}

int EzNetCtl::Finalize() {
    int result = 0;
    if (sEventHandler.handler != nullptr) {
        sceInetCtlUnregisterEventHandler(&sEventHandler);
        sEventHandler.handler = nullptr;
    }
    if (sEventFlag > 0) {
        result = DeleteEventFlag(sEventFlag);
        sEventFlag = 0;
    }
    return result;
}

unsigned int EzNetCtl::AlarmHandler(void *common) {
    iReleaseWaitThread(*static_cast<int *>(common));
    return 0;
}

void EzNetCtl::EventHandler(int interfaceId, int event) {
    if (event != INETCTL_EVENT_START) {
        return;
    }
    unsigned int flags;
    sceInetInterfaceControl(interfaceId, INET_CONTROL_GET_FLAGS, &flags, sizeof(flags));
    if ((flags & INET_INTERFACE_PPP) != 0) {
        sPppId = interfaceId;
    } else {
        sEthernetId = interfaceId;
    }
    if ((flags & INET_INTERFACE_PASSIVE) == 0) {
        SetEventFlag(sEventFlag,
                     (static_cast<unsigned int>(interfaceId) << kInterfaceIdShift) |
                         kInterfaceStartedBit);
    }
}

int EzNetCtl::Connect(NetcnfifEnv *env) {
    env->MergeDialNumbers();
    sPppId = 0;
    sEthernetId = 0;
    const int result = inetctl_4(env);
    if (result != 0) {
        return result;
    }
    SysClock timeout;
    USec2SysClock(kConnectTimeoutMicroseconds, &timeout);
    SetAlarm(&timeout, AlarmHandler, &sThread);
    unsigned int bits;
    if (WaitEventFlag(sEventFlag, kInterfaceStartedBit, WEF_CLEAR, &bits) == KE_RELEASE_WAIT) {
        return -1;
    }
    CancelAlarm(AlarmHandler, &sThread);
    return static_cast<unsigned short>(bits >> kInterfaceIdShift);
}

char *EzNetCtl::LookUpName(char *name, int size) {
    printf(kMessagePrefix, __FILE__);
    printf(kLookUpMessage, name, size);
    sceInetAddress address;
    // The binary leaves the sixth argument uninitialised.
    int result = sceInetName2Address(0, &address, name, kLookUpTimeout, kLookUpRetries, 0);
    if (result != 0) {
        printf(kMessagePrefix, __FILE__);
        printf(kName2AddressErrorMessage, result);
        name[0] = '\0';
    } else {
        result = sceInetAddress2String(name, kAddressTextSize, &address);
        if (result != 0) {
            printf(kMessagePrefix, __FILE__);
            printf(kAddress2StringErrorMessage, result);
            name[0] = '\0';
        }
    }
    printf(kMessagePrefix, __FILE__);
    printf(kResultMessage, name);
    return name;
}

void *EzNetCtl::RpcHandler(unsigned int function, void *buffer, int size) {
    auto *request = static_cast<NetCtlRequest *>(buffer);
    int result = KE_ERROR;
    switch (function) {
    case kFunctionGetDataAddress:
        request->mData = &sData;
        return buffer;
    case kFunctionApplyData:
        sEnv.Init();
        result = sEnv.WriteEnv(&sData, SCE_NETCNF_TYPE_NET);
        if (result >= 0) {
            result = Connect(&sEnv);
        }
        break;
    case kFunctionLoadEntry:
        result = sEnv.LoadEntry(request->mFileName, request->mUserName);
        if (result >= 0) {
            result = Connect(&sEnv);
        }
        break;
    case kFunctionGetStatus:
        static_cast<NetCtlStatus *>(buffer)->Update(); // The binary discards the result.
        return buffer;
    case kFunctionInetCtl7:
        result = inetctl_7(request->mInterfaceId);
        break;
    case kFunctionInetCtl5:
        result = inetctl_5(request->mInterfaceId);
        break;
    case kFunctionInetCtl6:
        result = inetctl_6(request->mInterfaceId);
        break;
    case kFunctionLookUpName:
        LookUpName(static_cast<char *>(buffer), size);
        return buffer;
    case kFunctionNetCheck:
        printf(kMessagePrefix, __FILE__);
        printf(kNetCheckMessage);
        break;
#ifdef VIDEO_STANDARD_PAL
    case kFunctionSetNetChecked:
        sNetChecked = 1;
        break;
#endif
    default:
        printf(kMessagePrefix, __FILE__);
        printf(kUnknownFunctionMessage, function);
        break;
    }
    request->mResult = result;
    return buffer;
}

int start(int argc, char **argv) {
    if (argc < 0) {
        return EzNetCtl::ModuleStop(-argc, argv);
    }
    return EzNetCtl::ModuleStart();
}
