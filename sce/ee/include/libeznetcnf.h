#ifndef LIBEZNETCNF_H
#define LIBEZNETCNF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Client of the eznetcnf server, which lists and loads the network configurations of a
 * configuration file on the IOP. The unit is Sony's sample libeznetcnf.c. Its server is the
 * ezncnf_s module.
 */

/** Device types of a combination. */
#define EZ_NETCNF_DEVICE_NONE 0     /*!< The combination has no interface, or an unknown one. */
#define EZ_NETCNF_DEVICE_MODEM 1    /*!< A dialled PPP connection. */
#define EZ_NETCNF_DEVICE_PPPOE 2    /*!< A PPP connection over Ethernet. */
#define EZ_NETCNF_DEVICE_ETHERNET 3 /*!< An Ethernet connection. */

/** The most combinations a list reports. */
#define EZ_NETCNF_LIST_MAX 10

/** The size of each name of a combination, the terminator included. */
#define EZ_NETCNF_NAME_SIZE 32

/** One combination of a configuration file. */
typedef struct {
    int status;                              /*!< Zero, -1 after load errors, or -8 when unused. */
    int deviceType;                          /*!< One of the EZ_NETCNF_DEVICE_ values. */
    char name[EZ_NETCNF_NAME_SIZE];          /*!< User name of the combination. */
    char interfaceName[EZ_NETCNF_NAME_SIZE]; /*!< User name of its interface. */
    char deviceName[EZ_NETCNF_NAME_SIZE];    /*!< User name of its device. */
} ezNetCnfCombination_t;

/** The size of each text of ezNetCnfData_t, the terminator included. */
#define EZ_NETCNF_TEXT_SIZE 256

/** The number of telephone numbers of ezNetCnfData_t. */
#define EZ_NETCNF_DIAL_NUMBERS 3

/**
 * The settings of one combination, as ezNetCnfLoadEntry() delivers them and
 * ezNetCtlSetProperties() applies them. The record is 0x1340 bytes.
 */
typedef struct {
    char interfaceName[EZ_NETCNF_TEXT_SIZE]; /*!< System name of the interface file. */
    char deviceName[EZ_NETCNF_TEXT_SIZE];    /*!< System name of the device file. */
    char address[EZ_NETCNF_TEXT_SIZE];       /*!< The address of the interface. */
    char netmask[EZ_NETCNF_TEXT_SIZE];       /*!< The network mask. */
    char broadcast[EZ_NETCNF_TEXT_SIZE];     /*!< The broadcast address. */
    char gateway[EZ_NETCNF_TEXT_SIZE];       /*!< The gateway. */
    char nameServer1[EZ_NETCNF_TEXT_SIZE];   /*!< The first name server. */
    char nameServer2[EZ_NETCNF_TEXT_SIZE];   /*!< The second name server. */
    char dialNumbers[EZ_NETCNF_DIAL_NUMBERS][EZ_NETCNF_TEXT_SIZE]; /*!< The telephone numbers. */
    char authName[EZ_NETCNF_TEXT_SIZE];        /*!< The account of the provider. */
    char authKey[EZ_NETCNF_TEXT_SIZE];         /*!< The password of the account. */
    char peerName[EZ_NETCNF_TEXT_SIZE];        /*!< The name of the peer. */
    char vendor[EZ_NETCNF_TEXT_SIZE];          /*!< The vendor of the device. */
    char product[EZ_NETCNF_TEXT_SIZE];         /*!< The product of the device. */
    char chatScript[EZ_NETCNF_TEXT_SIZE];      /*!< The chat script of the device. */
    char dialPrefix[EZ_NETCNF_TEXT_SIZE];      /*!< The prefix dialled first. */
    char dialPrefixPause[EZ_NETCNF_TEXT_SIZE]; /*!< The pause after the prefix. */
    int interfaceType;                         /*!< The type of the interface. */
    int interfaceMode;                         /*!< The mode of the interface. */
    int interfaceTimeout;                      /*!< The idle timeout of the interface. */
    int deviceType;                            /*!< The type of the device. */
    int deviceConfig;                          /*!< The configuration of the device. */
    int deviceMode;                            /*!< The mode of the device. */
    int deviceTimeout;                         /*!< The timeout of the device. */
    unsigned int reserved0;                    /*!< Not read by the server. */
    unsigned char dhcp;                        /*!< Whether DHCP configures the interface. */
    unsigned char pppOption4;                  /*!< A PPP option. */
    unsigned char pppOption5;                  /*!< A PPP option. */
    unsigned char pppOption7;                  /*!< A PPP option. */
    unsigned char pppOption6;                  /*!< A PPP option. */
    unsigned char pppoe;                       /*!< Whether PPP runs over Ethernet. */
    unsigned char pppOption2;                  /*!< A PPP option. */
    unsigned char pppOption3;                  /*!< A PPP option. */
    unsigned char pppOption1;                  /*!< A PPP option. */
    unsigned char reserved1[23];               /*!< Not read by the server. */
} ezNetCnfData_t;

/** The combinations of a configuration file, as ezNetCnfGetList() fills them. */
typedef struct {
    int listLength;                                 /*!< 6 on a memory card, otherwise 10. */
    int defaultNumber;                              /*!< The first of numbers. */
    int numbers[EZ_NETCNF_LIST_MAX];                /*!< The number of each combination. */
    ezNetCnfCombination_t list[EZ_NETCNF_LIST_MAX]; /*!< The combinations, by number less one. */
} ezNetCnfCombinationList_t;

/**
 * Bind to the eznetcnf server, retrying until the server answers.
 *
 * A missing module is reported and not otherwise handled.
 *
 * @return 1.
 * @ghidraAddress NTSC-U/C: 0x0026c648
 * @ghidraAddress PAL: 0x002761d8
 */
int ezNetCnfInit(void);

/**
 * List the combinations of a configuration file into a list in main memory.
 *
 * @param fname The configuration file.
 * @param flags The request flags. Bit 0 records the state of each interface.
 * @param list Receives the list.
 * @return The result of the server, or -1 before ezNetCnfInit() bound.
 * @ghidraAddress NTSC-U/C: 0x0026c728
 * @ghidraAddress PAL: 0x002762b8
 */
int ezNetCnfGetList(const char *fname, int flags, ezNetCnfCombinationList_t *list);

/**
 * Report a combination of a list.
 *
 * @param list The list.
 * @param number The number of the combination, from 1.
 * @return The combination, or null with a message for an empty list or a number out of range.
 * @ghidraAddress NTSC-U/C: 0x0026c7d8
 * @ghidraAddress PAL: 0x00276368
 */
ezNetCnfCombination_t *ezNetCnfGetCombination(ezNetCnfCombinationList_t *list, int number);

/**
 * Load a combination of a configuration file into a buffer in main memory.
 *
 * The name is inferred.
 *
 * @param fname The configuration file.
 * @param usrname The user name of the combination.
 * @param dest Receives the settings.
 * @return The result of the server, or -1 before ezNetCnfInit() bound.
 * @ghidraAddress NTSC-U/C: 0x0026c848
 * @ghidraAddress PAL: 0x002763d8
 */
int ezNetCnfLoadEntry(const char *fname, const char *usrname, ezNetCnfData_t *dest);

#ifdef __cplusplus
}
#endif

#endif
