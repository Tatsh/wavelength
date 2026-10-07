#include "SNProfil/snprofil.h"

#include <stdio.h>

#include <kernel.h>
#include <sifrpc.h>

namespace {

constexpr unsigned short kModuleVersion = 0x0101;
constexpr int kServerThreadPriority = 32;
constexpr int kServerThreadStackSize = 0x800;
constexpr int kRpcBufferSize = 0x200;

constexpr char kRpcInitializeMessage[] = "RPC Initialize\n";
constexpr char kSetRpcQueueMessage[] = "Set rpc queue\n";
constexpr char kRegisterFunctionMessage[] = "Register function\n";
constexpr char kEnteringRpcLoopMessage[] = "Entering sceSifRpcLoop\n";
constexpr char kRpcServerMessage[] = "rpc_server(%08x, %08X, %08x)\n";

// NTSC-U/C: 0x00000360
ProfileBuffer ProfBuffer;

// NTSC-U/C: 0x000083e0
unsigned char buffer[kRpcBufferSize];

// NTSC-U/C: 0x00000148
void *rpc_server(unsigned int function, void *data, int size) {
    printf(kRpcServerMessage, function, data, size);
    auto *info = static_cast<ProfileBufferInfo *>(data);
    info->buffer = &ProfBuffer;
    info->headerSize = kProfileHeaderSize;
    info->samples = info->buffer->samples;
    info->samplesSize = kProfileSamplesSize;
    return data;
}

// NTSC-U/C: 0x00000098
void test_th() {
    sceSifQueueData queue;
    sceSifServeData server;
    printf(kRpcInitializeMessage);
    sceSifInitRpc(0);
    printf(kSetRpcQueueMessage);
    sceSifSetRpcQueue(&queue, GetThreadId());
    printf(kRegisterFunctionMessage);
    sceSifRegisterRpc(&server, kSnProfilRpcServer, rpc_server, buffer, nullptr, nullptr, &queue);
    printf(kEnteringRpcLoopMessage);
    sceSifRpcLoop(&queue);
}

} // namespace

// NTSC-U/C: 0x00000350
ModuleInfo Module = {"SNProfil", kModuleVersion};

int start([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    CpuEnableIntr();
    // The binary also returns NO_RESIDENT_END for a null ProfBuffer address. That case never
    // occurs.
    ThreadParam param;
    param.attr = TH_C;
    param.entry = test_th;
    param.initPriority = kServerThreadPriority;
    param.stackSize = kServerThreadStackSize;
    param.option = 0;
    const int thread = CreateThread(&param);
    if (thread <= 0) {
        return NO_RESIDENT_END;
    }
    StartThread(thread, nullptr);
    return RESIDENT_END;
}
