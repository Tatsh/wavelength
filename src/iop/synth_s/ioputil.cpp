#include "synth_s/ioputil.h"

#include <kernel.h>
#include <libsd.h>

namespace {

constexpr int kThreadStackSize = 2048;
// AllocSysMemory() placement of the lowest free area that fits.
constexpr int kAllocFirst = 0;

} // namespace

void *IopAlloc(int size) {
    return AllocSysMemory(kAllocFirst, size, nullptr);
}

void IopFree(void *block) {
    FreeSysMemory(block);
}

int CreateSynthThread(void (*entry)(), int priority) {
    ThreadParam param;
    param.attr = TH_C;
    param.entry = entry;
    param.initPriority = priority;
    param.stackSize = kThreadStackSize;
    param.option = 0;
    return CreateThread(&param);
}

void SpuSetParam(unsigned short entry, unsigned short value) {
    sceSdSetParam(entry, value);
}
