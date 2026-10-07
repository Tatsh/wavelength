#ifndef MODEM_H
#define MODEM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Modem device registration of the resident modem library. A modem driver registers one record
 * per device. The library calls the record's operations with the record's private pointer and
 * learns of device changes through the record's event flag.
 */

/** Event bits a modem driver sets in the event flag of its record. */
enum ModemEvent {
    MODEM_EVENT_START_DONE = 0x001, /*!< A start request finished. */
    MODEM_EVENT_PLUG_OUT = 0x002,   /*!< The device was removed. */
    MODEM_EVENT_CONNECT = 0x010,    /*!< The line connected. */
    MODEM_EVENT_DISCONNECT = 0x020, /*!< The line disconnected. */
    MODEM_EVENT_RING = 0x040,       /*!< The line is ringing. */
    MODEM_EVENT_RECEIVE = 0x100,    /*!< Received bytes are waiting. */
    MODEM_EVENT_SEND = 0x200,       /*!< Space to send is free. */
};

/** Bus value a USB modem driver records. */
#define MODEM_BUS_USB 1

/** Registration record of a modem device. Reserved words are zero. */
typedef struct {
    unsigned int reserved0[2]; /*!< Zero. */
    const char *module;        /*!< Name of the driver module. */
    const char *vendor;        /*!< Vendor name. */
    const char *product;       /*!< Product name. */
    unsigned char bus;         /*!< Bus the device is on, such as #MODEM_BUS_USB. */
    unsigned char location[7]; /*!< Location of the device on the bus. */
    unsigned int reserved1[6]; /*!< Zero. */
    unsigned short reserved2;  /*!< Zero. */
    unsigned short reserved3;  /*!< Zero. */
    void *priv;                /*!< Driver pointer each operation receives. */
    int eventFlag;             /*!< Event flag of #ModemEvent bits, filled by the library. */
    int receiveCount;          /*!< Received bytes waiting to be read. */
    int sendSpace;             /*!< Bytes the driver can accept to send. */
    /** Start the device. Returns zero. */
    int (*start)(void *priv);
    /** Stop the device. Returns zero. */
    int (*stop)(void *priv);
    /** Read received bytes. Returns the byte count read. */
    int (*receive)(void *priv, void *buffer, int length);
    /** Send bytes. Returns the byte count accepted. */
    int (*send)(void *priv, const void *buffer, int length);
    /** Run a driver-defined command. Returns a nonnegative value, or a negative error code. */
    int (*control)(void *priv, unsigned int command, void *buffer, int size);
    unsigned int reserved4[4]; /*!< Zero. */
} ModemDeviceOps;

/**
 * Register a modem device.
 *
 * @param device Registration record. It must stay valid while registered.
 * @return A nonnegative value, or a negative error code.
 */
int ModemRegisterDevice(ModemDeviceOps *device);

/**
 * Unregister a modem device.
 *
 * @param device Record given to ModemRegisterDevice().
 * @return Zero, or a negative error code.
 */
int ModemUnregisterDevice(ModemDeviceOps *device);

#ifdef __cplusplus
}
#endif

#endif
