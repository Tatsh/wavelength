#ifndef INET_H
#define INET_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Connections, name resolution, and interface control of the inet library. An export whose role
 * the calling modules do not reveal is named after its export number.
 */

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

/** Connection control that reads a sceInetConnectionInfo. */
#define INET_CONTROL_GET_CONNECTION_INFO 0x00000001u

/** Interface flag bits #INET_CONTROL_GET_FLAGS reads. */
enum InetInterfaceFlag {
    INET_INTERFACE_PPP = 0x0040,     /*!< A PPP interface. */
    INET_INTERFACE_PASSIVE = 0x0200, /*!< The eznet modules do not wait for the interface. */
};

/** Error codes of the library. */
enum sceInetError {
    sceINETE_TIMEOUT = -500,                    /*!< A timeout occurred. */
    sceINETE_ABORT = -501,                      /*!< An abort interrupted the call. */
    sceINETE_BUSY = -502,                       /*!< Initialisation is not complete. */
    sceINETE_LINK_DOWN = -503,                  /*!< The device or connection is not ready. */
    sceINETE_INSUFFICIENT_RESOURCES = -504,     /*!< Memory ran out. */
    sceINETE_LOCAL_SOCKET_UNSPECIFIED = -505,   /*!< The local port is invalid. */
    sceINETE_FOREIGN_SOCKET_UNSPECIFIED = -506, /*!< The remote address or port is invalid. */
    sceINETE_CONNECTION_ALREADY_EXISTS = -507,  /*!< The connection is already open. */
    sceINETE_CONNECTION_DOES_NOT_EXIST = -508,  /*!< The connection is not open. */
    sceINETE_CONNECTION_CLOSING = -509,         /*!< The connection is closing. */
    sceINETE_CONNECTION_RESET = -510,           /*!< The connection was reset. */
    sceINETE_CONNECTION_REFUSED = -511,         /*!< The connection was refused. */
    sceINETE_INVALID_ARGUMENT = -512,           /*!< An argument is invalid. */
    sceINETE_INVALID_CALL = -513,               /*!< The call is invalid. */
    sceINETE_NO_ROUTE = -514,                   /*!< The destination has no route. */
};

/** Connection types. */
enum sceInetType {
    sceINETT_DGRAM = 0,   /*!< UDP. */
    sceINETT_CONNECT = 1, /*!< TCP, opened actively. */
    sceINETT_LISTEN = 2,  /*!< TCP, opened passively. */
    sceINETT_RAW = 3,     /*!< Raw IP. */
};

/** Values of sceInetConnectionInfo::proto. */
enum sceInetProtocol {
    sceINETI_PROTO_TCP = 1, /*!< TCP. */
    sceINETI_PROTO_UDP = 2, /*!< UDP. */
    sceINETI_PROTO_IP = 3,  /*!< Raw IP. */
};

/** Values of sceInetConnectionInfo::state. */
enum sceInetConnectionState {
    sceINETI_STATE_UNKNOWN = 0,      /*!< Unknown. */
    sceINETI_STATE_CLOSED = 1,       /*!< Closed. */
    sceINETI_STATE_CREATED = 2,      /*!< Created. */
    sceINETI_STATE_OPENED = 3,       /*!< Opened. */
    sceINETI_STATE_LISTEN = 4,       /*!< TCP listening. */
    sceINETI_STATE_SYN_SENT = 5,     /*!< TCP synchronisation sent. */
    sceINETI_STATE_SYN_RECEIVED = 6, /*!< TCP synchronisation received. */
    sceINETI_STATE_ESTABLISHED = 7,  /*!< TCP established. */
    sceINETI_STATE_FIN_WAIT_1 = 8,   /*!< TCP first finish wait. */
    sceINETI_STATE_FIN_WAIT_2 = 9,   /*!< TCP second finish wait. */
    sceINETI_STATE_CLOSE_WAIT = 10,  /*!< TCP close wait. */
    sceINETI_STATE_CLOSING = 11,     /*!< TCP closing. */
    sceINETI_STATE_LAST_ACK = 12,    /*!< TCP last acknowledgement. */
    sceINETI_STATE_TIME_WAIT = 13,   /*!< TCP time wait. */
};

/** A network address. */
typedef struct sceInetAddress {
    int reserved;           /*!< Always zero. */
    unsigned char data[12]; /*!< Address bytes. An IPv4 address is the first four. */
} sceInetAddress;

/** State of one connection, as #INET_CONTROL_GET_CONNECTION_INFO reads it. */
typedef struct sceInetConnectionInfo {
    int cid;                      /*!< Connection identifier. */
    int proto;                    /*!< A #sceInetProtocol. */
    int recvQueueLength;          /*!< Bytes in the receive buffer. */
    int sendQueueLength;          /*!< Bytes in the send buffer. */
    sceInetAddress localAddress;  /*!< Local address. */
    int localPort;                /*!< Local port. */
    sceInetAddress remoteAddress; /*!< Remote address. */
    int remotePort;               /*!< Remote port. */
    int state;                    /*!< A #sceInetConnectionState. */
    unsigned int reserved[4];     /* +0x3c */
} sceInetConnectionInfo;

