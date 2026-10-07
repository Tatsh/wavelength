#include "ezncnf_s/netcnfifenv.h"

#include <stdint.h>

#include <sysclib.h>

#include "ezncnf_s/netcnflistreply.h"

namespace {

constexpr char kMemoryCardPrefix[] = "mc";
constexpr char kHardDiskPrefix[] = "pfs";
constexpr uintptr_t kMemoryAlignment = 4;
constexpr unsigned char kPppoeUnset = 0xff;

} // namespace

// NTSC-U/C: 0x00001fa0
alignas(16) unsigned char NetcnfifEnv::sMemory[kMemorySize];

int NetcnfifEnv::GetDeviceType() const {
    const sceNetCnfInterface *settings = nullptr;
    if (root != nullptr && root->head != nullptr) {
        settings = root->head->interface;
    }
    if (settings == nullptr) {
        return kNetCnfDeviceTypeNone;
    }
    switch (settings->type) {
    case SCE_NETCNF_INTERFACE_ETHERNET:
    case SCE_NETCNF_INTERFACE_NIC:
        return kNetCnfDeviceTypeEthernet;
    case SCE_NETCNF_INTERFACE_PPP:
        if (settings->pppoe == 0 || settings->pppoe == kPppoeUnset) {
            return kNetCnfDeviceTypeModem;
        }
        return kNetCnfDeviceTypePppoe;
    default:
        return kNetCnfDeviceTypeNone;
    }
}

int NetcnfifEnv::LoadEntry(const char *fileName, const char *userName) {
    Init();
    hostFile = SelectByDevice(fileName, 0, 0, 1);
    const int result = sceNetCnfLoadEntry(fileName, SCE_NETCNF_TYPE_NET, userName, this);
    if (result >= 0) {
        memoryPointer = reinterpret_cast<unsigned char *>(
            (reinterpret_cast<uintptr_t>(memoryPointer) + kMemoryAlignment - 1) &
            ~(kMemoryAlignment - 1));
        memoryBase = memoryPointer;
    }
    return result;
}

int NetcnfifEnv::SelectByDevice(const char *fileName, int memoryCard, int hardDisk, int other) {
    if (strncmp(fileName, kMemoryCardPrefix, sizeof(kMemoryCardPrefix) - 1) == 0) {
        return memoryCard;
    }
    if (strncmp(fileName, kHardDiskPrefix, sizeof(kHardDiskPrefix) - 1) == 0) {
        return hardDisk;
    }
    return other;
}

void NetcnfifEnv::Init() {
    bzero(static_cast<sceNetCnfEnv *>(this), sizeof(sceNetCnfEnv));
    memoryBase = sMemory;
    memoryPointer = sMemory;
    memoryEnd = sMemory + kMemorySize;
}
