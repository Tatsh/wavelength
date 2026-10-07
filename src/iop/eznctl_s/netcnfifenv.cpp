#include "eznctl_s/netcnfifenv.h"

#include <stdint.h>
#include <stdio.h>

#include <sysclib.h>

namespace {

constexpr char kMemoryCardPrefix[] = "mc";
constexpr char kHardDiskPrefix[] = "pfs";
constexpr char kPairNameSeparator[] = " + ";
constexpr char kUnknownTypeMessage[] = "[%s] unknown type (%d)\n";

constexpr uintptr_t kMemoryAlignment = 4;
// Alignment selector of every sceNetCnfAllocMem() call.
constexpr int kAllocAlignment = 2;
constexpr int kErrorNoMemory = -2;
constexpr int kRootVersion = 3;
constexpr int kUnset = -1;
constexpr unsigned char kUnsetOption = 0xff;
constexpr unsigned char kDefaultPppOption6 = 4;
constexpr unsigned char kPppoeEnabled = 1;
// Only the first dial numbers are merged, as many as a NetcnfifData stores.
constexpr int kMergedDialNumberCount = NetcnfifData::kDialNumberCount;

} // namespace

// NTSC-U/C: 0x00003ee0
alignas(16) unsigned char NetcnfifEnv::sMemory[kMemorySize];

// NTSC-U/C: 0x000046e0
char NetcnfifEnv::sChatScript[NetcnfifData::kTextSize];

// NTSC-U/C: 0x000047e0
char NetcnfifEnv::sPairName[NetcnfifData::kTextSize];

// NTSC-U/C: 0x000048e0
sceNetCnfRouteCommand NetcnfifEnv::sRoute;

// NTSC-U/C: 0x00004940
sceNetCnfNameServerCommand NetcnfifEnv::sNameServers[kNameServerCount];

inline void NetcnfifEnv::AlignMemory() {
    memoryPointer = reinterpret_cast<unsigned char *>(
        (reinterpret_cast<uintptr_t>(memoryPointer) + kMemoryAlignment - 1) &
        ~(kMemoryAlignment - 1));
    memoryBase = memoryPointer;
}

void NetcnfifEnv::MergeDialNumbers() {
    sceNetCnfInterface *settings = root->head->interface;
    if (settings->type == SCE_NETCNF_INTERFACE_PPP && settings->pppoe != kPppoeEnabled) {
        char *merged[kMergedDialNumberCount];
        memset(merged, 0, sizeof(merged));
        int count = 0;
        for (int i = 0; i < kMergedDialNumberCount; ++i) {
            const sceNetCnfPair *pair = root->head;
            if (pair->interface->dialNumbers[i] == nullptr) {
                continue;
            }
            const sceNetCnfInterface *device = pair->device;
            // Either prefix being set reads both, even a null one.
            if (device->dialPrefix != nullptr || device->dialPrefixPause != nullptr) {
                const size_t length = strlen(device->dialPrefix) + strlen(device->dialPrefixPause) +
                                      strlen(pair->interface->dialNumbers[i]);
                merged[i] = static_cast<char *>(
                    sceNetCnfAllocMem(this, static_cast<int>(length + 1), kAllocAlignment));
                if (merged[i] != nullptr) {
                    strcpy(merged[i], device->dialPrefix);
                    strcat(merged[i], device->dialPrefixPause);
                    strcat(merged[i], pair->interface->dialNumbers[i]);
                }
            }
            ++count;
        }
        if (preserveDialNumbers == 0) {
            for (int i = 0; i < kMergedDialNumberCount; ++i) {
                if (merged[i] != nullptr) {
                    root->head->interface->dialNumbers[i] = merged[i];
                }
            }
            root->head->device->dialPrefix = nullptr;
            root->head->device->dialPrefixPause = nullptr;
        }
        root->head->interface->lastDialNumber = count - 1;
    }
    settings = root->head->interface;
    if (settings->type != SCE_NETCNF_INTERFACE_PPP && settings->pppoe != kPppoeEnabled) {
        settings->type = root->head->device->type;
    }
}

