#include "softfx_s/lock.h"

#include <kernel.h>

namespace {

constexpr int kLockCount = 1;

// NTSC-U/C: 0x00004d90
int g_nLockSema = KE_ERROR;

// NTSC-U/C: 0x00004d94
SemaParam g_lockSemaParam;

} // namespace

void CreateLock() {
    g_lockSemaParam.initCount = kLockCount;
    g_lockSemaParam.maxCount = kLockCount;
    g_lockSemaParam.option = 0;
    g_lockSemaParam.attr = 0;
    g_nLockSema = CreateSema(&g_lockSemaParam);
}

void DeleteLock() {
    DeleteSema(g_nLockSema);
}

void AcquireLock([[maybe_unused]] const char *owner) {
    WaitSema(g_nLockSema);
}

void ReleaseLock([[maybe_unused]] const char *owner) {
    SignalSema(g_nLockSema);
}
