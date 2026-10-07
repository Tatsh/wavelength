#ifndef NETCNF_H
#define NETCNF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Network configuration files of the netcnf library. A combination ("net") file pairs an interface
 * file with a device file, and loading an entry builds the records below in the memory of a
 * sceNetCnfEnv. Only the members the eznet modules touch are identified.
 */

/** Kinds of configuration file. */
enum sceNetCnfType {
    SCE_NETCNF_TYPE_NET = 0,       /*!< A combination of an interface and a device. */
    SCE_NETCNF_TYPE_INTERFACE = 1, /*!< An interface (connection) file. */
    SCE_NETCNF_TYPE_DEVICE = 2,    /*!< A device (hardware) file. */
};

/** Values of sceNetCnfInterface::type. */
enum sceNetCnfInterfaceType {
    SCE_NETCNF_INTERFACE_ETHERNET = 1, /*!< An Ethernet interface. */
    SCE_NETCNF_INTERFACE_PPP = 2,      /*!< A PPP interface, dialled or over Ethernet. */
    SCE_NETCNF_INTERFACE_NIC = 3,      /*!< A network adaptor treated like Ethernet. */
};

/** Values of sceNetCnfCommand::code. */
enum sceNetCnfCommandCode {
    SCE_NETCNF_COMMAND_NAME_SERVER = 1, /*!< Add a name server. */
    SCE_NETCNF_COMMAND_ROUTE = 3,       /*!< Add a route. */
};

/** Bits of the route flags. */
enum sceNetCnfRouteFlag {
    SCE_NETCNF_ROUTE_GATEWAY = 0x04, /*!< The route goes through its gateway. */
};

/** One entry of a configuration file listing. */
typedef struct sceNetCnfList {
    int type;             /*!< A #sceNetCnfType. */
    int status;           /*!< Entry status. */
    char systemName[256]; /*!< Name of the file that stores the entry. */
    char userName[256];   /*!< Name the user gave the entry. */
} sceNetCnfList;

/** A network address in the library's binary form. */
typedef struct sceNetCnfAddress {
    unsigned char data[20]; /*!< Opaque to the eznet modules. */
} sceNetCnfAddress;

/**
 * Header of an interface command, linked into a list. Each command record begins with it, and its
 * code identifies the record type.
 */
typedef struct sceNetCnfCommand {
    struct sceNetCnfCommand *next;     /*!< Next command. */
    struct sceNetCnfCommand *previous; /*!< Previous command. */
    int code;                          /*!< A #sceNetCnfCommandCode. */
} sceNetCnfCommand;

/** A #SCE_NETCNF_COMMAND_NAME_SERVER command. */
typedef struct sceNetCnfNameServerCommand {
    sceNetCnfCommand header;  /*!< List header. */
    sceNetCnfAddress address; /*!< Server address. */
} sceNetCnfNameServerCommand;

/** A #SCE_NETCNF_COMMAND_ROUTE command. */
typedef struct sceNetCnfRouteCommand {
    sceNetCnfCommand header;      /*!< List header. */
    sceNetCnfAddress destination; /*!< Destination. */
    sceNetCnfAddress gateway;     /*!< Gateway. */
    sceNetCnfAddress genmask;     /*!< Destination mask. */
    unsigned int flags;           /*!< #sceNetCnfRouteFlag bits. */
    unsigned int reserved[5];     /* +0x4c */
} sceNetCnfRouteCommand;

/**
 * Settings of one interface or device file. An unset number is -1 and an unset byte option is
 * 0xff.
 */
