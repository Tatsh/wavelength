#ifndef INET_H
#define INET_H

#ifdef __cplusplus
extern "C" {
#endif

/** Name resolution and interface control of the inet library. */

/** Interface control that reads the interface flag bits into a 4-byte buffer. */
#define INET_CONTROL_GET_FLAGS 0x00000008u

/** Interface control of an Ethernet interface that takes no buffer and reports a status. */
#define INET_CONTROL_LINK_STATUS 0x80030000u

/** PPP interface control that reads the number being dialled. */
#define INET_CONTROL_PPP_DIAL_NUMBER 0x90008003u

/** PPP interface control that reads the latest phase or error message. */
#define INET_CONTROL_PPP_MESSAGE 0x90008038u

/** PPP interface control that reads a message while connected. */
#define INET_CONTROL_PPP_CONNECTION_MESSAGE 0x90008042u

/** PPP interface control that reads a message after a disconnection. */
#define INET_CONTROL_PPP_DISCONNECTION_MESSAGE 0x90008043u

/** Interface flag bits #INET_CONTROL_GET_FLAGS reads. */
enum InetInterfaceFlag {
    INET_INTERFACE_PPP = 0x0040,     /*!< A PPP interface. */
    INET_INTERFACE_PASSIVE = 0x0200, /*!< The eznetctl module does not wait for the interface. */
};

/** A network address. */
typedef struct sceInetAddress {
    unsigned char data[16]; /*!< Opaque to the eznet modules. */
} sceInetAddress;

/**
 * Resolve a host name. Export 4.
 *
 * @param flags Zero.
 * @param address Receives the address.
 * @param name Host name.
 * @param timeout Timeout. The eznetctl module passes 5000.
 * @param retries Retry count. The eznetctl module passes 1.
 * @return Zero, or a negative error code.
 */
int sceInetName2Address(
    int flags, sceInetAddress *address, const char *name, int timeout, int retries);

/**
 * Write the text form of an address. Export 5.
 *
 * @param text Receives the text.
 * @param size Size of @p text.
 * @param address Address.
 * @return Zero, or a negative error code.
 */
int sceInetAddress2String(char *text, int size, const sceInetAddress *address);

/**
 * Run a control operation on an interface. Export 25.
 *
 * @param interfaceId Interface identifier.
 * @param code Control code such as #INET_CONTROL_GET_FLAGS.
 * @param buffer Argument and result buffer, or null.
 * @param size Size of @p buffer.
 * @return The result of the operation, or a negative error code.
 */
int sceInetInterfaceControl(int interfaceId, unsigned int code, void *buffer, int size);

#ifdef __cplusplus
}
#endif

#endif
