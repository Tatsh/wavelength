#ifndef USBD_H
#define USBD_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * USB device driver services of the resident usbd library. A driver registers its operations,
 * reads the descriptors of each device usbd offers it, opens pipes to the device's endpoints, and
 * queues transfers that complete through a callback.
 */

/** Descriptor types, from the USB specification. */
enum UsbDescriptorType {
    USB_DESCRIPTOR_TYPE_DEVICE = 1,        /*!< Device descriptor. */
    USB_DESCRIPTOR_TYPE_CONFIGURATION = 2, /*!< Configuration descriptor. */
    USB_DESCRIPTOR_TYPE_STRING = 3,        /*!< String descriptor. */
    USB_DESCRIPTOR_TYPE_INTERFACE = 4,     /*!< Interface descriptor. */
    USB_DESCRIPTOR_TYPE_ENDPOINT = 5,      /*!< Endpoint descriptor. */
};

/** Request types of a control transfer, from the USB specification. */
enum UsbRequestType {
    USB_REQUEST_TYPE_OUT_DEVICE = 0x00,    /*!< Standard request from the host to the device. */
    USB_REQUEST_TYPE_OUT_INTERFACE = 0x01, /*!< Standard request from the host to an interface. */
    USB_REQUEST_TYPE_IN_DEVICE = 0x80,     /*!< Standard request from the device to the host. */
};

/** Standard requests of a control transfer, from the USB specification. */
enum UsbRequest {
    USB_REQUEST_GET_DESCRIPTOR = 6,    /*!< Read a descriptor. */
    USB_REQUEST_SET_CONFIGURATION = 9, /*!< Select a configuration. */
    USB_REQUEST_SET_INTERFACE = 11,    /*!< Select an alternate setting of an interface. */
};

/** Setup packet of a control transfer. */
typedef struct {
    unsigned char requestType; /*!< One of #UsbRequestType. */
    unsigned char request;     /*!< One of #UsbRequest. */
    unsigned short value;      /*!< Request value. */
    unsigned short index;      /*!< Request index. */
    unsigned short length;     /*!< Byte count of the data stage. */
} UsbDeviceRequest;

/** Device descriptor. */
typedef struct {
    unsigned char length;             /*!< Descriptor size in bytes. */
    unsigned char descriptorType;     /*!< #USB_DESCRIPTOR_TYPE_DEVICE. */
    unsigned short usbVersion;        /*!< USB release in binary-coded decimal. */
    unsigned char deviceClass;        /*!< Class code. */
    unsigned char deviceSubClass;     /*!< Subclass code. */
    unsigned char deviceProtocol;     /*!< Protocol code. */
    unsigned char maxPacketSize;      /*!< Largest packet of the default pipe. */
    unsigned short vendorId;          /*!< Vendor identifier. */
    unsigned short productId;         /*!< Product identifier. */
    unsigned short deviceVersion;     /*!< Device release in binary-coded decimal. */
    unsigned char manufacturerIndex;  /*!< String index of the manufacturer name. */
    unsigned char productIndex;       /*!< String index of the product name. */
    unsigned char serialNumberIndex;  /*!< String index of the serial number. */
    unsigned char configurationCount; /*!< Number of configurations. */
} UsbDeviceDescriptor;

/** Configuration descriptor. */
typedef struct {
    unsigned char length;             /*!< Descriptor size in bytes. */
    unsigned char descriptorType;     /*!< #USB_DESCRIPTOR_TYPE_CONFIGURATION. */
    unsigned char totalLength[2];     /*!< Size of the configuration and its descriptors. */
    unsigned char interfaceCount;     /*!< Number of interfaces. */
    unsigned char configurationValue; /*!< Value that #USB_REQUEST_SET_CONFIGURATION selects. */
    unsigned char configurationIndex; /*!< String index of the configuration name. */
    unsigned char attributes;         /*!< Power attributes. */
    unsigned char maxPower;           /*!< Largest current draw in units of 2 mA. */
} UsbConfigurationDescriptor;

/** Interface descriptor. */
typedef struct {
    unsigned char length;            /*!< Descriptor size in bytes. */
    unsigned char descriptorType;    /*!< #USB_DESCRIPTOR_TYPE_INTERFACE. */
    unsigned char interfaceNumber;   /*!< Interface number. */
    unsigned char alternateSetting;  /*!< Alternate setting. */
    unsigned char endpointCount;     /*!< Number of endpoints, not counting the default pipe. */
    unsigned char interfaceClass;    /*!< Class code. */
    unsigned char interfaceSubClass; /*!< Subclass code. */
    unsigned char interfaceProtocol; /*!< Protocol code. */
    unsigned char interfaceIndex;    /*!< String index of the interface name. */
} UsbInterfaceDescriptor;