typedef struct sceNetCnfInterface {
    int type;                      /*!< A #sceNetCnfInterfaceType. */
    char *vendor;                  /*!< Device vendor. */
    char *product;                 /*!< Device product. */
    unsigned int reserved0;        /* +0x0c */
    unsigned char dhcp;            /*!< Nonzero to configure the address by DHCP. */
    unsigned char reserved1[3];    /* +0x11 */
    char *address;                 /*!< Static address. */
    unsigned int reserved2;        /* +0x18 */
    char *netmask;                 /*!< Static netmask. */
    char *broadcast;               /*!< Static broadcast address. */
    char *chatScript;              /*!< Modem chat script, in the library's coded form. */
    int lastDialNumber;            /*!< Index of the last of dialNumbers that is set. */
    unsigned int reserved3;        /* +0x2c */
    char *dialPrefix;              /*!< Digits dialled before each number. */
    char *dialPrefixPause;         /*!< Pause dialled after dialPrefix. */
    char *dialNumbers[10];         /*!< Telephone numbers. */
    unsigned int reserved4[2];     /* +0x60 */
    int deviceMode;                /*!< A device mode number. */
    unsigned int reserved5;        /* +0x6c */
    char *authName;                /*!< Authentication name. */
    char *authKey;                 /*!< Authentication key. */
    char *peerName;                /*!< Peer name. */
    unsigned int reserved6[3];     /* +0x7c */
    int timeout;                   /*!< A timeout number. */
    unsigned char reserved7[5];    /* +0x8c */
    unsigned char pppOption1;      /*!< A PPP negotiation option. */
    unsigned char reserved8;       /* +0x92 */
    unsigned char pppOption2;      /*!< A PPP negotiation option. */
    unsigned char pppOption3;      /*!< A PPP negotiation option. */
    unsigned char reserved9[2];    /* +0x95 */
    unsigned char pppOption4;      /*!< A PPP negotiation option. */
    unsigned char pppOption5;      /*!< A PPP negotiation option. */
    unsigned char reserved10[91];  /* +0x99 */
    unsigned char pppOption6;      /*!< A PPP negotiation option, 4 by default. */
    unsigned char reserved11[2];   /* +0xf5 */
    unsigned char pppOption7;      /*!< A PPP negotiation option, 0 by default. */
    unsigned char reserved12[54];  /* +0xf8 */
    unsigned char pppoe;           /*!< 1 when a PPP interface runs over Ethernet. */
    unsigned char reserved13[13];  /* +0x12f */
    int interfaceMode;             /*!< An interface mode number. */
    unsigned int reserved14[3];    /* +0x140 */
    int deviceConfig;              /*!< A device configuration number. */
    sceNetCnfCommand *commandHead; /*!< First command. */
    sceNetCnfCommand *commandTail; /*!< Last command. */
    unsigned int reserved15[2];    /* +0x158 */
} sceNetCnfInterface;

/** An interface and a device that form one connection, linked into a list. */
typedef struct sceNetCnfPair {
    struct sceNetCnfPair *next;     /*!< Next pair. */
    struct sceNetCnfPair *previous; /*!< Previous pair. */
    char *displayName;              /*!< Name shown for the pair. */
    char *interfaceName;            /*!< System name of the interface file. */
    char *deviceName;               /*!< System name of the device file. */
    sceNetCnfInterface *interface;  /*!< Interface settings. */
    sceNetCnfInterface *device;     /*!< Device settings. */
    unsigned int reserved[3];       /* +0x1c */
} sceNetCnfPair;

/** The pairs of a combination file. */
typedef struct sceNetCnfRoot {
    sceNetCnfPair *head;       /*!< First pair. */
    sceNetCnfPair *tail;       /*!< Last pair. */
    int version;               /*!< Format version. */
    unsigned int reserved0;    /* +0x0c */
    int setting1;              /*!< A number setting, -1 when unset. */
    int setting2;              /*!< A number setting, -1 when unset. */
    unsigned int reserved1[2]; /* +0x18 */
    int setting3;              /*!< A number setting, -1 when unset. */
    unsigned int reserved2[2]; /* +0x24 */
} sceNetCnfRoot;

