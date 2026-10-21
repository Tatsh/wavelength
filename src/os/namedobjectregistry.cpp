#include "os/namedobjectregistry.h"

NamedObjectRegistry *NamedObjectRegistry::Get() {
    // The destruction registered with atexit() on first use is at
    // NTSC-U/C: 0x0029e6c8, PAL: 0x002a8390
    static NamedObjectRegistry sRegistry;
    return &sRegistry;
}

void NamedObjectRegistryInit() {
}

void NamedObjectRegistryTerminate() {
    NamedObjectRegistry::Get()->DeleteEntries();
}

NamedObjectRegistry::NamedObjectRegistry() {
}

NamedObjectRegistry::~NamedObjectRegistry() {
}

void NamedObjectRegistry::DeleteEntries() {
    mEntries.clear();
}