/** Endpoint descriptor. */
typedef struct {
    unsigned char length;           /*!< Descriptor size in bytes. */
    unsigned char descriptorType;   /*!< #USB_DESCRIPTOR_TYPE_ENDPOINT. */
    unsigned char endpointAddress;  /*!< Endpoint number and direction. */
    unsigned char attributes;       /*!< Transfer type. */
    unsigned char maxPacketSize[2]; /*!< Largest packet. */
    unsigned char interval;         /*!< Polling interval. */
} UsbEndpointDescriptor;

/**
 * Completion callback of a transfer.
 *
 * @param result Zero on success, otherwise an error code.
 * @param count Bytes transferred.
 * @param arg Argument given to sceUsbdTransferPipe().
 */
typedef void (*UsbdDoneCallback)(int result, int count, void *arg);

/** Operations of a device driver. The words after the callbacks are zero. */
typedef struct UsbdLddOps {
    struct UsbdLddOps *next;     /*!< Link that usbd fills. */
    struct UsbdLddOps *previous; /*!< Link that usbd fills. */
    const char *name;            /*!< Driver name. */
    /** Report whether the driver takes the device, nonzero for yes. */
    int (*probe)(int deviceId);
    /** Attach the driver to a device it took. Returns zero on success. */
    int (*connect)(int deviceId);
    /** Detach the driver from a device that was removed. Returns zero on success. */
    int (*disconnect)(int deviceId);
    unsigned int reserved[6]; /*!< Zero. */
} UsbdLddOps;

/**
 * Register a device driver. usbd probes every attached device with it before returning.
 *
 * @param driver Driver operations. They must stay valid while registered.
 * @return Zero, or an error code.
 */
int sceUsbdRegisterLdd(UsbdLddOps *driver);

/**
 * Unregister a device driver.
 *
 * @param driver Driver operations given to sceUsbdRegisterLdd().
 * @return Zero, or an error code.
 */
int sceUsbdUnregisterLdd(UsbdLddOps *driver);

/**
 * Find the next descriptor of a type in a device's descriptors.
 *
 * @param deviceId Device identifier.
 * @param previous Descriptor to search after, or null to search from the start.
 * @param type One of #UsbDescriptorType.
 * @return The descriptor, or null.
 */
void *sceUsbdScanStaticDescriptor(int deviceId, const void *previous, unsigned char type);

/**
 * Attach a driver pointer to a device.
 *
 * @param deviceId Device identifier.
 * @param data Pointer to attach.
 * @return Zero, or an error code.
 */
int sceUsbdSetPrivateData(int deviceId, void *data);

/**
 * Read the driver pointer attached to a device.
 *
 * @param deviceId Device identifier.
 * @return The pointer, or null.
 */
void *sceUsbdGetPrivateData(int deviceId);

/**
 * Open a pipe to an endpoint.
 *
 * @param deviceId Device identifier.
 * @param endpoint Endpoint descriptor, or null for the default control pipe.
 * @return The pipe identifier, or a negative error code.
 */
int sceUsbdOpenPipe(int deviceId, const UsbEndpointDescriptor *endpoint);

/**
 * Queue a transfer on a pipe.
 *
 * @param pipe Pipe identifier.
 * @param data Data buffer, or null when the transfer has no data stage.
 * @param length Byte count.
 * @param request Setup packet for the control pipe, otherwise null.
 * @param callback Completion callback, or null.
 * @param arg Argument the callback receives.
 * @return Zero when the transfer was queued, otherwise an error code.
 */
int sceUsbdTransferPipe(int pipe,
                        void *data,
                        int length,
                        UsbDeviceRequest *request,
                        UsbdDoneCallback callback,
                        void *arg);

/**
 * Report the port path of a device.
 *
 * @param deviceId Device identifier.
 * @param location Receives the seven-byte location of the device.
 * @return Zero, or an error code.
 */
int sceUsbdGetDeviceLocation(int deviceId, unsigned char *location);

#ifdef __cplusplus
}
#endif

#endif
