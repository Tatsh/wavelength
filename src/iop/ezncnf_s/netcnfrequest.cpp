#include "ezncnf_s/netcnfrequest.h"

#include <stdio.h>

#include <kernel.h>
#include <sysclib.h>

#include "ezncnf_s/eznetcnf.h"
#include "ezncnf_s/netcnfifdata.h"

namespace {

// AllocSysMemory() placement of the highest free area that fits.
constexpr int kAllocHighest = 1;
constexpr int kErrorNoMemory = -2;
constexpr int kEntryStatusUnused = -8;
constexpr int kEntryStatusLoadErrors = -1;
constexpr int kDecimal = 10;

// Combinations a file on each device can store.
constexpr int kMemoryCardEntryLimit = 6;
constexpr int kOtherEntryLimit = 10;

constexpr char kMessagePrefix[] = "%s> ";
constexpr char kGetCountErrorMessage[] = "sceNetCnfGetCount(%s) error %d\n";
constexpr char kAllocErrorMessage[] = "AllocSysMemory failed\n";
constexpr char kListLimitMessage[] = "ezList length limit reached at \"%s\"";

} // namespace

char *NetCnfRequest::FindUserName(const char *systemName, sceNetCnfList *list, int count) {
    for (int i = 0; i < count; ++i) {
        if (strcmp(list[i].systemName, systemName) == 0) {
            return list[i].userName;
        }
    }
    return nullptr;
}

int NetCnfRequest::ParseNumber(const char *name) {
    for (const char *c = name; *c != '\0'; ++c) {
        if ((look_ctype_table(*c) & CTYPE_DIGIT) != 0) {
            return static_cast<int>(strtol(c, nullptr, kDecimal));
        }
    }
    return 0;
}

inline int NetCnfRequest::FillList(sceNetCnfList *list,
                                   NetCnfListReply *reply,
                                   int netCount,
                                   int interfaceCount,
                                   int deviceCount) {
    bzero(list, (netCount + interfaceCount + deviceCount) * sizeof(sceNetCnfList));
    bzero(reply, sizeof(NetCnfListReply));
    for (int i = 0; i < kNetCnfListEntryCount; ++i) {
        reply->mEntries[i].mStatus = kEntryStatusUnused;
    }
    sceNetCnfList *interfaces = &list[netCount];
    sceNetCnfList *devices = &interfaces[interfaceCount];
    sceNetCnfGetList(mFileName, SCE_NETCNF_TYPE_NET, list);
    sceNetCnfGetList(mFileName, SCE_NETCNF_TYPE_INTERFACE, interfaces);
    sceNetCnfGetList(mFileName, SCE_NETCNF_TYPE_DEVICE, devices);
    NetcnfifEnv &env = EzNetCnf::sEnv;
    for (int i = 0; i < netCount; ++i) {
        if (i >= kNetCnfListEntryCount) {
            printf(kMessagePrefix, __FILE__);
            printf(kListLimitMessage, list[i].systemName);
            break;
        }
        char *userName = list[i].userName;
        const int number = ParseNumber(userName);
        if (number <= 0) {
            continue;
        }
        reply->mNumbers[i] = number;
        const int result = env.LoadEntry(mFileName, userName);
        if (result < 0) {
            return result;
        }
        // An entry is stored at the index its name numbers, without a range check.
        NetCnfListEntry *entry = &reply->mEntries[number - 1];
        entry->mDeviceType = env.GetDeviceType();
        strncpy(entry->mName, userName, kNetCnfListNameSize - 1);
        // A name missing from a listing is copied from a null pointer.
        strncpy(entry->mDeviceName,
                FindUserName(env.root->head->deviceName, devices, deviceCount),
                kNetCnfListNameSize - 1);
        char *interfaceName =
            FindUserName(env.root->head->interfaceName, interfaces, interfaceCount);
        strncpy(entry->mInterfaceName, interfaceName, kNetCnfListNameSize - 1);
        entry->mStatus = env.loadErrors != 0 ? kEntryStatusLoadErrors : 0;
        if ((mFlags & kFlagQueryInterface) != 0) {
            entry->mStatus = netcnf_22(mFileName, SCE_NETCNF_TYPE_INTERFACE, interfaceName, &env);
        }
    }
    reply->mDefaultNumber = reply->mNumbers[0];
    const int result = NetcnfifEnv::SelectByDevice(
        mFileName, kMemoryCardEntryLimit, kOtherEntryLimit, kOtherEntryLimit);
    reply->mEntryLimit = result;
    EzNetCnf::SendToEe(reply, mDestination, sizeof(NetCnfListReply), false);
    return result;
}

int NetCnfRequest::GetList() {
    const int netCount = sceNetCnfGetCount(mFileName, SCE_NETCNF_TYPE_NET);
    if (netCount <= 0) {
        printf(kMessagePrefix, __FILE__);
        printf(kGetCountErrorMessage, mFileName, netCount);
        return netCount;
    }
    const int interfaceCount = sceNetCnfGetCount(mFileName, SCE_NETCNF_TYPE_INTERFACE);
    const int deviceCount = sceNetCnfGetCount(mFileName, SCE_NETCNF_TYPE_DEVICE);
    const int total = netCount + interfaceCount + deviceCount;
    CpuSuspendIntr(&EzNetCnf::sInterruptState);
    auto *list = static_cast<sceNetCnfList *>(
        AllocSysMemory(kAllocHighest, total * sizeof(sceNetCnfList), nullptr));
    auto *reply = static_cast<NetCnfListReply *>(
        AllocSysMemory(kAllocHighest, sizeof(NetCnfListReply), nullptr));
    CpuResumeIntr(EzNetCnf::sInterruptState);
    int result;
    if (list == nullptr || reply == nullptr) {
        printf(kMessagePrefix, __FILE__);
        printf(kAllocErrorMessage);
        result = kErrorNoMemory;
    } else {
        result = FillList(list, reply, netCount, interfaceCount, deviceCount);
    }
    CpuSuspendIntr(&EzNetCnf::sInterruptState);
    if (list != nullptr) {
        FreeSysMemory(list);
    }
    if (reply != nullptr) {
        FreeSysMemory(reply);
    }
    CpuResumeIntr(EzNetCnf::sInterruptState);
    return result;
}

int NetCnfRequest::LoadEntry() {
    CpuSuspendIntr(&EzNetCnf::sInterruptState);
    auto *data =
        static_cast<NetcnfifData *>(AllocSysMemory(kAllocHighest, sizeof(NetcnfifData), nullptr));
    CpuResumeIntr(EzNetCnf::sInterruptState);
    int result;
    if (data == nullptr) {
        printf(kMessagePrefix, __FILE__);
        printf(kAllocErrorMessage);
        result = kErrorNoMemory;
    } else {
        bzero(data, sizeof(NetcnfifData));
        result = EzNetCnf::sEnv.LoadEntry(mFileName, mUserName);
        if (result >= 0) {
            data->ReadEnv(&EzNetCnf::sEnv, SCE_NETCNF_TYPE_NET); // The binary discards the result.
            EzNetCnf::SendToEe(data, mDestination, sizeof(NetcnfifData), false);
        }
    }
    if (data != nullptr) {
        CpuSuspendIntr(&EzNetCnf::sInterruptState);
        FreeSysMemory(data);
        CpuResumeIntr(EzNetCnf::sInterruptState);
    }
    return result;
}