/** Working state of a load, and the memory the loaded records live in. */
typedef struct sceNetCnfEnv {
    unsigned int reserved0[2];     /* +0x00 */
    unsigned char *memoryBase;     /*!< Start of the free memory. */
    unsigned char *memoryPointer;  /*!< Next free byte. */
    unsigned char *memoryEnd;      /*!< End of the memory. */
    unsigned int reserved1;        /* +0x14 */
    sceNetCnfRoot *root;           /*!< Loaded combination. */
    sceNetCnfInterface *interface; /*!< Loaded interface or device file. */
    unsigned int reserved2;        /* +0x20 */
    int hostFile;                  /*!< Nonzero for a file on neither a memory card nor a disk. */
    unsigned int reserved3;        /* +0x28 */
    int loadErrors;                /*!< Nonzero when the load reported errors. */
    int preserveDialNumbers;       /*!< Nonzero to skip merging the dial numbers. */
    unsigned char reserved4[2108]; /* +0x34 */
} sceNetCnfEnv;

/**
 * Count the entries of one kind in a configuration file. Export 4.
 *
 * @param fileName Configuration file.
 * @param type A #sceNetCnfType.
 * @return The number of entries, or a negative error code.
 */
int sceNetCnfGetCount(const char *fileName, int type);

/**
 * List the entries of one kind in a configuration file. Export 5.
 *
 * @param fileName Configuration file.
 * @param type A #sceNetCnfType.
 * @param list Receives one record per entry.
 * @return The number of entries, or a negative error code.
 */
int sceNetCnfGetList(const char *fileName, int type, sceNetCnfList *list);

/**
 * Load one entry into an environment. Export 6.
 *
 * @param fileName Configuration file.
 * @param type A #sceNetCnfType.
 * @param userName Name the user gave the entry.
 * @param env Environment that receives the records.
 * @return Zero or a positive value, or a negative error code.
 */
int sceNetCnfLoadEntry(const char *fileName, int type, const char *userName, sceNetCnfEnv *env);

/**
 * Allocate from the memory of an environment. Export 10.
 *
 * @param env Environment.
 * @param size Byte count.
 * @param alignment Alignment selector. The eznet modules pass 2.
 * @return The memory, or null when the environment has too little free memory.
 */
void *sceNetCnfAllocMem(sceNetCnfEnv *env, int size, int alignment);

/**
 * Initialise the settings of an interface or device. Export 11.
 *
 * @param interface Settings to initialise.
 */
void sceNetCnfInitInterface(sceNetCnfInterface *interface);

/**
 * Convert the text form of an address. Export 15.
 *
 * @param address Receives the address.
 * @param text Text, or null for the empty address.
 * @return Zero, or a negative error code.
 */
int sceNetCnfName2Address(sceNetCnfAddress *address, const char *text);

/**
 * Write the text form of an address. Export 16.
 *
 * @param text Receives the text.
 * @param size Size of @p text.
 * @param address Address.
 * @return Zero, or a negative error code.
 */
int sceNetCnfAddress2String(char *text, int size, const sceNetCnfAddress *address);

/**
 * Export 20. Convert the text of a chat script to the coded form sceNetCnfInterface::chatScript
 * stores.
 *
 * @param text Script text.
 * @param coded Receives the coded form.
 * @param size Size of @p coded.
 * @return Zero, or a negative error code.
 */
int netcnf_20(const char *text, char *coded, int size);

/**
 * Export 21. Convert the coded form of a chat script back to text.
 *
 * @param coded Coded form.
 * @param text Receives the text.
 * @param size Size of @p text.
 * @return Zero, or a negative error code.
 */
int netcnf_21(const char *coded, char *text, int size);

/**
 * Export 22. The ezncnf module records its result as the status of an interface entry.
 *
 * @param fileName Configuration file.
 * @param type A #sceNetCnfType.
 * @param userName Name the user gave the entry.
 * @param env Environment the entry was loaded into.
 * @return A status.
 */
int netcnf_22(const char *fileName, int type, const char *userName, sceNetCnfEnv *env);

#ifdef __cplusplus
}
#endif

#endif
