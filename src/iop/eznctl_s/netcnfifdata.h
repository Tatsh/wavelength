#pragma once

/**
 * A combination in the flat form the EE writes, with every setting as text or a number. An unset
 * number is -1 and an unset byte option is 0xff.
 *
 * The module was built without RTTI. The name is inferred from the module's routines.
 */
class NetcnfifData {
public:
    /** Size of each text setting. */
    static constexpr int kTextSize = 256;

    /** Number of dial numbers. */
    static constexpr int kDialNumberCount = 3;

    /**
     * Clear every setting to its unset value. The module never calls it.
     *
     * @ghidraAddress NTSC-U/C: 0x00000f1c
     * @ghidraAddress PAL: 0x00000f2c
     */
    void Reset();

    char mInterfaceName[kTextSize];                 /*!< System name of the interface file. */
    char mDeviceName[kTextSize];                    /*!< System name of the device file. */
    char mAddress[kTextSize];                       /*!< sceNetCnfInterface::address. */
    char mNetmask[kTextSize];                       /*!< sceNetCnfInterface::netmask. */
    char mBroadcast[kTextSize];                     /*!< sceNetCnfInterface::broadcast. */
    char mGateway[kTextSize];                       /*!< Gateway of the route command. */
    char mNameServer1[kTextSize];                   /*!< First name server. */
    char mNameServer2[kTextSize];                   /*!< Second name server. */
    char mDialNumbers[kDialNumberCount][kTextSize]; /*!< sceNetCnfInterface::dialNumbers. */
    char mAuthName[kTextSize];                      /*!< sceNetCnfInterface::authName. */
    char mAuthKey[kTextSize];                       /*!< sceNetCnfInterface::authKey. */
    char mPeerName[kTextSize];                      /*!< sceNetCnfInterface::peerName. */
    char mVendor[kTextSize];          /*!< sceNetCnfInterface::vendor of the device. */
    char mProduct[kTextSize];         /*!< sceNetCnfInterface::product of the device. */
    char mChatScript[kTextSize];      /*!< Text of the device's chat script. */
    char mDialPrefix[kTextSize];      /*!< sceNetCnfInterface::dialPrefix. */
    char mDialPrefixPause[kTextSize]; /*!< sceNetCnfInterface::dialPrefixPause. */
    int mInterfaceType;               /*!< sceNetCnfInterface::type of the interface. */
    int mInterfaceMode;               /*!< sceNetCnfInterface::interfaceMode. */
    int mInterfaceTimeout;            /*!< sceNetCnfInterface::timeout. */
    int mDeviceType;                  /*!< sceNetCnfInterface::type of the device. */
    int mDeviceConfig;                /*!< sceNetCnfInterface::deviceConfig. */
    int mDeviceMode;                  /*!< sceNetCnfInterface::deviceMode. */
    int mDeviceTimeout;               /*!< sceNetCnfInterface::timeout of the device. */
    unsigned int mReserved0;          /*!< Never touched by the module. */
    unsigned char mDhcp;              /*!< sceNetCnfInterface::dhcp. */
    unsigned char mPppOption4;        /*!< sceNetCnfInterface::pppOption4. */
    unsigned char mPppOption5;        /*!< sceNetCnfInterface::pppOption5. */
    unsigned char mPppOption7;        /*!< sceNetCnfInterface::pppOption7. */
    unsigned char mPppOption6;        /*!< sceNetCnfInterface::pppOption6. */
    unsigned char mPppoe;             /*!< sceNetCnfInterface::pppoe. */
    unsigned char mPppOption2;        /*!< sceNetCnfInterface::pppOption2. */
    unsigned char mPppOption3;        /*!< sceNetCnfInterface::pppOption3. */
    unsigned char mPppOption1;        /*!< sceNetCnfInterface::pppOption1. */
    unsigned char mReserved1[23];     /*!< Never touched by the module. */
};
