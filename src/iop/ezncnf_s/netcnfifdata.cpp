#include "ezncnf_s/netcnfifdata.h"

#include <stdio.h>

#include <sysclib.h>

namespace {

constexpr int kUnset = -1;
constexpr unsigned char kUnsetOption = 0xff;
constexpr unsigned char kDefaultPppOption6 = 4;
constexpr int kInterfaceDialNumberCount = 10;
constexpr char kUnknownTypeMessage[] = "[%s] unknown type (%d)\n";

} // namespace

// NTSC-U/C: 0x000027a0
int NetcnfifData::sNameServerCount;

void NetcnfifData::Reset() {
    bzero(this, sizeof(NetcnfifData));
    mInterfaceType = kUnset;
    mInterfaceMode = kUnset;
    mInterfaceTimeout = kUnset;
    mDeviceType = kUnset;
    mDeviceConfig = kUnset;
    mDeviceMode = kUnset;
    mDeviceTimeout = kUnset;
    mDhcp = kUnsetOption;
    mPppOption4 = kUnsetOption;
    mPppOption5 = kUnsetOption;
    mPppOption7 = 0;
    mPppOption6 = kDefaultPppOption6;
    mPppoe = kUnsetOption;
    mPppOption2 = kUnsetOption;
    mPppOption3 = kUnsetOption;
    mPppOption1 = kUnsetOption;
}

int NetcnfifData::ReadCommand(const sceNetCnfCommand *command, int *nameServerCount) {
    int result = 0;
    switch (command->code) {
    case SCE_NETCNF_COMMAND_NAME_SERVER: {
        char *text;
        if (*nameServerCount == 0) {
            text = mNameServer1;
        } else if (*nameServerCount == 1) {
            text = mNameServer2;
        } else {
            return 0;
        }
        const auto *nameServer =
            reinterpret_cast<const sceNetCnfNameServerCommand *>(command); // Header is member 0.
        result = sceNetCnfAddress2String(text, kTextSize, &nameServer->address);
        ++*nameServerCount;
        break;
    }
    case SCE_NETCNF_COMMAND_ROUTE: {
        const auto *route =
            reinterpret_cast<const sceNetCnfRouteCommand *>(command); // Header is member 0.
        result = sceNetCnfAddress2String(mGateway, kTextSize, &route->gateway);
        break;
    }
    default:
        break;
    }
    return result;
}

int NetcnfifData::ReadInterface(const sceNetCnfInterface *interface, int kind) {
    int result = 0;
    switch (kind) {
    case SCE_NETCNF_TYPE_INTERFACE:
        mInterfaceType = interface->type;
        mDhcp = interface->dhcp;
        if (interface->address != nullptr) {
            strcpy(mAddress, interface->address);
        }
        if (interface->netmask != nullptr) {
            strcpy(mNetmask, interface->netmask);
        }
        if (interface->broadcast != nullptr) {
            strcpy(mBroadcast, interface->broadcast);
        }
        sNameServerCount = 0;
        for (const sceNetCnfCommand *command = interface->commandHead; command != nullptr;
             command = command->next) {
            result = ReadCommand(command, &sNameServerCount);
            if (result < 0) {
                return result;
            }
        }
        for (int i = 0; i < kInterfaceDialNumberCount; ++i) {
            if (interface->dialNumbers[i] == nullptr) {
                continue;
            }
            // Only the first kDialNumberCount numbers are copied.
            if (i < kDialNumberCount) {
                strcpy(mDialNumbers[i], interface->dialNumbers[i]);
            }
        }
        if (interface->authName != nullptr) {
            strcpy(mAuthName, interface->authName);
        }
        if (interface->authKey != nullptr) {
            strcpy(mAuthKey, interface->authKey);
        }
        if (interface->peerName != nullptr) {
            strcpy(mPeerName, interface->peerName);
        }
        mPppOption4 = interface->pppOption4;
        mPppOption5 = interface->pppOption5;
        mPppOption7 = interface->pppOption7;
        mPppOption6 = interface->pppOption6;
        mPppoe = interface->pppoe;
        mPppOption2 = interface->pppOption2;
        mPppOption3 = interface->pppOption3;
        mPppOption1 = interface->pppOption1;
        mInterfaceMode = interface->interfaceMode;
        mInterfaceTimeout = interface->timeout;
        break;
    case SCE_NETCNF_TYPE_DEVICE:
        mDeviceType = interface->type;
        if (interface->vendor != nullptr) {
            strcpy(mVendor, interface->vendor);
        }
        if (interface->product != nullptr) {
            strcpy(mProduct, interface->product);
        }
        mDeviceConfig = interface->deviceConfig;
        if (interface->chatScript != nullptr) {
            result = netcnf_21(interface->chatScript, mChatScript, kTextSize);
            if (result < 0) {
                return result;
            }
        }
        if (interface->dialPrefix != nullptr) {
            strcpy(mDialPrefix, interface->dialPrefix);
        }
        if (interface->dialPrefixPause != nullptr) {
            strcpy(mDialPrefixPause, interface->dialPrefixPause);
        }
        mDeviceMode = interface->deviceMode;
        mDeviceTimeout = interface->timeout;
        break;
    default:
        break;
    }
    return result;
}

int NetcnfifData::ReadPairs(const sceNetCnfRoot *root) {
    int result = 0;
    for (const sceNetCnfPair *pair = root->head; pair != nullptr; pair = pair->next) {
        Reset();
        strcpy(mInterfaceName, pair->interfaceName);
        strcpy(mDeviceName, pair->deviceName);
        if (pair->interface != nullptr) {
            result = ReadInterface(pair->interface, SCE_NETCNF_TYPE_INTERFACE);
        }
        if (pair->device != nullptr) {
            result = ReadInterface(pair->device, SCE_NETCNF_TYPE_DEVICE);
        }
    }
    return result;
}

int NetcnfifData::ReadEnv(const sceNetCnfEnv *env, int kind) {
    int result = 0;
    if (kind == SCE_NETCNF_TYPE_NET) {
        result = ReadPairs(env->root);
    } else if (kind >= SCE_NETCNF_TYPE_INTERFACE && kind <= SCE_NETCNF_TYPE_DEVICE) {
        result = ReadInterface(env->interface, kind);
    } else {
        printf(kUnknownTypeMessage, __func__, kind);
    }
    return result;
}
