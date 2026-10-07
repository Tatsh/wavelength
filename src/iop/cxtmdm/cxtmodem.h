#pragma once

#include <kernel.h>
#include <modem.h>
#include <usbd.h>

/** State of an endpoint pipe. */
struct UsbEndpoint {
    int pipe;     /*!< Pipe identifier. */
    int pending;  /*!< Transfers queued and not completed. */
    bool blocked; /*!< Never set by the module. A nonzero value would stop new transfers. */
};

/** One register write in a packet on the register endpoint. */
struct RegisterWrite {
    unsigned char command; /*!< CxtModem::kRegisterWriteCommand. */
    unsigned char index;   /*!< Register index. */
    unsigned char value;   /*!< Register value. */
};

/** Packet on the control endpoint. */
struct ControlPacket {
    unsigned char reserved[2]; /*!< Zero. */
    unsigned char mode;        /*!< Mode bits. */
    unsigned char lines;       /*!< Line bits such as CxtModem::kLineActive. */
};

/**
 * A Conexant USB modem the driver attached to. The modem library drives it through the record in
 * mModem. The device exposes UART-style registers written through the register endpoint, takes
 * bytes to send on the bulk out endpoint, and reports received bytes and the modem status on the
 * bulk in endpoint.
 *
 * The module was built without RTTI. The name is inferred from the module's routines.
 */
class CxtModem {
public:
    /** Load modes the `lmode` argument selects. */
    enum LoadMode {
        kLoadModeNormal = 0, /*!< Stay loaded. */
        kLoadModeAuto = 1,   /*!< Stay loaded only when a modem is attached. */
        kLoadModeTest = 2,   /*!< Report whether a modem is attached, then unload. */
    };

    /** Value written in each RegisterWrite. */
    static constexpr unsigned char kRegisterWriteCommand = 0x40;

    /** Line bits of the control packet. */
    enum LineBit {
        kLineActive = 0x01,    /*!< Set while the modem is started. */
        kLineConnected = 0x02, /*!< Set while the line is connected. */
        kLineReserved = 0x04,  /*!< Only ever cleared. */
    };

    /**
     * Report whether the driver takes a device. Records that a modem is attached.
     *
     * @param deviceId Device identifier.
     * @return Nonzero to take the device. A test load does not take it.
     * @ghidraAddress NTSC-U/C: 0x00001574
     */
    static int Probe(int deviceId);

    /**
     * Attach to a device. Open its pipes, select its configuration, read its names, register the
     * modem, and start the firmware download thread.
     *
     * @param deviceId Device identifier.
     * @return Zero, or -1 on failure.
     * @ghidraAddress NTSC-U/C: 0x00001610
     */
    static int Connect(int deviceId);

    /**
     * Detach from a removed device. An active modem only reports the removal and is released by
     * Stop().
     *
     * @param deviceId Device identifier.
     * @return Zero, or -1 when the device has no modem.
     * @ghidraAddress NTSC-U/C: 0x00001b34
     */
    static int Disconnect(int deviceId);

    /** Load mode from the module arguments. */
    static LoadMode sLoadMode;

    /** Whether a modem is attached, or for a normal load, whether to stay loaded. */
    static bool sDeviceFound;

    /** Size of sDialString. */
    static constexpr int kDialStringSize = 1024;

    /** Value of the `dial` module argument. */
    static char sDialString[kDialStringSize];

private:
    /** Modem states. */
    enum State {
        kStateIdle = 0,          /*!< Not started. */
        kStateStarting = 1,      /*!< Started, waiting for the firmware. */
        kStateReady = 2,         /*!< Started, with no carrier. */
        kStateConnected = 3,     /*!< Started, with a carrier. */
        kStateDisconnecting = 4, /*!< Never entered by the module. */
        kStateUnplugged = 5,     /*!< The device was removed while active. */
    };

    /** Number of registers. */
    static constexpr int kRegisterCount = 8;

    /** Number of endpoint records. */
    static constexpr int kEndpointCount = 16;

    /** Size of the transmit buffer. */
    static constexpr int kTransmitBufferSize = 0x400;

    /** Size of the transmit chunk buffer. */
    static constexpr int kTransmitChunkSize = 16;

    /** Size of the receive buffer. */
    static constexpr int kReceiveBufferSize = 0x1000;

    /** Number of staged register writes. */
    static constexpr int kStagedWriteCount = 16;

    /** Size of the bulk in buffer. */
    static constexpr int kInputBufferSize = 32;

    /** Size of mVendorName. */
    static constexpr int kVendorNameSize = 31;

    /** Size of mProductName. */
    static constexpr int kProductNameSize = 33;

