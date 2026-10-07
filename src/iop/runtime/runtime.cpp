#include "runtime/runtime.h"

#include <stddef.h>

#include <kernel.h>

// The IOP has no C or C++ library. The module carries the allocation, termination, and static
// object routines its C++ code needs. The addresses below are those of the softFX module's copy.

extern "C" {

using Constructor = void (*)();
using Destructor = void (*)(void *);

// The linker script places these around the constructor list.
extern Constructor __ctors_start[];
extern Constructor __ctors_end[];

void *__dso_handle = nullptr;

void *malloc(size_t size);
void free(void *block);
[[noreturn]] void exit(int status);
int __cxa_atexit(Destructor destructor, void *object, void *dso);
[[noreturn]] void __cxa_pure_virtual();

} // extern "C"

namespace {

// The least number of exit routines the C standard requires an implementation to support.
constexpr int kExitHandlerLimit = 32;
constexpr int kFailureStatus = -1;

struct ExitHandler {
    Destructor mDestructor;
    void *mObject;
};

ExitHandler g_exitHandlers[kExitHandlerLimit];
int g_nExitHandlers = 0;

// NTSC-U/C: 0x00003968, PAL: 0x00003968
[[noreturn]] void DefaultNewHandler() {
    exit(kFailureStatus);
}

} // namespace

// NTSC-U/C: 0x00003410, PAL: 0x00003410
void *malloc(size_t size) {
    return AllocSysMemory(0, static_cast<int>(size), nullptr);
}

// NTSC-U/C: 0x00003438, PAL: 0x00003438
void free(void *block) {
    FreeSysMemory(block);
}

// NTSC-U/C: 0x00003458, PAL: 0x00003458
void exit([[maybe_unused]] int status) {
    for (;;) {
    }
}

int __cxa_atexit(Destructor destructor, void *object, [[maybe_unused]] void *dso) {
    if (g_nExitHandlers == kExitHandlerLimit) {
        return kFailureStatus;
    }
    g_exitHandlers[g_nExitHandlers++] = {destructor, object};
    return 0;
}

// NTSC-U/C: 0x000038c0, PAL: 0x000038c0
void __cxa_pure_virtual() {
    exit(kFailureStatus);
}

// NTSC-U/C: 0x000038d0, PAL: 0x000038d0
void *operator new(size_t size) {
    void *block = malloc(size == 0 ? 1 : size);
    if (block == nullptr) {
        DefaultNewHandler();
    }
    return block;
}

// NTSC-U/C: 0x000038a0, PAL: 0x000038a0
void *operator new[](size_t size) {
    return operator new(size);
}

// NTSC-U/C: 0x00003850, PAL: 0x00003850
void operator delete(void *block) noexcept {
    if (block != nullptr) {
        free(block);
    }
}

// NTSC-U/C: 0x00003880, PAL: 0x00003880
void operator delete[](void *block) noexcept {
    operator delete(block);
}

void RunGlobalConstructors() {
    for (auto *constructor = __ctors_end; constructor != __ctors_start;) {
        (*--constructor)();
    }
}

void RunGlobalDestructors() {
    while (g_nExitHandlers > 0) {
        const auto &handler = g_exitHandlers[--g_nExitHandlers];
        handler.mDestructor(handler.mObject);
    }
}
