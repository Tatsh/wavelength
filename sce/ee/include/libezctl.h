#ifndef LIBEZCTL_H
#define LIBEZCTL_H

#include <libeznetcnf.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Client of the eznetctl server, which brings the network interface of the IOP up with a
 * configuration and reports its state. The unit is Sony's sample libezctl.c. Its server is the
 * eznctl_s module.
 */

/** The size of the description of ezNetCtlStatus_t, the terminator included. */
#define EZ_NETCTL_MESSAGE_SIZE 256

/** The size of the host name buffer of ezNetCtlLookUpName(), the terminator included. */
#define EZ_NETCTL_NAME_SIZE 256

/** The state of the network interface, as ezNetCtlGetStatus() fills it. */
typedef struct {
    int result;                           /*!< The result of the state query, or -1. */
    int interfaceId;                      /*!< The interface read last. */
    int linkStatus;                       /*!< The link state of the Ethernet interface, or -1. */
    int state;                            /*!< The state of the interface. */
    char message[EZ_NETCTL_MESSAGE_SIZE]; /*!< The description of the state. */
} ezNetCtlStatus_t;

/**
 * Bind to the eznetctl server, retrying until the server answers.
 *
 * A missing module is reported and not otherwise handled.
 *
 * @return 1.
 * @ghidraAddress NTSC-U/C: 0x0026c8f8
 * @ghidraAddress PAL: 0x00276488
 */
int ezNetCtlInit(void);

/**
 * Send settings to the server and bring the interface up with them.
 *
 * @param data The settings.
 * @return The result of the server, -1 before ezNetCtlInit() bound, or -2 when the server
 * reports no buffer for the settings.
 * @ghidraAddress NTSC-U/C: 0x0026c9d8
 * @ghidraAddress PAL: 0x00276578
 */
int ezNetCtlSetProperties(const ezNetCnfData_t *data);

/**
 * Read the state of the interface.
 *
 * @param status Receives the state.
 * @return status->result, or -1 before ezNetCtlInit() bound.
 * @ghidraAddress NTSC-U/C: 0x0026cb08
 * @ghidraAddress PAL: 0x002766a8
 */
int ezNetCtlGetStatus(ezNetCtlStatus_t *status);

/**
 * Bring an interface up.
 *
 * The name is inferred.
 *
 * @param id The interface.
 * @return 0, or -1 before ezNetCtlInit() bound.
 * @ghidraAddress NTSC-U/C: 0x0026cba8
 * @ghidraAddress PAL: 0x00276748
 */
int ezNetCtlUpInterface(int id);

/**
 * Take an interface down.
 *
 * The name is inferred.
 *
 * @param id The interface.
 * @return 0, or -1 before ezNetCtlInit() bound.
 * @ghidraAddress NTSC-U/C: 0x0026cc18
 * @ghidraAddress PAL: 0x002767b8
 */
int ezNetCtlDownInterface(int id);

/**
 * Report the buffer ezNetCtlLookUpName() fills with the address of the host.
 *
 * The name is inferred.
 *
 * @return The buffer.
 * @ghidraAddress NTSC-U/C: 0x0026cc88
 * @ghidraAddress PAL: 0x00276828
 */
char *ezNetCtlGetNameBuffer(void);

/**
 * Look up the address of a host, with up to three attempts.
 *
 * The server writes the address over the name in the buffer of ezNetCtlGetNameBuffer(), and an
 * empty buffer marks a failed attempt. The name is inferred.
 *
 * @param name The host name.
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x0026cc98
 * @ghidraAddress PAL: 0x00276838
 */
int ezNetCtlLookUpName(const char *name);

#ifdef __cplusplus
}
#endif

#endif