    /**
     * Set event bits in the flag of the modem record. Unless the bits include
     * #MODEM_EVENT_SEND, a description of the bits is built and never printed.
     *
     * @param events #ModemEvent bits.
     * @ghidraAddress NTSC-U/C: 0x00000000
     */
    void NotifyModem(unsigned int events);

    /**
     * Set event bits in the module's event flag. A description of the bits is built and never
     * printed.
     *
     * @param events Event bits.
     * @ghidraAddress NTSC-U/C: 0x00000138
     */
    void Signal(unsigned int events);

    /**
     * Send bytes for the modem library once the modem is started and its firmware is loaded.
     * When the transmit buffer stays full after a short wait, the rest is not sent.
     *
     * @param modem The modem.
     * @param data Bytes.
     * @param length Byte count.
     * @return Bytes accepted.
     * @ghidraAddress NTSC-U/C: 0x00000208
     */
    static int Send(void *modem, const void *data, int length);

    /**
     * Send bytes, waiting while the transmit buffer is full.
     *
     * @param data Bytes.
     * @param length Byte count.
     * @return Bytes sent.
     * @ghidraAddress NTSC-U/C: 0x00000338
     */
    int Write(const char *data, int length);

    /**
     * Run a command for the modem library.
     *
     * @param modem The modem.
     * @param command Command code.
     * @param buffer Argument or result.
     * @param size Size of the buffer.
     * @return Zero, the dial string length for its command, or a negative error code.
     * @ghidraAddress NTSC-U/C: 0x00000444
     */
    static int Control(void *modem, unsigned int command, void *buffer, int size);

    /**
     * Report the modem status byte from the last bulk in packet.
     *
     * @return The status.
     * @ghidraAddress NTSC-U/C: 0x000006c8
     */
    unsigned char ModemStatus() const;

    /**
     * Start the modem for the modem library, waiting up to ten seconds for the firmware.
     *
     * @param modem The modem.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x000006d4
     */
    static int Start(void *modem);

    /**
     * Report the receive buffer to the modem library after received bytes have waited.
     *
     * @param modem The modem.
     * @return Zero, to stop the alarm.
     * @ghidraAddress NTSC-U/C: 0x00000854
     */
    static unsigned int AlarmHandler(void *modem);

    /**
     * Report the events received and not yet consumed.
     *
     * @return Event bits.
     * @ghidraAddress NTSC-U/C: 0x000008a8
     */
    unsigned int PendingEvents() const;

    /**
     * Wait for and consume event bits.
     *
     * @param mask Bits to wait for.
     * @return The bits of mask that were set, or -1 when the wait failed.
     * @ghidraAddress NTSC-U/C: 0x000008b4
     */
    int WaitEvents(unsigned int mask);

    /**
     * Thread that polls the modem status each second for carrier and ring changes.
     *
     * @param modem The modem.
     * @ghidraAddress NTSC-U/C: 0x0000092c
     */
    static void StatusThread(void *modem);

    /**
     * Thread that runs the modem state machine on the module's events.
     *
     * @param modem The modem.
     * @ghidraAddress NTSC-U/C: 0x00000a2c
     */
    static void EventThread(void *modem);

    /**
     * Thread that identifies the modem and downloads its firmware, then marks the modem ready.
     *
     * @param modem The modem.
     * @ghidraAddress NTSC-U/C: 0x00000c00
     */
    static void DownloadThread(void *modem);

    /**
     * Allocate a modem and start its event and status threads.
     *
     * @return The modem, or null.
     * @ghidraAddress NTSC-U/C: 0x00000f40
     */
    static CxtModem *Create();

    /**
     * Stop the threads, delete the kernel objects, and free the modem.
     *
     * @ghidraAddress NTSC-U/C: 0x000010a4
     */
    void Destroy();

    /**
     * Stop the modem for the modem library. An unplugged modem is unregistered and freed.
     *
     * @param modem The modem.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00001130
     */
    static int Stop(void *modem);

    /**
     * Raise or drop the request-to-send bit as the receive buffer drains or fills.
     *
     * @ghidraAddress NTSC-U/C: 0x00001224
     */
    void UpdateFlowControl();

    /**
     * Read received bytes for the modem library once the firmware is loaded.
     *
     * @param modem The modem.
     * @param data Receives the bytes.
     * @param length Largest byte count.
     * @return Bytes read.
     * @ghidraAddress NTSC-U/C: 0x000012b4
     */
    static int Receive(void *modem, void *data, int length);

    /**
     * Read received bytes.
     *
     * @param data Receives the bytes.
     * @param length Largest byte count.
     * @return Bytes read.
     * @ghidraAddress NTSC-U/C: 0x0000139c
     */
    int Read(char *data, int length);

    /**
     * Completion of the interface selection. Marks the device configured and opens the line.
     *
     * @param result Transfer result.
     * @param count Bytes transferred.
     * @param modem The modem.
     * @ghidraAddress NTSC-U/C: 0x0000147c
     */
    static void SetInterfaceDone(int result, int count, void *modem);

