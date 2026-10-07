#pragma once

/** Kinds of connection a NetCnfListEntry reports. */
enum NetCnfDeviceType {
    kNetCnfDeviceTypeNone = 0,     /*!< The combination has no interface, or an unknown one. */
    kNetCnfDeviceTypeModem = 1,    /*!< A dialled PPP connection. */
    kNetCnfDeviceTypePppoe = 2,    /*!< A PPP connection over Ethernet. */
    kNetCnfDeviceTypeEthernet = 3, /*!< An Ethernet connection. */
};

/** Number of combinations a NetCnfListReply describes. */
constexpr int kNetCnfListEntryCount = 10;

/** Size of each name of a NetCnfListEntry. */
constexpr int kNetCnfListNameSize = 32;

/** One combination of a NetCnfListReply. */
struct NetCnfListEntry {
    int mStatus;                              /*!< Zero, -1 after load errors, or -8 when unused. */
    int mDeviceType;                          /*!< A NetCnfDeviceType. */
    char mName[kNetCnfListNameSize];          /*!< User name of the combination. */
    char mInterfaceName[kNetCnfListNameSize]; /*!< User name of its interface. */
    char mDeviceName[kNetCnfListNameSize];    /*!< User name of its device. */
};

/**
 * The combinations of a configuration file, as NetCnfRequest::GetList() sends them to the EE.
 * Each combination is stored at the index its user name numbers, counting from one.
 */
struct NetCnfListReply {
    int mEntryLimit;                                 /*!< 6 on a memory card, otherwise 10. */
    int mDefaultNumber;                              /*!< First of mNumbers. */
    int mNumbers[kNetCnfListEntryCount];             /*!< Number of each listed combination. */
    NetCnfListEntry mEntries[kNetCnfListEntryCount]; /*!< The combinations by number. */
};
