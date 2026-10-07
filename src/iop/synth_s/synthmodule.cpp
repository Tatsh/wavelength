#include "synth_s/synthmodule.h"

#include <kernel.h>
#include <sifrpc.h>
#include <sysclib.h>

#include "synth_s/synthlock.h"
#include "synth_s/synthserver.h"

namespace {

constexpr unsigned short kModuleVersion = 0x0110;
constexpr int kServerThreadStackSize = 2048;
constexpr int kServerThreadPriority = 11;

const char kUnloadArgument[] = "other";

} // namespace

// NTSC-U/C: 0x000069f0
ModuleInfo Module = {"synth", kModuleVersion};

// NTSC-U/C: 0x000069f8
int SynthModule::sServerThreadId = -1;

int SynthModule::Load(int, char **) {
    EnableIntr(INUM_DMA_4);
    EnableIntr(INUM_DMA_7);
    sceSifInitRpc(0);
    SynthLock::Create();
    ThreadParam param;
    param.attr = TH_C;
    param.entry = SynthServer::Thread;
    param.initPriority = kServerThreadPriority;
    param.option = 0;
    param.stackSize = kServerThreadStackSize;
    sServerThreadId = CreateThread(&param);
    if (sServerThreadId > 0) {
        StartThread(sServerThreadId, 0);
        return REMOVABLE_RESIDENT_END;
    }
    return NO_RESIDENT_END;
}

int SynthModule::Unload(int, char **argv) {
    if (strcmp(argv[0], kUnloadArgument) != 0) {
        return NO_RESIDENT_END;
    }
    if (TerminateThread(sServerThreadId) != KE_OK) {
        return REMOVABLE_RESIDENT_END;
    }
    if (DeleteThread(sServerThreadId) == KE_OK) {
        return NO_RESIDENT_END;
    }
    return REMOVABLE_RESIDENT_END;
}

extern "C" int start(int argc, char **argv) {
    // Yes, retail runs neither the global constructors nor the global destructors.
    if (argc < 0) {
        return SynthModule::Unload(-argc, argv);
    }
    return SynthModule::Load(argc, argv);
}
