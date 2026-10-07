#include "eznctl_s/netcnfifdata.h"

#include <sysclib.h>

namespace {

constexpr int kUnset = -1;
constexpr unsigned char kUnsetOption = 0xff;
constexpr unsigned char kDefaultPppOption6 = 4;

} // namespace

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
