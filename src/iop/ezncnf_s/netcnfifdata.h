#pragma once

#include <netcnf.h>

/**
 * A combination in the flat form the EE reads, with every setting as text or a number. An unset
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
     * Clear every setting to its unset value.
     *
     * @ghidraAddress NTSC-U/C: 0x00000c9c
     * @ghidraAddress PAL: 0x00000c9c
     */
    void Reset();

    /**
     * Fill the record from an environment.
     *
     * @param env Environment.
     * @param kind #SCE_NETCNF_TYPE_NET to read the combination, or the kind of the single file
     * loaded.
     * @return Zero, or a negative error code.
     * @ghidraAddress NTSC-U/C: 0x000011a8
     * @ghidraAddress PAL: 0x000011a8
     */
    int ReadEnv(const sceNetCnfEnv *env, int kind);

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

private:
    /**
     * Fill the record from each pair of a combination. Each pair overwrites the last.
     *
     * @param root Combination.
     * @return The result of the last interface read.
     * @ghidraAddress NTSC-U/C: 0x000010e8
     * @ghidraAddress PAL: 0x000010e8
     */
    int ReadPairs(const sceNetCnfRoot *root);

    /**
     * Fill the settings of one interface or device file.
     *
     * @param interface Settings.
     * @param kind #SCE_NETCNF_TYPE_INTERFACE or #SCE_NETCNF_TYPE_DEVICE.
     * @return Zero, or a negative error code.
     * @ghidraAddress NTSC-U/C: 0x00000dc8
     * @ghidraAddress PAL: 0x00000dc8
     */
    int ReadInterface(const sceNetCnfInterface *interface, int kind);

    /**
     * Fill the setting of one interface command. Only the first two name servers are recorded.
     *
     * @param command Command.
     * @param nameServerCount Name servers read so far, advanced for each one.
     * @return Zero, or a negative error code.
     * @ghidraAddress NTSC-U/C: 0x00000d10
     * @ghidraAddress PAL: 0x00000d10
     */
    int ReadCommand(const sceNetCnfCommand *command, int *nameServerCount);

    /** Name servers ReadInterface() has read. */
    static int sNameServerCount;
};
