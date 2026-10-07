#pragma once

/**
 * Connection state the EE requests with EzNetCtl::kFunctionGetStatus. The reply overwrites the
 * request buffer.
 *
 * The module was built without RTTI. The name is inferred from the module's routines.
 */
class NetCtlStatus {
public:
    /** Size of mMessage. */
    static constexpr int kMessageSize = 256;

    /**
     * Read the state of the started interfaces, the PPP one last, and describe it in mMessage.
     *
     * @return mResult, or #INETCTL_STATE_DISCONNECTED without reading once the EE set
     * EzNetCtl::sNetChecked.
     * @ghidraAddress NTSC-U/C: 0x0000033c
     * @ghidraAddress PAL: 0x0000033c
     */
    int Update();

    int mResult;                 /*!< Result of sceInetCtlGetState(), or -1. */
    int mInterfaceId;            /*!< Interface read last. */
    int mLinkStatus;             /*!< #INET_CONTROL_LINK_STATUS of the Ethernet interface, or -1. */
    int mState;                  /*!< An #InetCtlState. */
    char mMessage[kMessageSize]; /*!< Description of the state. */
};