    /**
     * Completion of the configuration selection. Selects the interface.
     *
     * @param result Transfer result.
     * @param count Bytes transferred.
     * @param modem The modem.
     * @ghidraAddress NTSC-U/C: 0x000014d4
     */
    static void SetConfigurationDone(int result, int count, void *modem);

    /**
     * Queue a register write. Indices of 8 or more are ignored.
     *
     * @param index Register index.
     * @param value Register value.
     * @ghidraAddress NTSC-U/C: 0x00001dd8
     */
    void SetRegister(unsigned int index, unsigned char value);

    /**
     * Send the transmit buffer in chunks of up to 14 bytes. A call made while another runs makes
     * the running call loop again.
     *
     * @param unused Not read.
     * @ghidraAddress NTSC-U/C: 0x00001e14
     */
    void FlushTransmit(int unused);

    /**
     * Take the next received byte. Emptying the buffer rewinds it.
     *
     * @return The byte, or zero when the buffer is empty.
     * @ghidraAddress NTSC-U/C: 0x00001f90
     */
    char ReadByte();

    /**
     * Report whether the transmit buffer is full.
     *
     * @return True when full.
     * @ghidraAddress NTSC-U/C: 0x00001fe8
     */
    bool IsTransmitFull() const;

    /**
     * Append a byte to the transmit buffer unless it is full.
     *
     * @param value The byte.
     * @ghidraAddress NTSC-U/C: 0x00001ffc
     */
    void PutByte(char value);

    /**
     * Completion of a register packet. Sends the next one.
     *
     * @param result Transfer result.
     * @param count Bytes transferred.
     * @param modem The modem.
     * @ghidraAddress NTSC-U/C: 0x00002038
     */
    static void RegisterTransferDone(int result, int count, void *modem);

    /**
     * Completion of a bulk out chunk. Sends the next chunk and reports free space.
     *
     * @param result Transfer result.
     * @param count Bytes transferred.
     * @param modem The modem.
     * @ghidraAddress NTSC-U/C: 0x00002090
     */
    static void TransmitDone(int result, int count, void *modem);

    /**
     * Completion of a bulk in packet. Takes its bytes and queues the next packet. A failure stops
     * receiving.
     *
     * @param result Transfer result.
     * @param count Bytes transferred.
     * @param modem The modem.
     * @ghidraAddress NTSC-U/C: 0x00002124
     */
    static void ReceiveDone(int result, int count, void *modem);

    /**
     * Store the received bytes of a bulk in packet and record its status byte. The modem library
     * learns of them at once when the buffer passes 1024 bytes, otherwise after the alarm.
     *
     * @param data The packet, a status byte followed by pairs of a flag byte and a data byte.
     * @param pairs Number of pairs.
     * @ghidraAddress NTSC-U/C: 0x000021e8
     */
    void ParseInput(const unsigned char *data, int pairs);

    /**
     * Report whether received bytes are waiting.
     *
     * @return True when the receive buffer is not empty.
     * @ghidraAddress NTSC-U/C: 0x0000239c
     */
    bool HasInput() const;

    /**
     * Queue a transfer on an endpoint once the device is configured.
     *
     * @param endpoint The endpoint.
     * @param data Data buffer.
     * @param length Byte count.
     * @param callback Completion callback. It receives the modem.
     * @return The usbd result, or 306 when the device is not configured.
     * @ghidraAddress NTSC-U/C: 0x000023b4
     */
    int Transfer(UsbEndpoint *endpoint, void *data, int length, UsbdDoneCallback callback);

    /**
     * Queue a bulk in transfer unless one is pending.
     *
     * @ghidraAddress NTSC-U/C: 0x0000241c
     */
    void StartReceive();

    /**
     * Send the queued register writes, up to 16 per packet.
     *
     * @ghidraAddress NTSC-U/C: 0x0000247c
     */
    void FlushRegisters();

    /**
     * Does nothing. Runs after each register packet.
     *
     * @ghidraAddress NTSC-U/C: 0x000025b8
     */
    void RegistersWritten();

    /**
     * Completion of a control packet. Sends the packet again when it changed.
     *
     * @param result Transfer result.
     * @param count Bytes transferred.
     * @param modem The modem.
     * @ghidraAddress NTSC-U/C: 0x000025c0
     */
    static void ControlPacketDone(int result, int count, void *modem);

    /**
     * Send the control packet when it changed and none is pending.
     *
     * @ghidraAddress NTSC-U/C: 0x00002604
     */
    void SendControlPacket();

    /**
     * Change the mode bits of the control packet and send it.
     *
     * @param set Bits to set.
     * @param clear Bits to clear first.
     * @ghidraAddress NTSC-U/C: 0x00002668
     */
    void UpdateMode(unsigned char set, unsigned char clear);