/**
 * Resolve a host name. Export 4.
 *
 * @param flags Zero.
 * @param address Receives the address.
 * @param name Host name.
 * @param timeout Timeout. The eznetctl module passes 5000.
 * @param retries Retry count. The eznetctl module passes 1.
 * @param option A further argument the libnetb module forwards from the EE.
 * @return Zero, or a negative error code.
 */
int sceInetName2Address(
    int flags, sceInetAddress *address, const char *name, int timeout, int retries, int option);

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
 * Export 6. The libnetb module passes a parameter block from the EE.
 *
 * @param param Parameter block.
 * @return A connection identifier, or a negative error code.
 */
int inet_6(void *param);

/**
 * Export 7.
 *
 * @param cid Connection identifier.
 * @param value An argument from the EE.
 * @return A result.
 */
int inet_7(int cid, int value);

/**
 * Export 8. The libnetb module calls it to close a connection.
 *
 * @param cid Connection identifier.
 * @param value An argument from the EE.
 * @return A result.
 */
int inet_8(int cid, int value);

/**
 * Receive from a connection. Export 9.
 *
 * @param cid Connection identifier.
 * @param data Receives the data.
 * @param count Size of @p data.
 * @param flags Flags in, and the flags of the receive out.
 * @param timeout Timeout, or -1 to wait forever.
 * @return The byte count received, or a negative #sceInetError.
 */
int sceInetRecv(int cid, void *data, int count, int *flags, int timeout);

/**
 * Send on a connection. Export 10.
 *
 * @param cid Connection identifier.
 * @param data Data.
 * @param count Byte count.
 * @param flags Flags in, and the flags of the send out.
 * @param timeout Timeout.
 * @return The byte count sent, or a negative #sceInetError.
 */
int sceInetSend(int cid, const void *data, int count, int *flags, int timeout);

/**
 * Export 11.
 *
 * @param cid Connection identifier.
 * @param value An argument from the EE.
 * @return A result.
 */
int inet_11(int cid, int value);

/**
 * Receive a datagram and its sender. Export 12.
 *
 * @param cid Connection identifier.
 * @param data Receives the data.
 * @param count Size of @p data.
 * @param flags Flags in, and the flags of the receive out.
 * @param address Receives the sender address.
 * @param port Receives the sender port.
 * @param timeout Timeout, or -1 to wait forever.
 * @return The byte count received, or a negative #sceInetError.
 */
int sceInetRecvFrom(
    int cid, void *data, int count, int *flags, sceInetAddress *address, int *port, int timeout);

/**
 * Send a datagram. Export 13.
 *
 * @param cid Connection identifier.
 * @param data Data.
 * @param count Byte count.
 * @param flags Flags in, and the flags of the send out.
 * @param address Destination address.
 * @param port Destination port.
 * @param timeout Timeout.
 * @return The byte count sent, or a negative #sceInetError.
 */
int sceInetSendTo(int cid,
                  const void *data,
                  int count,
                  int *flags,
                  const sceInetAddress *address,
                  int port,
                  int timeout);

/**
 * Export 14. The libnetb module forwards every argument from the EE.
 *
 * @param argument0 First argument.
 * @param buffer1 Buffer in the request.
 * @param argument2 Third argument.
 * @param buffer3 Buffer in the request.
 * @param argument4 Fifth argument.
 * @param argument5 Sixth argument.
 * @param argument6 Seventh argument.
 * @return A result.
 */
int inet_14(int argument0,
            void *buffer1,
            int argument2,
            void *buffer3,
            int argument4,
            int argument5,
            int argument6);

/**
 * Run a control operation on a connection. Export 15.
 *
 * @param cid Connection identifier.
 * @param code Control code such as #INET_CONTROL_GET_CONNECTION_INFO.
 * @param buffer Argument and result buffer, or null.
 * @param size Size of @p buffer.
 * @return The result of the operation, or a negative #sceInetError.
 */
int sceInetControl(int cid, unsigned int code, void *buffer, int size);

/**
 * Export 16. The libnetb module forwards every argument from the EE.
 *
 * @param buffer Result buffer.
 * @param argument1 Second argument.
 * @param argument2 Third argument.
 * @return A result.
 */
int inet_16(void *buffer, int argument1, int argument2);

/**
 * Export 24. The libnetb module forwards every argument from the EE.
 *
 * @param buffer Argument and result buffer.
 * @param value Second argument.
 * @return A result.
 */
int inet_24(void *buffer, int value);

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

/**
 * Export 27. The libnetb module forwards every argument from the EE.
 *
 * @param buffer Argument and result buffer.
 * @param value Second argument.
 * @return A result.
 */
int inet_27(void *buffer, int value);

/**
 * Export 30. The libnetb module forwards every argument from the EE.
 *
 * @param buffer Argument and result buffer.
 * @param value Second argument.
 * @return A result.
 */
int inet_30(void *buffer, int value);

/**
 * Export 36.
 *
 * @param value An argument from the EE.
 * @return A result.
 */
int inet_36(int value);

/**
 * Export 38. The libnetb module forwards every argument from the EE.
 *
 * @param buffer Result buffer.
 * @param argument1 Second argument.
 * @param argument2 Third argument.
 * @return A result.
 */
int inet_38(void *buffer, int argument1, int argument2);

/**
 * Export 41.
 *
 * @return A result.
 */
int inet_41(void);

#ifdef __cplusplus
}
#endif

#endif
