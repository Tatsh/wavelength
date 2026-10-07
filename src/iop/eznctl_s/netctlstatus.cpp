#include "eznctl_s/netctlstatus.h"

#include <inet.h>
#include <inetctl.h>
#include <sysclib.h>

#include "eznctl_s/eznetctl.h"

namespace {

constexpr char kDialingPhaseMessage[] = "PHASE Dialing";
constexpr char kDialingPrefix[] = "Dialing ";
constexpr char kAuthFailedMessage[] = "AUTH Failed";
constexpr char kDisconnectAuthMessage[] = "DISCONNECT Auth";
#ifdef VIDEO_STANDARD_PAL
constexpr char kAuthenticationFailedMessage[] = "Authentication failure";
#else
constexpr char kAuthenticationFailedMessage[] = "Authentication failed";
#endif

constexpr int kDialingPrefixLength = sizeof(kDialingPrefix) - 1;
constexpr int kStateCount = INETCTL_STATE_DISCONNECTED + 1;

// NTSC-U/C: 0x00002168, PAL: 0x0000217c
const char *const kStateNames[kStateCount] = {
    "Detached",
    "Connecting",
    "Retrying",
    "Connected",
    "Disconnecting",
    "Disconnected",
};

} // namespace

int NetCtlStatus::Update() {
#ifdef VIDEO_STANDARD_PAL
    if (EzNetCtl::sNetChecked != 0) {
        return INETCTL_STATE_DISCONNECTED;
    }
#endif
    mResult = -1;
    mLinkStatus = -1;
    mState = INETCTL_STATE_DETACHED;
    bzero(mMessage, kMessageSize);
    if (EzNetCtl::sEthernetId != 0) {
        mInterfaceId = EzNetCtl::sEthernetId;
        mResult = sceInetCtlGetState(EzNetCtl::sEthernetId, &mState);
        mLinkStatus =
            sceInetInterfaceControl(EzNetCtl::sEthernetId, INET_CONTROL_LINK_STATUS, nullptr, 0);
    }
    if (EzNetCtl::sPppId != 0) {
        mInterfaceId = EzNetCtl::sPppId;
        mResult = sceInetCtlGetState(EzNetCtl::sPppId, &mState);
        switch (mState) {
        case INETCTL_STATE_CONNECTED:
            if (EzNetCtl::sEthernetId == 0) {
                sceInetInterfaceControl(
                    EzNetCtl::sPppId, INET_CONTROL_PPP_CONNECTION_MESSAGE, mMessage, kMessageSize);
            }
            break;
        case INETCTL_STATE_CONNECTING:
            sceInetInterfaceControl(
                EzNetCtl::sPppId, INET_CONTROL_PPP_MESSAGE, mMessage, kMessageSize);
            if (strcmp(mMessage, kDialingPhaseMessage) == 0) {
                strcpy(mMessage, kDialingPrefix);
                sceInetInterfaceControl(EzNetCtl::sPppId,
                                        INET_CONTROL_PPP_DIAL_NUMBER,
                                        &mMessage[kDialingPrefixLength],
                                        kMessageSize - kDialingPrefixLength);
            }
            break;
        case INETCTL_STATE_DISCONNECTED:
#ifndef VIDEO_STANDARD_PAL
            sceInetInterfaceControl(
                EzNetCtl::sPppId, INET_CONTROL_PPP_DISCONNECTION_MESSAGE, mMessage, kMessageSize);
            if (strlen(mMessage) == 0) {
                sceInetInterfaceControl(
                    EzNetCtl::sPppId, INET_CONTROL_PPP_MESSAGE, mMessage, kMessageSize);
            }
#else
            sceInetInterfaceControl(
                EzNetCtl::sPppId, INET_CONTROL_PPP_MESSAGE, mMessage, kMessageSize);
#endif
            if (strcmp(mMessage, kAuthFailedMessage) == 0 ||
                strcmp(mMessage, kDisconnectAuthMessage) == 0) {
                strcpy(mMessage, kAuthenticationFailedMessage);
            }
            break;
        default:
            break;
        }
    }
    size_t length = strlen(mMessage);
    while (length != 0 && (look_ctype_table(mMessage[length - 1]) & CTYPE_SPACE) != 0) {
        --length;
        mMessage[length] = '\0';
    }
    if (length == 0 && mState >= 0 && mState < kStateCount) {
        strcpy(mMessage, kStateNames[mState]);
    }
    return mResult;
}