    /**
     * Change the line bits of the control packet and send it.
     *
     * @param set Bits to set.
     * @param clear Bits to clear first.
     * @ghidraAddress NTSC-U/C: 0x000026a0
     */
    void UpdateLines(unsigned char set, unsigned char clear);

    /**
     * Close or open the line. Opening programs the registers and starts receiving.
     *
     * @param phase 1 to close or 2 to open. The module never requests 1.
     * @ghidraAddress NTSC-U/C: 0x000026d8
     */
    void Configure(int phase);

    /** Program the serial port for 8 data bits at the fastest rate, pausing between steps. */
    void ProgramSerialPort();

    int mDeviceId;                                      /*!< usbd device identifier. */
    unsigned int mReserved0;                            // +0x04
    unsigned char mReserved1;                           // +0x08
    bool mConfigured;                                   /*!< The interface is selected. */
    bool mRegistersDirty;                               /*!< Register writes wait to be staged. */
    bool mInterfaceSelected;                            /*!< Set with mConfigured and never read. */
    unsigned int mReserved2;                            // +0x0c
    int mControlPipe;                                   /*!< Default control pipe. */
    unsigned int mReserved3;                            // +0x14
    UsbEndpoint mEndpoints[kEndpointCount];             /*!< Pipes of the interface's endpoints. */
    unsigned char mTransmitBuffer[kTransmitBufferSize]; /*!< Bytes to send. */
    unsigned char mTransmitChunk[kTransmitChunkSize];   /*!< Chunk on the bulk out endpoint. */
    int mTransmitLength;                                /*!< Bytes in mTransmitBuffer. */
    unsigned char mReceiveBuffer[kReceiveBufferSize];   /*!< Received bytes. */
    unsigned char mRegisterValues[kRegisterCount];      /*!< Last value written to each register. */
    unsigned char mRegisterPending[kRegisterCount];   /*!< Nonzero for registers not yet staged. */
    ControlPacket mControlPacket;                     /*!< Packet on the control endpoint. */
    bool mControlPacketPending;                       /*!< The control packet changed. */
    unsigned char mRegisterWriteCommand;              /*!< #kRegisterWriteCommand. */
    int mReceiveRead;                                 /*!< Next byte of mReceiveBuffer to read. */
    int mReceiveWrite;                                /*!< End of the bytes in mReceiveBuffer. */
    RegisterWrite mRegisterPacket[kStagedWriteCount]; /*!< Packet on the register endpoint. */
    RegisterWrite mStagedWrites[kStagedWriteCount];   /*!< Writes staged for the next packet. */
    int mStagedCount;                                 /*!< Writes in mStagedWrites. */
    unsigned int mReserved4;                          // +0x1570
    unsigned char mInputBuffer[kInputBufferSize];     /*!< Packet on the bulk in endpoint. */
    unsigned int mFlags;                              /*!< Bit 1 is set while receiving. */
    unsigned int mReserved5[2];                       // +0x1598
    int mTransmitDepth;                  /*!< Nesting of FlushTransmit(), -1 at rest. */
    unsigned int mReserved6[2];          // +0x15a4
    unsigned char mModemStatus;          /*!< Status byte of the last bulk in packet. */
    unsigned int mReserved7[2];          // +0x15b0
    int mTransmitSema;                   /*!< Signalled when mTransmitBuffer empties. */
    unsigned int mReserved8[3];          // +0x15bc
    SysClock mAlarmInterval;             /*!< Delay of the receive alarm. */
    unsigned char mClearedOnConfigure;   /*!< Cleared by Configure() and never read. */
    ModemDeviceOps mModem;               /*!< Registration with the modem library. */
    int mReceivedBytes;                  /*!< Received byte statistic. */
    int mSentBytes;                      /*!< Sent byte statistic. */
    unsigned int mPendingEvents;         /*!< Events received and not consumed. */
    int mEventFlag;                      /*!< The module's event flag. */
    int mEventThread;                    /*!< Thread running EventThread(). */
    int mStatusThread;                   /*!< Thread running StatusThread(). */
    int mDownloadThread;                 /*!< Thread running DownloadThread(). */
    int mAlarmActive;                    /*!< 1 while the receive alarm is set. */
    int mState;                          /*!< A #State. */
    int mReady;                          /*!< 1 once the firmware is loaded. */
    int mCarrier;                        /*!< 1 while the carrier is present. */
    int mRing;                           /*!< 1 while the line rings. */
    int mStarted;                        /*!< 1 while the modem is started. */
    char mVendorName[kVendorNameSize];   /*!< Vendor name from the device. */
    char mProductName[kProductNameSize]; /*!< Product name from the device. */
    int mReceiveNotifications;           /*!< Receive reports to the modem library. */
};