int NetcnfifEnv::LoadEntry(const char *fileName, const char *userName) {
    Init();
    hostFile = SelectByDevice(fileName, 0, 0, 1);
    const int result = sceNetCnfLoadEntry(fileName, SCE_NETCNF_TYPE_NET, userName, this);
    if (result >= 0) {
        AlignMemory();
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

void NetcnfifEnv::ResetInterface(sceNetCnfInterface *interface) {
    interface->type = kUnset;
    interface->dhcp = kUnsetOption;
    interface->address = nullptr;
    interface->netmask = nullptr;
    interface->broadcast = nullptr;
    interface->commandHead = nullptr;
    interface->commandTail = nullptr;
    for (int i = 0; i < kMergedDialNumberCount; ++i) {
        interface->dialNumbers[i] = nullptr;
    }
    interface->authName = nullptr;
    interface->authKey = nullptr;
    interface->peerName = nullptr;
    interface->pppOption4 = kUnsetOption;
    interface->pppOption5 = kUnsetOption;
    interface->pppOption7 = 0;
    interface->pppOption6 = kDefaultPppOption6;
    interface->pppoe = kUnsetOption;
    interface->pppOption2 = kUnsetOption;
    interface->pppOption3 = kUnsetOption;
    interface->pppOption1 = kUnsetOption;
    interface->interfaceMode = kUnset;
    interface->deviceConfig = kUnset;
    interface->vendor = nullptr;
    interface->product = nullptr;
    interface->chatScript = nullptr;
    interface->dialPrefix = nullptr;
    interface->dialPrefixPause = nullptr;
    interface->deviceMode = kUnset;
    interface->timeout = kUnset;
}

bool NetcnfifEnv::IsNonzeroAddress(const char *text) {
    bool nonzero = false;
    for (const char *c = text; *c != '\0'; ++c) {
        if (*c != '.' && *c != '0') {
            nonzero = true;
        }
    }
    return nonzero;
}

inline void NetcnfifEnv::AppendCommand(sceNetCnfCommand *command) {
    command->previous = interface->commandTail;
    if (interface->commandTail != nullptr) {
        interface->commandTail->next = command;
    } else {
        interface->commandHead = command;
    }
    command->next = nullptr;
    interface->commandTail = command;
}

int NetcnfifEnv::AddRoute(const char *gateway) {
    bzero(&sRoute, sizeof(sRoute));
    sRoute.header.code = SCE_NETCNF_COMMAND_ROUTE;
    AppendCommand(&sRoute.header);
    int result = sceNetCnfName2Address(&sRoute.destination, nullptr);
    if (result < 0) {
        return result;
    }
    result = sceNetCnfName2Address(&sRoute.gateway, gateway);
    if (result < 0) {
        return result;
    }
    result = sceNetCnfName2Address(&sRoute.genmask, nullptr);
    if (result < 0) {
        return result;
    }
    if (gateway == nullptr) {
        sRoute.flags = 0;
    } else {
        sRoute.flags |= SCE_NETCNF_ROUTE_GATEWAY;
    }
    return result;
}

int NetcnfifEnv::AddNameServer(const char *address, int index) {
    sceNetCnfNameServerCommand *command = nullptr;
    if (index == 1) {
        command = &sNameServers[0];
        bzero(command, sizeof(*command));
    } else if (index == 2) {
        command = &sNameServers[1];
        bzero(command, sizeof(*command));
    }
    command->header.code = SCE_NETCNF_COMMAND_NAME_SERVER; // Any other index writes through null.
    AppendCommand(&command->header);
    return sceNetCnfName2Address(&command->address, address);
}

int NetcnfifEnv::WriteAddresses(NetcnfifData *data) {
    int result = 0;
    if (data->mDhcp == 0) {
        if (data->mGateway[0] != '\0' && IsNonzeroAddress(data->mGateway)) {
            result = AddRoute(data->mGateway);
        } else {
            result = AddRoute(nullptr);
        }
    }
    if (data->mNameServer1[0] != '\0' && IsNonzeroAddress(data->mNameServer1)) {
        result = AddNameServer(data->mNameServer1, 1);
        if (data->mNameServer2[0] != '\0' && IsNonzeroAddress(data->mNameServer2)) {
            result = AddNameServer(data->mNameServer2, 2);
        }
    }
    return result;
}

inline void NetcnfifEnv::AppendPair(sceNetCnfPair *pair) {
    pair->previous = root->tail;
    if (root->tail != nullptr) {
        root->tail->next = pair;
    } else {
        root->head = pair;
    }
    pair->next = nullptr;
    root->tail = pair;
}

inline void NetcnfifEnv::ResetRoot() {
    root->version = kRootVersion;
    root->setting1 = kUnset;
    root->setting2 = kUnset;
    root->setting3 = kUnset;
}

int NetcnfifEnv::Attach(int kind) {
    if (interface == nullptr) {
        return 0;
    }
    if (root != nullptr) {
        sceNetCnfPair *pair = root->head;
        if (pair != nullptr) {
            if (kind == SCE_NETCNF_TYPE_INTERFACE) {
                pair->interface = interface;
            }
            if (kind == SCE_NETCNF_TYPE_DEVICE) {
                root->head->device = interface;
            }
            return 0;
        }
    } else {
        root = static_cast<sceNetCnfRoot *>(
            sceNetCnfAllocMem(this, sizeof(sceNetCnfRoot), kAllocAlignment));
        if (root == nullptr) {
            return kErrorNoMemory;
        }
        ResetRoot();
    }
    auto *pair = static_cast<sceNetCnfPair *>(
        sceNetCnfAllocMem(this, sizeof(sceNetCnfPair), kAllocAlignment));
    if (pair == nullptr) {
        return kErrorNoMemory;
    }
    if (kind == SCE_NETCNF_TYPE_INTERFACE) {
        pair->interface = interface;
    }
    if (kind == SCE_NETCNF_TYPE_DEVICE) {
        pair->device = interface;
    }
    AppendPair(pair);
    return 0;
}

int NetcnfifEnv::WriteInterface(NetcnfifData *data, int kind) {
    if (interface == nullptr) {
        interface = static_cast<sceNetCnfInterface *>(
            sceNetCnfAllocMem(this, sizeof(sceNetCnfInterface), kAllocAlignment));
        if (interface == nullptr) {
            return kErrorNoMemory;
        }
        sceNetCnfInitInterface(interface);
    }
    ResetInterface(interface);
    int result = 0;
    bool empty = true;
    int timeout;
    switch (kind) {
    case SCE_NETCNF_TYPE_INTERFACE:
        if (data->mInterfaceType != kUnset) {
            interface->type = data->mInterfaceType;
            empty = false;
        }
        if (data->mDhcp != kUnsetOption) {
            interface->dhcp = data->mDhcp;
            empty = false;
        }
        if (data->mAddress[0] != '\0') {
            interface->address = data->mAddress;
            empty = false;
        }
        if (data->mNetmask[0] != '\0') {
            interface->netmask = data->mNetmask;
            empty = false;
        }
        if (data->mBroadcast[0] != '\0') {
            interface->broadcast = data->mBroadcast;
            empty = false;
        }
        result = WriteAddresses(data);
        if (result < 0) {
            return result;
        }
        if (result != 0) {
            empty = false;
        }
        for (int i = 0; i < NetcnfifData::kDialNumberCount; ++i) {
            if (data->mDialNumbers[i][0] != '\0') {
                interface->dialNumbers[i] = data->mDialNumbers[i];
                empty = false;
            }
        }
        if (data->mAuthName[0] != '\0') {
            interface->authName = data->mAuthName;
            empty = false;
        }
        if (data->mAuthKey[0] != '\0') {
            interface->authKey = data->mAuthKey;
            empty = false;
        }
        if (data->mPeerName[0] != '\0') {
            interface->peerName = data->mPeerName;
            empty = false;
        }
        if (data->mPppOption4 != kUnsetOption) {
            interface->pppOption4 = data->mPppOption4;
            empty = false;
        }
        if (data->mPppOption5 != kUnsetOption) {
            interface->pppOption5 = data->mPppOption5;
            empty = false;
        }
        if (data->mPppOption7 != 0) {
            interface->pppOption7 = data->mPppOption7;
            empty = false;
        }
        interface->pppOption6 = data->mPppOption6; // Copied without marking the record set.
        if (data->mPppoe != kUnsetOption) {
            interface->pppoe = data->mPppoe;
            empty = false;
        }
        if (data->mPppOption2 != kUnsetOption) {
            interface->pppOption2 = data->mPppOption2;
            empty = false;
        }
        if (data->mPppOption3 != kUnsetOption) {
            interface->pppOption3 = data->mPppOption3;
            empty = false;
        }
        if (data->mPppOption1 != kUnsetOption) {
            interface->pppOption1 = data->mPppOption1;
            empty = false;
        }
        if (data->mInterfaceMode != kUnset) {
            interface->interfaceMode = data->mInterfaceMode;
            empty = false;
        }
        timeout = data->mInterfaceTimeout;
        break;
    case SCE_NETCNF_TYPE_DEVICE:
        if (data->mDeviceType != kUnset) {
            interface->type = data->mDeviceType;
            empty = false;
        }
        if (data->mVendor[0] != '\0') {
            interface->vendor = data->mVendor;
            empty = false;
        }
        if (data->mProduct[0] != '\0') {
            interface->product = data->mProduct;
            empty = false;
        }
        if (data->mDeviceConfig != kUnset) {
            interface->deviceConfig = data->mDeviceConfig;
            empty = false;
        }
        if (data->mChatScript[0] != '\0') {
            result = netcnf_20(data->mChatScript, sChatScript, NetcnfifData::kTextSize);
            if (result < 0) {
                return result;
            }
            interface->chatScript = sChatScript;
            empty = false;
        }
        if (data->mDialPrefix[0] != '\0') {
            interface->dialPrefix = data->mDialPrefix;
            empty = false;
        }
        if (data->mDialPrefixPause[0] != '\0') {
            interface->dialPrefixPause = data->mDialPrefixPause;
            empty = false;
        }
        if (data->mDeviceMode != kUnset) {
            interface->deviceMode = data->mDeviceMode;
            empty = false;
        }
        timeout = data->mDeviceTimeout;
        break;
    default:
        timeout = kUnset;
        break;
    }
    if (timeout != kUnset) {
        interface->timeout = timeout;
        empty = false;
    }
    if (empty) {
        interface = nullptr;
        return kErrorNoSettings;
    }
    return result;
}

int NetcnfifEnv::WritePairs(NetcnfifData *data) {
    if (data->mInterfaceName[0] == '\0' || data->mDeviceName[0] == '\0') {
        return kErrorNoSettings;
    }
    if (root == nullptr) {
        root = static_cast<sceNetCnfRoot *>(
            sceNetCnfAllocMem(this, sizeof(sceNetCnfRoot), kAllocAlignment));
        if (root == nullptr) {
            return kErrorNoMemory;
        }
    }
    ResetRoot();
    sceNetCnfPair *pair;
    if (root->head == nullptr) {
        pair = static_cast<sceNetCnfPair *>(
            sceNetCnfAllocMem(this, sizeof(sceNetCnfPair), kAllocAlignment));
        if (pair == nullptr) {
            return kErrorNoMemory;
        }
        AppendPair(pair);
    } else {
        pair = root->head;
    }
    strcpy(sPairName, data->mInterfaceName);
    strcat(sPairName, kPairNameSeparator);
    strcat(sPairName, data->mDeviceName);
    pair->displayName = sPairName;
    pair->interfaceName = data->mInterfaceName;
    pair->deviceName = data->mDeviceName;
    int result = 0;
    for (int kind = SCE_NETCNF_TYPE_INTERFACE; kind <= SCE_NETCNF_TYPE_DEVICE; ++kind) {
        interface = nullptr;
        result = WriteInterface(data, kind);
        if (result < 0 && result != kErrorNoSettings) {
            break;
        }
        result = Attach(kind);
        if (result < 0) {
            break;
        }
    }
    return result;
}

int NetcnfifEnv::WriteEnv(NetcnfifData *data, int kind) {
    int result = 0;
    if (kind == SCE_NETCNF_TYPE_NET) {
        result = WritePairs(data);
    } else if (kind >= SCE_NETCNF_TYPE_INTERFACE && kind <= SCE_NETCNF_TYPE_DEVICE) {
        result = WriteInterface(data, kind);
        if (result < 0) {
            return result;
        }
        result = Attach(kind);
    } else {
        printf(kUnknownTypeMessage, __func__, kind);
    }
    if (result >= 0) {
        AlignMemory();
    }
    return result;
}
