#include "ezncnf_s/eznetcnf.h"

#include <stdio.h>

#include <kernel.h>
#include <sif.h>
#include <sysclib.h>

namespace {

constexpr unsigned short kModuleVersion = 0x0109;
constexpr char kUnloadToken[] = "other";
constexpr int kRpcThreadPriority = 121;
constexpr int kRpcThreadStackSize = 0x800;
// SIF DMA moves whole quadwords.
constexpr int kDmaAlignment = 16;

constexpr char kMessagePrefix[] = "%s> ";
constexpr char kDmaPaddingMessage[] = "warning: SIF DMA transfer size %d padded to %d\n";
constexpr char kUnknownFunctionMessage[] = "unrecognized function id %d\n";

} // namespace

// NTSC-U/C: 0x00001570
ModuleInfo Module = {"eznetcnf", kModuleVersion};

// NTSC-U/C: 0x00001560
int EzNetCnf::sThread = KE_ERROR;

// NTSC-U/C: 0x00001580
int EzNetCnf::sInterruptState;

// NTSC-U/C: 0x00001730
NetcnfifEnv EzNetCnf::sEnv;

// NTSC-U/C: 0x00001590
sceSifQueueData EzNetCnf::sQueue;

// NTSC-U/C: 0x000015a8
sceSifServeData EzNetCnf::sServer;

// NTSC-U/C: 0x000015f0
alignas(16) NetCnfRequest EzNetCnf::sBuffer;

int EzNetCnf::ModuleStart() {
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

int EzNetCnf::ModuleStop([[maybe_unused]] int argc, char **argv) {
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

void EzNetCnf::RpcServerThread() {
    sceSifInitRpc(0);
    sceSifSetRpcQueue(&sQueue, GetThreadId());
    sceSifRegisterRpc(&sServer, kRpcServerId, RpcHandler, &sBuffer, nullptr, nullptr, &sQueue);
    sceSifRpcLoop(&sQueue);
}

void EzNetCnf::RemoveRpcServer() {
    sceSifRemoveRpc(&sServer, &sQueue);
    sceSifRemoveRpcQueue(&sQueue);
}

int EzNetCnf::Initialize() {
    return 0;
}

int EzNetCnf::Finalize() {
    return 0;
}

void *EzNetCnf::RpcHandler(unsigned int function, void *buffer, [[maybe_unused]] int size) {
    auto *request = static_cast<NetCnfRequest *>(buffer);
    int result = KE_ERROR;
    switch (function) {
    case kFunctionGetCount:
        result = sceNetCnfGetCount(request->mFileName, request->mType);
        break;
    case kFunctionGetList:
        result = request->GetList();
        break;
    case kFunctionLoadEntry:
        result = request->LoadEntry();
        break;
    default:
        printf(kMessagePrefix, __FILE__);
        printf(kUnknownFunctionMessage, function);
        break;
    }
    request->mResult = result;
    return request;
}

unsigned int EzNetCnf::SendToEe(void *data, void *destination, int size, bool noWait) {
    sceSifDmaData transfer;
    transfer.data = data;
    transfer.addr = destination;
    transfer.size = (size + kDmaAlignment - 1) & -kDmaAlignment;
    transfer.mode = 0;
    if ((size & (kDmaAlignment - 1)) != 0) {
        printf(kMessagePrefix, __FILE__);
        printf(kDmaPaddingMessage, size, transfer.size);
    }
    unsigned int id;
    do {
        CpuSuspendIntr(&sInterruptState);
        id = sceSifSetDma(&transfer, 1);
        CpuResumeIntr(sInterruptState);
    } while (id == 0);
    if (!noWait) {
        while (sceSifDmaStat(id) >= 0) {
        }
    }
    return id;
}

int start(int argc, char **argv) {
    if (argc < 0) {
        return EzNetCnf::ModuleStop(-argc, argv);
    }
    return EzNetCnf::ModuleStart();
}
