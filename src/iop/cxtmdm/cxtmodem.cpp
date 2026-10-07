#include "cxtmdm/cxtmodem.h"

#include <stdio.h>

#include <kernel.h>
#include <modem.h>
#include <netdev.h>
#include <sysclib.h>
#include <usbd.h>

namespace {

constexpr unsigned short kConexantVendorId = 0x0572;
constexpr unsigned short kModemProductId = 0x1232;
constexpr unsigned short kModemProductIdAlternate = 0x1272;
constexpr int kInterfaceCount = 1;
constexpr int kInterfaceEndpointCount = 8;

// Endpoints by their order in the interface.
enum EndpointIndex {
    kRegisterEndpoint = 0,
    kBulkOutEndpoint = 4,
    kBulkInEndpoint = 5,
    kControlEndpoint = 6,
};

// UART-style registers of the device and the values the module writes.
enum Register {
    kRegisterDivisorLow = 0,
    kRegisterDivisorHigh = 1,
    kRegisterFifoControl = 2,
    kRegisterLineControl = 3,
    kRegisterModemControl = 4,
};
constexpr unsigned char kLineControlDivisorLatch = 0x80;
constexpr unsigned char kLineControlEightBits = 0x03;
constexpr unsigned char kDivisorLow = 1;
constexpr unsigned char kDivisorHigh = 0;
constexpr unsigned char kFifoEnable = 0xe1;
constexpr unsigned char kFifoReset = 0x03;
constexpr unsigned char kModemControlOff = 0x00;
constexpr unsigned char kModemControlOn = 0x03;
constexpr unsigned char kModemControlRequestToSend = 0x02;
constexpr unsigned char kRegisterPendingMark = 0xff;

// Bits of the modem status byte.
constexpr unsigned char kStatusRing = 0x40;
constexpr unsigned char kStatusCarrier = 0x80;

// Mode bits of the control packet.
constexpr unsigned char kModeReset = 0x08;
constexpr unsigned char kModeOpen = 0xc9;

constexpr unsigned int kFlagReceiving = 0x02;

// Bits of the module's event flag.
enum Event : unsigned int {
    kEventStart = 0x001,
    kEventStop = 0x002,
    kEventState = 0x008,
    kEventBreak = 0x400,
    kEventAll = 0x7ff,
};

enum ConfigurePhase {
    kConfigureClose = 1,
    kConfigureOpen = 2,
};

// Commands of the modem library's control operation.
constexpr unsigned int kControlGetThreadPriority = 0xc0000000;
constexpr unsigned int kControlGetZero = 0xc0000100;
constexpr unsigned int kControlClearReceive = 0xc0000110;
constexpr unsigned int kControlClearTransmit = 0xc0000111;
constexpr unsigned int kControlGetDialString = 0xc0000200;
constexpr unsigned int kControlGetReceivedBytes = 0xc0010000;
constexpr unsigned int kControlGetSentBytes = 0xc0010001;
constexpr unsigned int kControlSetThreadPriority = 0xc1000000;
constexpr int kControlBadSize = -0x200;
constexpr int kControlBadCommand = -0x201;
constexpr int kControlValueSize = 4;
constexpr int kMinThreadPriority = 9;
constexpr int kMaxThreadPriority = 123;

constexpr int kThreadStackSize = 0x1000;
constexpr int kAllocationMode = 0;

constexpr int kTransferNotConfigured = 306;
constexpr int kTransmitChunkLength = 14;
constexpr int kInputTransferLength = 31;
constexpr unsigned char kInputByteValid = 0x01;
constexpr unsigned int kMaxInputPairs = 16;
constexpr int kReceiveNotifyThreshold = 0x400;
constexpr int kFlowResumeThreshold = 0x400;
constexpr int kFlowPauseThreshold = 0xc00;
constexpr int kSendNotifyThreshold = 0x100;

// Delays in microseconds.
constexpr int kStringRequestDelay = 100;
constexpr int kLineDelay = 10000;
constexpr int kTransmitRetryDelay = 10000;
constexpr unsigned int kAlarmDelay = 10000;
constexpr int kRegisterSettleDelay = 100000;
constexpr int kStartPollDelay = 500000;
constexpr int kStatusPollDelay = 1000000;
constexpr int kResponseDelay = 1000000;
constexpr int kPowerUpDelay = 2000000;
constexpr int kPromptRetryDelay = 5000000;

constexpr int kStartPollCount = 20;
constexpr int kIdentifyAttempts = 100000;
constexpr int kPromptAttempts = 100;
constexpr int kResponseLength = 50;
constexpr int kResponseBufferSize = 152;
constexpr int kModelNameLength = 9;
constexpr int kPromptLength = 2;

// Firmware download progress.
enum DownloadStep {
    kDownloadNone = 0,
    kDownloadModelFound = 1,
    kDownloadPromptFound = 2,
};

constexpr int kNameBufferSize = 64;
constexpr int kNameLength = 32;
constexpr int kStringRequestLength = 61;
constexpr unsigned char kFirstPrintable = 0x20;
constexpr unsigned char kPrintableCount = 0x5f;
constexpr int kDescriptionSize = 104;

constexpr char kModemModuleName[] = "cxtmdm";
constexpr char kErrorFormat[] = "cxtmodem: %s -> 0x%x\n";
constexpr char kDefaultVendorName[] = "Conexant";
constexpr char kDefaultProductName[] = "SMARTSCM";
constexpr char kIdentifyCommand[] = "ati3\r";
constexpr char kDownloadCommand[] = "at**\r";
constexpr char kModelName[] = "P2109-V90";
constexpr char kDownloadPrompt[] = "..";

// NTSC-U/C: 0x000034e8
int g_threadPriority = 28;

// NTSC-U/C: 0x00003530
char g_transferByte = 'X';

// Firmware the modem receives after the download command.
// NTSC-U/C: 0x00002bc0
const char kFirmware[] = "S3100000A000B20269A9008DBC044C0EE101\r\n"
                         "S31500009DA560606B606060606B6B6060606060606087\r\n"
                         "S31500009DB56B60606B60606060606060606060606082\r\n"
                         "S31500009DC56060606060606060606060606060606088\r\n"
                         "S31500009DD5606060606060606B6B6060606060606062\r\n"
                         "S31500009DE560606B606060606060606060606B606052\r\n"
                         "S30700009DF5000066\r\n"
                         "S31500009EB00000E500000000D02000000000000000C7\r\n"
                         "S31500009EC0DD000003000000000000000000000000AC\r\n"
                         "S31500009ED0000000000000000000000000000000007C\r\n"
                         "S31500009EE00000000000000018DA000000000000007A\r\n"
                         "S31500009EF00000E200000000000000000000A60000D4\r\n"
                         "S30600009F00005A\r\n"
                         "S31500009F010917FF4C11AD6487D00CA21820445A29B9\r\n"
                         "S31500009F111F09804CF558604C83863D534C562BA93E\r\n"
                         "S31500009F216B8DA79DA0FFA204C8B97B04C90DF03EA5\r\n"
                         "S31500009F31DD1B9FD0F1CA10F0C8B97B04C934D00427\r\n"
                         "S31500009F41A9388027C935D004A939801FC936D0D68A\r\n"
                         "S31500009F51206A9FA200E8C8B97B04C90DD0F7B97B76\r\n"
                         "S31500009F6104997C0488CAD0F6C8A931997B04A0FF5C\r\n"
                         "S31500009F71A203C8B97B04C90DF026DDA29FD0F1CAA0\r\n"
                         "S31500009F8110F0C8B97B04997704C90DD0F5ADCE049C\r\n"
                         "S31500009F912901F009B28C40B200414CA3254CA02501\r\n"
                         "S31500009FA1605652542DE25D0110244F6321AD3F8767\r\n"
                         "S31500009FB1C9B1F015AD4D87C900F0132073E720D75D\r\n"
                         "S31500009FC19FE2600110084CAE77A9008D4D8760A213\r\n"
                         "S31500009FD103A9004C9BE44C019E4CF99DD20839061D\r\n"
                         "S31500009FE1604C7886A900CDA102D00AA973CD0D01D6\r\n"
                         "S31000009FF1B0038D0D01A9608DA79D60D7\r\n"
                         "S31500009DF97F4C04D210600160AD43878540AD44872E\r\n"
                         "S31500009E09854138A540E98C8540A541E9008541A5EC\r\n"
                         "S31500009E1940ED52878540A541ED53878541A54148C7\r\n"
                         "S31500009E29A5408542AD47878540AD48878541208B4A\r\n"
                         "S31500009E3969B26444B20045B20043205022A5408D60\r\n"
                         "S31500009E494D87A5418D4E87688542AD47878540ADCB\r\n"
                         "S31500009E5948878541208B69B26444B20045B2004304\r\n"
                         "S31500009E69205022A54148A54048B2FF42208B696887\r\n"
                         "S31500009E798544688545A54065448540A54165458570\r\n"
                         "S31500009E8941A54269008542A5406D4D878540A5419A\r\n"
                         "S31400009E996D4E878541A54269008D4F874CA9867E\r\n"
                         "S31500008678A950CD0286B0038D028660AD5702C93C6B\r\n"
                         "S31500008688D01EB20747B25046B20A45B2B944209640\r\n"
                         "S31500008698E3B20747B25046B20B45B2B9442096E357\r\n"
                         "S315000086A860A5408D4D87A5418D4E87B2D040B20753\r\n"
                         "S315000086B841B25A42208B69A542CD4F87F004B038A3\r\n"
                         "S315000086C8800EA541CD4E87D005A540CD4D87B02853\r\n"
                         "S315000086D8AD4B878540AD4C878541A541C900D0047F\r\n"
                         "S315000086E8A540C996B012A9758D3F87A9778D408791\r\n"
                         "S30E000086F8B200AEB248B1B75960F8\r\n"
                         "S70500000000FA\r\n";

template <typename Descriptor>
inline const Descriptor *ScanDescriptor(int deviceId, const void *previous, unsigned char type) {
    return static_cast<const Descriptor *>(sceUsbdScanStaticDescriptor(deviceId, previous, type));
}

inline bool IsConexantModem(const UsbDeviceDescriptor *device) {
    return device->vendorId == kConexantVendorId &&
           (device->productId == kModemProductId || device->productId == kModemProductIdAlternate);
}

// Copy the printable characters of a string descriptor's first 31 characters to a name, dropping
// commas and equals signs. The descriptor buffer must extend one byte past kNameBufferSize.
inline bool CopyStringDescriptor(const char *descriptor, char *name) {
    int skipped = 0;
    for (int i = 1; i < kNameLength; ++i) {
        const int offset = (i + skipped) * 2;
        if (offset > kNameBufferSize) {
            break;
        }
        const char character = descriptor[offset];
        if (static_cast<unsigned char>(character - kFirstPrintable) >= kPrintableCount) {
            if (i == 1) {
                return false;
            }
            name[i - 1] = '\0';
            break;
        }
        if (character == ',' || character == '=') {
            ++skipped;
            --i;
        } else {
            name[i - 1] = character;
        }
    }
    return true;
}

} // namespace

// NTSC-U/C: 0x00003564
CxtModem::LoadMode CxtModem::sLoadMode = CxtModem::kLoadModeNormal;

// NTSC-U/C: 0x00003560
bool CxtModem::sDeviceFound = false;

// NTSC-U/C: 0x00003570
char CxtModem::sDialString[CxtModem::kDialStringSize];

void CxtModem::NotifyModem(unsigned int events) {
    if ((events & MODEM_EVENT_SEND) == 0) {
        // The description is built and never printed.
        char description[kDescriptionSize];
        int length = 0;
        description[0] = '\0';
        length += sprintf(
            &description[length], "%s", (events & MODEM_EVENT_START_DONE) != 0 ? " StartDone" : "");
        length += sprintf(
            &description[length], "%s", (events & MODEM_EVENT_PLUG_OUT) != 0 ? " PlugOut" : "");
        length += sprintf(
            &description[length], "%s", (events & MODEM_EVENT_CONNECT) != 0 ? " Connect" : "");
        length += sprintf(&description[length],
                          "%s",
                          (events & MODEM_EVENT_DISCONNECT) != 0 ? " Disconnect" : "");
        length +=
            sprintf(&description[length], "%s", (events & MODEM_EVENT_RING) != 0 ? " Ring" : "");
        length +=
            sprintf(&description[length], "%s", (events & MODEM_EVENT_RECEIVE) != 0 ? " Recv" : "");
        sprintf(&description[length], "%s", (events & MODEM_EVENT_SEND) != 0 ? " Send" : "");
    }
    SetEventFlag(mModem.eventFlag, events);
}

void CxtModem::Signal(unsigned int events) {
    // The description is built and never printed.
    char description[kDescriptionSize];
    int length = 0;
    description[0] = '\0';
    length += sprintf(&description[length], "%s", (events & kEventStart) != 0 ? " START" : "");
    length += sprintf(&description[length], "%s", (events & kEventStop) != 0 ? " STOP" : "");
    length += sprintf(&description[length], "%s", (events & kEventState) != 0 ? " STATE" : "");
    sprintf(&description[length], "%s", (events & kEventBreak) != 0 ? " BREAK" : "");
    SetEventFlag(mEventFlag, events);
}

int CxtModem::Send(void *modem, const void *data, int length) {
    auto *self = static_cast<CxtModem *>(modem);
    if (self->mStarted == 0 || self->mReady == 0) {
        return 0;
    }
    int state;
    CpuSuspendIntr(&state);
    int space = self->mModem.sendSpace - length;
    if (space < 0) {
        space = kTransmitBufferSize; // Matches the binary.
    }
    self->mModem.sendSpace = space;
    CpuResumeIntr(state);

    const auto *bytes = static_cast<const char *>(data);
    int sent = 0;
    for (; length > 0; --length) {
        if (self->IsTransmitFull()) {
            self->FlushTransmit(0);
            DelayThread(kTransmitRetryDelay);
        }
        if (self->IsTransmitFull()) {
            break;
        }
        bcopy(&bytes[sent], &g_transferByte, 1);
        self->PutByte(g_transferByte);
        ++sent;
    }
    self->FlushTransmit(0);
    return sent;
}

int CxtModem::Write(const char *data, int length) {
    int state;
    CpuSuspendIntr(&state);
    int space = mModem.sendSpace - length;
    if (space < 0) {
        space = kTransmitBufferSize; // Matches the binary.
    }
    mModem.sendSpace = space;
    CpuResumeIntr(state);

    int sent = 0;
    for (; length > 0; --length) {
        while (IsTransmitFull()) {
            FlushTransmit(0);
            WaitSema(mTransmitSema);
        }
        bcopy(&data[sent], &g_transferByte, 1);
        PutByte(g_transferByte);
        ++sent;
    }
    FlushTransmit(0);
    return sent;
}

int CxtModem::Control(void *modem, unsigned int command, void *buffer, int size) {
    auto *self = static_cast<CxtModem *>(modem);
    int result = 0;
    if (command != kControlClearReceive && command != kControlClearTransmit &&
        command != kControlGetDialString && size != kControlValueSize) {
        return kControlBadSize;
    }
    switch (command) {
    case kControlGetThreadPriority:
        bcopy(&g_threadPriority, buffer, kControlValueSize);
        break;
    case kControlGetZero: {
        const int zero = 0;
        bcopy(&zero, buffer, kControlValueSize);
        break;
    }
    case kControlClearReceive: {
        int state;
        CpuSuspendIntr(&state);
        self->mModem.receiveCount = 0;
        self->mReceiveRead = 0;
        self->mReceiveWrite = 0;
        CpuResumeIntr(state);
        break;
    }
    case kControlClearTransmit: {
        int state;
        CpuSuspendIntr(&state);
        self->mTransmitLength = 0;
        self->mModem.sendSpace = kTransmitBufferSize;
        CpuResumeIntr(state);
        break;
    }
    case kControlGetDialString:
        result = static_cast<int>(strlen(sDialString)) + 1;
        if (size < result) {
            return kControlBadSize;
        }
        bcopy(sDialString, buffer, result);
        break;
    case kControlGetReceivedBytes:
        bcopy(&self->mReceivedBytes, buffer, kControlValueSize);
        break;
    case kControlGetSentBytes:
        bcopy(&self->mSentBytes, buffer, kControlValueSize);
        break;
    case kControlSetThreadPriority: {
        int priority;
        bcopy(buffer, &priority, kControlValueSize);
        if (self->mEventThread > 0) {
            result = ChangeThreadPriority(self->mEventThread, priority);
            if (result == KE_OK) {
                g_threadPriority = priority;
            }
        }
        if (self->mStatusThread > 0) {
            result = ChangeThreadPriority(self->mStatusThread, priority);
            if (result == KE_OK) {
                g_threadPriority = priority;
            }
        }
        if (self->mEventThread <= 0 || result == KE_ILLEGAL_PRIORITY) {
            if (priority >= kMinThreadPriority && priority <= kMaxThreadPriority) {
                g_threadPriority = priority;
            }
        }
        break;
    }
    default:
        result = kControlBadCommand;
        break;
    }
    return result;
}

unsigned char CxtModem::ModemStatus() const {
    return mModemStatus;
}

int CxtModem::Start(void *modem) {
    auto *self = static_cast<CxtModem *>(modem);
    const auto *device =
        ScanDescriptor<UsbDeviceDescriptor>(self->mDeviceId, nullptr, USB_DESCRIPTOR_TYPE_DEVICE);
    if (device == nullptr || !IsConexantModem(device)) {
        return 0;
    }
    if (sceUsbdScanStaticDescriptor(self->mDeviceId, device, USB_DESCRIPTOR_TYPE_INTERFACE) ==
        nullptr) {
        return 0;
    }
    if (self->mState == kStateUnplugged) {
        return 0;
    }
    if (self->mReady == 0) {
        for (int attempt = 0;; ++attempt) {
            DelayThread(kStartPollDelay);
            if (attempt >= kStartPollCount || self->mReady != 0) {
                break;
            }
        }
    }
    if (self->mStarted != 1) {
        self->ProgramSerialPort();
    }
    self->UpdateLines(kLineActive, 0);
    self->Signal(kEventStart);
    self->mStarted = 1;
    return 0;
}

unsigned int CxtModem::AlarmHandler(void *modem) {
    auto *self = static_cast<CxtModem *>(modem);
    self->mAlarmActive = 0;
    self->mModem.receiveCount = self->mReceiveWrite - self->mReceiveRead;
    iSetEventFlag(self->mModem.eventFlag, MODEM_EVENT_RECEIVE);
    ++self->mReceiveNotifications;
    return 0;
}

unsigned int CxtModem::PendingEvents() const {
    return mPendingEvents;
}

int CxtModem::WaitEvents(unsigned int mask) {
    if ((mPendingEvents & mask) == 0) {
        unsigned int result;
        if (WaitEventFlag(mEventFlag, mask, WEF_OR | WEF_CLEAR, &result) != KE_OK) {
            return -1;
        }
        mPendingEvents |= result;
    }
    const unsigned int events = mPendingEvents & mask;
    mPendingEvents &= ~mask;
    return static_cast<int>(events);
}

void CxtModem::StatusThread(void *modem) {
    auto *self = static_cast<CxtModem *>(modem);
    for (;;) {
        const unsigned char status = self->ModemStatus();
        if (self->mState == kStateReady || self->mState == kStateConnected) {
            const int carrier = (status & kStatusCarrier) != 0;
            if (self->mCarrier == 0) {
                if (carrier == 1) {
                    self->mCarrier = carrier;
                    self->Signal(kEventState);
                    self->UpdateLines(kLineConnected, 0);
                }
            } else if (carrier == 0) {
                self->mCarrier = 0;
                self->Signal(kEventState);
                self->UpdateLines(0, kLineConnected);
            }
        }
        const int ring = (status & kStatusRing) != 0;
        if (self->mRing == 0) {
            if (ring == 1) {
                self->mRing = ring;
                self->Signal(kEventState);
            }
        } else if (ring == 0) {
            self->mRing = 0;
            self->Signal(kEventState);
        }
        self->FlushTransmit(0);
        DelayThread(kStatusPollDelay);
    }
}

void CxtModem::EventThread(void *modem) {
    auto *self = static_cast<CxtModem *>(modem);
    self->mModem.sendSpace = kTransmitBufferSize;
    self->mState = kStateIdle;
    for (;;) {
        const int events = self->WaitEvents(kEventAll);
        if (events < 0) {
            return;
        }
        if (self->mState == kStateUnplugged) {
            continue;
        }
        switch (self->mState) {
        case kStateIdle:
            if ((events & kEventStart) != 0) {
                if (self->mReady == 1) {
                    self->mState = kStateReady;
                    self->NotifyModem(MODEM_EVENT_START_DONE);
                } else {
                    self->mState = kStateStarting;
                }
            }
            break;
        case kStateStarting:
            if ((events & kEventState) != 0 && self->mReady == 1) {
                self->mState = kStateReady;
                self->NotifyModem(MODEM_EVENT_START_DONE);
            }
            break;
        case kStateReady:
            if ((events & kEventState) != 0 && self->mCarrier != 0) {
                self->mState = kStateConnected;
                self->NotifyModem(MODEM_EVENT_CONNECT);
                self->UpdateLines(kLineConnected, 0);
            }
            break;
        case kStateConnected:
            if ((events & kEventState) != 0 && self->mCarrier == 0) {
                self->mState = kStateReady;
                self->NotifyModem(MODEM_EVENT_DISCONNECT);
                self->UpdateLines(0, kLineConnected);
            }
            break;
        case kStateDisconnecting:
            self->mState = kStateIdle;
            self->NotifyModem(MODEM_EVENT_DISCONNECT);
            self->UpdateLines(0, kLineConnected);
            break;
        default:
            break;
        }
        if (self->mState != kStateReady && self->mState != kStateConnected) {
            continue;
        }
        if ((events & kEventStop) != 0) {
            self->mState = kStateIdle;
            self->UpdateLines(0, kLineActive);
            continue;
        }
        if ((events & kEventBreak) != 0) {
            (void)self->PendingEvents(); // The binary discards the result.
        }
        if (self->mCarrier == 0 && self->mRing == 1) {
            self->NotifyModem(MODEM_EVENT_RING);
        }
    }
}

void CxtModem::DownloadThread(void *modem) {
    auto *self = static_cast<CxtModem *>(modem);
    self->ProgramSerialPort();
    self->UpdateLines(kLineActive, 0);
    DelayThread(kPowerUpDelay);
    // The commands are sent with their terminators.
    self->Write(kIdentifyCommand, sizeof(kIdentifyCommand));
    DelayThread(kResponseDelay);

    char response[kResponseBufferSize];
    int step = kDownloadNone;
    int received = 0;
    for (int attempt = 0; attempt < kIdentifyAttempts; ++attempt) {
        const int count = self->Read(&response[received], kResponseLength - received);
        if (count == 0) {
            continue;
        }
        received += count;
        for (int i = 0; i < received - 1; ++i) {
            if (memcmp(&response[i], kModelName, kModelNameLength) == 0) {
                step = kDownloadModelFound;
                break;
            }
        }
        if (step == kDownloadModelFound) {
            break;
        }
    }
    if (step == kDownloadModelFound) {
        self->Write(kDownloadCommand, sizeof(kDownloadCommand));
        DelayThread(kResponseDelay);
        received = 0;
        for (int attempt = 0; attempt < kPromptAttempts; ++attempt) {
            const int count = self->Read(&response[received], kResponseLength - received);
            if (count == 0) {
                continue;
            }
            received += count;
            for (int i = 0; i < received - 1; ++i) {
                if (memcmp(&response[i], kDownloadPrompt, kPromptLength) == 0) {
                    step = kDownloadPromptFound;
                    break;
                }
            }
            if (step == kDownloadPromptFound) {
                break;
            }
            DelayThread(kPromptRetryDelay);
        }
        // The firmware is sent whether or not the prompt arrived.
        self->Write(kFirmware, sizeof(kFirmware) - 1);
    }
    self->SetRegister(kRegisterFifoControl, kFifoReset);
    if (self->mState == kStateIdle) {
        self->SetRegister(kRegisterModemControl, kModemControlOff);
        self->SetRegister(kRegisterFifoControl, kFifoReset);
    }
    if (self->mState == kStateStarting) {
        self->Signal(kEventState);
    }
    self->mReady = 1;
}

CxtModem *CxtModem::Create() {
    auto *modem = static_cast<CxtModem *>(NetdevAllocMemory(kAllocationMode, sizeof(CxtModem)));
    if (modem == nullptr) {
        return nullptr;
    }
    bzero(modem, sizeof(CxtModem));
    modem->mTransmitDepth = -1;
    modem->mEndpoints[kBulkOutEndpoint].pending = 0;
    SemaParam sema;
    sema.attr = 0;
    sema.initCount = 1;
    sema.maxCount = 1;
    sema.option = 0;
    modem->mTransmitSema = CreateSema(&sema);
    modem->mRegisterWriteCommand = kRegisterWriteCommand;

    EventFlagParam flag;
    flag.attr = 0;
    flag.initPattern = 0;
    flag.option = 0;
    const int eventFlag = CreateEventFlag(&flag);
    modem->mEventFlag = eventFlag;
    // A failure does not delete the semaphore.
    if (eventFlag > 0) {
        ThreadParam param;
        param.attr = TH_C;
        param.entryWithArgument = EventThread;
        param.option = 0;
        param.initPriority = g_threadPriority;
        param.stackSize = kThreadStackSize;
        const int eventThread = CreateThread(&param);
        modem->mEventThread = eventThread;
        if (eventThread > 0) {
            if (StartThread(eventThread, modem) != KE_OK) {
                // The status thread does not exist yet, and the binary deletes it.
                DeleteThread(modem->mStatusThread);
            } else {
                modem->mCarrier = 0;
                modem->mRing = 0;
                param.attr = TH_C;
                param.entryWithArgument = StatusThread;
                param.option = 0;
                param.initPriority = g_threadPriority;
                param.stackSize = kThreadStackSize;
                const int statusThread = CreateThread(&param);
                modem->mStatusThread = statusThread;
                if (statusThread > 0) {
                    if (StartThread(statusThread, modem) == KE_OK) {
                        USec2SysClock(kAlarmDelay, &modem->mAlarmInterval);
                        return modem;
                    }
                    DeleteThread(modem->mStatusThread);
                }
            }
        }
        DeleteEventFlag(modem->mEventFlag);
    }
    NetdevFreeMemory(kAllocationMode, modem);
    return nullptr;
}

void CxtModem::Destroy() {
    TerminateThread(mEventThread);
    TerminateThread(mStatusThread);
    TerminateThread(mDownloadThread);
    DeleteThread(mEventThread);
    DeleteThread(mStatusThread);
    DeleteThread(mDownloadThread);
    DeleteSema(mTransmitSema);
    DeleteEventFlag(mEventFlag);
    NetdevFreeMemory(kAllocationMode, this);
}

int CxtModem::Stop(void *modem) {
    auto *self = static_cast<CxtModem *>(modem);
    self->SetRegister(kRegisterModemControl, kModemControlOff);
    self->SetRegister(kRegisterFifoControl, kFifoReset);
    self->UpdateLines(0, kLineConnected);
    if (self->mAlarmActive != 0) {
        CancelAlarm(AlarmHandler, self);
        self->mAlarmActive = 0;
        self->Signal(kEventStop);
    }
    if (self->mState == kStateUnplugged) {
        ModemUnregisterDevice(&self->mModem);
        self->Destroy();
    } else {
        self->Signal(kEventStop);
    }
    // The binary writes these fields even after Destroy() freed the modem.
    int state;
    CpuSuspendIntr(&state);
    self->mModem.receiveCount = 0;
    self->mReceiveRead = 0;
    self->mReceiveWrite = 0;
    self->mTransmitLength = 0;
    self->mModem.sendSpace = kTransmitBufferSize;
    CpuResumeIntr(state);
    self->mStarted = 0;
    return 0;
}

void CxtModem::UpdateFlowControl() {
    const unsigned char control = mRegisterValues[kRegisterModemControl];
    if (mReceiveWrite - mReceiveRead < kFlowResumeThreshold &&
        (control & kModemControlRequestToSend) == 0) {
        SetRegister(kRegisterModemControl, control | kModemControlRequestToSend);
    } else if (mReceiveWrite - mReceiveRead > kFlowPauseThreshold &&
               (control & kModemControlRequestToSend) != 0) {
        SetRegister(kRegisterModemControl, control & ~kModemControlRequestToSend);
    }
}

int CxtModem::Receive(void *modem, void *data, int length) {
    auto *self = static_cast<CxtModem *>(modem);
    if (self->mReady == 0) {
        return 0;
    }
    return self->Read(static_cast<char *>(data), length);
}

int CxtModem::Read(char *data, int length) {
    int state;
    CpuSuspendIntr(&state);
    int count = 0;
    for (; length > 0 && HasInput(); --length) {
        // The binary stores the byte through a null pointer and copies it from there.
        const char value = ReadByte();
        bcopy(&value, &data[count], 1);
        ++count;
    }
    mModem.receiveCount = mReceiveWrite - mReceiveRead;
    CpuResumeIntr(state);
    UpdateFlowControl();
    return count;
}

void CxtModem::SetInterfaceDone(int result, int, void *modem) {
    auto *self = static_cast<CxtModem *>(modem);
    if (result != 0) {
        printf(kErrorFormat, "sceUsbdSetInterface", result);
    }
    self->mInterfaceSelected = true;
    self->mConfigured = true;
    self->Configure(kConfigureOpen);
}

void CxtModem::SetConfigurationDone(int result, int, void *modem) {
    auto *self = static_cast<CxtModem *>(modem);
    if (result != 0) {
        printf(kErrorFormat, "sceUsbdSetConfiguration", result);
    }
    UsbDeviceRequest request;
    request.requestType = USB_REQUEST_TYPE_OUT_INTERFACE;
    request.request = USB_REQUEST_SET_INTERFACE;
    request.value = 0;
    request.index = 0;
    request.length = 0;
    const int error =
        sceUsbdTransferPipe(self->mControlPipe, nullptr, 0, &request, SetInterfaceDone, self);
    if (error != 0) {
        printf(kErrorFormat, "sceUsbdSetInterface", error);
    }
}

int CxtModem::Probe(int deviceId) {
    const auto *device =
        ScanDescriptor<UsbDeviceDescriptor>(deviceId, nullptr, USB_DESCRIPTOR_TYPE_DEVICE);
    if (device == nullptr) {
        return 1; // The binary takes a device that has no device descriptor.
    }
    if (!IsConexantModem(device)) {
        return 0;
    }
    sDeviceFound = true;
    if (sceUsbdScanStaticDescriptor(deviceId, device, USB_DESCRIPTOR_TYPE_INTERFACE) == nullptr) {
        return 0;
    }
    return sLoadMode != kLoadModeTest;
}

int CxtModem::Connect(int deviceId) {
    // The vendor buffer, then the product buffer, then a byte the vendor name parse can read past
    // the end of the product buffer.
    char names[2 * kNameBufferSize + 1] = {};
    char *vendor = &names[0];
    char *product = &names[kNameBufferSize];
    memcpy(vendor, kDefaultVendorName, sizeof(kDefaultVendorName));
    memcpy(product, kDefaultProductName, sizeof(kDefaultProductName));

    const auto *configuration = ScanDescriptor<UsbConfigurationDescriptor>(
        deviceId, nullptr, USB_DESCRIPTOR_TYPE_CONFIGURATION);
    if (configuration == nullptr) {
        return -1;
    }
    if (configuration->interfaceCount != kInterfaceCount) {
        return -1;
    }
    const auto *interface = ScanDescriptor<UsbInterfaceDescriptor>(
        deviceId, configuration, USB_DESCRIPTOR_TYPE_INTERFACE);
    if (interface == nullptr) {
        return -1;
    }
    if (interface->endpointCount != kInterfaceEndpointCount) {
        return -1;
    }
    CxtModem *modem = Create();
    if (modem == nullptr) {
        return -1;
    }
    modem->mDeviceId = deviceId;
    const int controlPipe = sceUsbdOpenPipe(deviceId, nullptr);
    modem->mControlPipe = controlPipe;
    if (controlPipe < 0) {
        return -1;
    }
    const void *previous = interface;
    for (int i = 0; i < kInterfaceEndpointCount; ++i) {
        const auto *endpoint =
            ScanDescriptor<UsbEndpointDescriptor>(deviceId, previous, USB_DESCRIPTOR_TYPE_ENDPOINT);
        if (endpoint == nullptr) {
            return -1;
        }
        const int pipe = sceUsbdOpenPipe(deviceId, endpoint);
        modem->mEndpoints[i].pipe = pipe;
        if (pipe < 0) {
            return -1;
        }
        previous = endpoint;
    }
    sceUsbdSetPrivateData(deviceId, modem);

    UsbDeviceRequest request;
    request.requestType = USB_REQUEST_TYPE_OUT_DEVICE;
    request.request = USB_REQUEST_SET_CONFIGURATION;
    request.value = configuration->configurationValue;
    request.index = 0;
    request.length = 0;
    const int error =
        sceUsbdTransferPipe(modem->mControlPipe, nullptr, 0, &request, SetConfigurationDone, modem);
    if (error != 0) {
        printf(kErrorFormat, "sceUsbdSetConfiguration", error);
        return -1;
    }

    const auto *device =
        ScanDescriptor<UsbDeviceDescriptor>(modem->mDeviceId, nullptr, USB_DESCRIPTOR_TYPE_DEVICE);
    if (device == nullptr || !IsConexantModem(device)) {
        return -1;
    }
    // The binary sign-extends the string indices.
    const auto manufacturerIndex = static_cast<signed char>(device->manufacturerIndex);
    const auto productIndex = static_cast<signed char>(device->productIndex);
    DelayThread(kStringRequestDelay);
    // The vendor copy runs one byte into the product name, which the product copy replaces.
    bcopy(vendor, modem->mVendorName, sizeof(modem->mVendorName));
    bcopy(product, modem->mProductName, kNameLength);

    request.requestType = USB_REQUEST_TYPE_IN_DEVICE;
    request.request = USB_REQUEST_GET_DESCRIPTOR;
    request.value =
        static_cast<unsigned short>(manufacturerIndex | (USB_DESCRIPTOR_TYPE_STRING << 8));
    request.index = 0;
    request.length = kStringRequestLength;
    sceUsbdTransferPipe(
        modem->mControlPipe, vendor, kStringRequestLength, &request, nullptr, modem);
    DelayThread(kStringRequestDelay);
    if (CopyStringDescriptor(vendor, modem->mVendorName)) {
        modem->mProductName[0] = '\0';
        request.requestType = USB_REQUEST_TYPE_IN_DEVICE;
        request.request = USB_REQUEST_GET_DESCRIPTOR;
        request.value =
            static_cast<unsigned short>(productIndex | (USB_DESCRIPTOR_TYPE_STRING << 8));
        request.index = 0;
        request.length = kStringRequestLength;
        sceUsbdTransferPipe(
            modem->mControlPipe, product, kStringRequestLength, &request, nullptr, modem);
        DelayThread(kStringRequestDelay);
        CopyStringDescriptor(product, modem->mProductName);
    }

    ModemDeviceOps &record = modem->mModem;
    record.module = kModemModuleName;
    record.vendor = modem->mVendorName;
    record.product = modem->mProductName;
    record.bus = MODEM_BUS_USB;
    if (sceUsbdGetDeviceLocation(deviceId, record.location) != 0) {
        return -1;
    }
    record.start = Start;
    record.stop = Stop;
    record.receive = Receive;
    record.send = Send;
    record.reserved2 = 0;
    record.reserved3 = 0;
    record.priv = modem;
    record.control = Control;
    if (ModemRegisterDevice(&record) < 0) {
        return -1;
    }
    DelayThread(kStringRequestDelay);

    modem->mReady = 0;
    modem->mStarted = 0;
    ThreadParam param;
    param.attr = TH_C;
    param.entryWithArgument = DownloadThread;
    param.option = 0;
    param.initPriority = g_threadPriority;
    param.stackSize = kThreadStackSize;
    const int thread = CreateThread(&param);
    modem->mDownloadThread = thread;
    if (thread > 0 && StartThread(thread, modem) == KE_OK) {
        return 0;
    }
    DeleteThread(modem->mDownloadThread);
    return 0;
}

int CxtModem::Disconnect(int deviceId) {
    auto *modem = static_cast<CxtModem *>(sceUsbdGetPrivateData(deviceId));
    if (modem == nullptr) {
        return -1;
    }
    switch (modem->mState) {
    case kStateStarting:
    case kStateReady:
    case kStateConnected:
    case kStateDisconnecting:
        modem->mState = kStateUnplugged;
        modem->NotifyModem(MODEM_EVENT_PLUG_OUT);
        break;
    default:
        modem->NotifyModem(MODEM_EVENT_PLUG_OUT);
        ModemUnregisterDevice(&modem->mModem);
        modem->Destroy();
        break;
    }
    // The binary writes these fields even after Destroy() freed the modem.
    modem->mReady = 0;
    modem->mStarted = 0;
    return 0;
}

void CxtModem::SetRegister(unsigned int index, unsigned char value) {
    if (index < kRegisterCount) {
        mRegisterValues[index] = value;
        mRegisterPending[index] = kRegisterPendingMark;
        mRegistersDirty = true;
        FlushRegisters();
    }
}

void CxtModem::FlushTransmit(int) {
    if (++mTransmitDepth != 0) {
        return;
    }
    UsbEndpoint &out = mEndpoints[kBulkOutEndpoint];
    do {
        while (out.pending <= 0 && !out.blocked && mConfigured) {
            int chunk = mTransmitLength;
            if (chunk > kTransmitChunkLength) {
                chunk = kTransmitChunkLength;
            }
            int copied = 0;
            for (; copied < chunk; ++copied) {
                mTransmitChunk[copied] = mTransmitBuffer[copied];
            }
            int kept = 0;
            for (int i = copied; i < mTransmitLength; ++i) {
                mTransmitBuffer[kept++] = mTransmitBuffer[i];
            }
            mTransmitLength = kept;
            if (kept == 0) {
                SignalSema(mTransmitSema);
            }
            if (copied > 0) {
                Transfer(&out, mTransmitChunk, copied, TransmitDone);
                break;
            }
            if (mTransmitLength == 0) {
                break;
            }
        }
    } while (--mTransmitDepth >= 0);
}

char CxtModem::ReadByte() {
    const int available = mReceiveWrite - mReceiveRead;
    if (available <= 0) {
        mReceiveRead = 0;
        mReceiveWrite = 0;
        return 0;
    }
    const unsigned char value = mReceiveBuffer[mReceiveRead];
    ++mReceiveRead;
    if (available == 1) {
        mReceiveRead = 0;
        mReceiveWrite = 0;
    }
    return static_cast<char>(value);
}

bool CxtModem::IsTransmitFull() const {
    return static_cast<unsigned int>(mTransmitLength) >= kTransmitBufferSize;
}

void CxtModem::PutByte(char value) {
    if (static_cast<unsigned int>(mTransmitLength) < kTransmitBufferSize) {
        mTransmitBuffer[mTransmitLength] = static_cast<unsigned char>(value);
        ++mTransmitLength;
    }
}

void CxtModem::RegisterTransferDone(int result, int, void *modem) {
    auto *self = static_cast<CxtModem *>(modem);
    --self->mEndpoints[kRegisterEndpoint].pending;
    if (!self->mConfigured) {
        return;
    }
    self->RegistersWritten();
    if (result == 0) {
        self->FlushRegisters();
    }
}

void CxtModem::TransmitDone(int result, int count, void *modem) {
    auto *self = static_cast<CxtModem *>(modem);
    --self->mEndpoints[kBulkOutEndpoint].pending;
    if (!self->mConfigured || result != 0) {
        return;
    }
    self->FlushTransmit(1);
    int state;
    CpuSuspendIntr(&state);
    self->mSentBytes += count;
    CpuResumeIntr(state);
    const int length = self->mTransmitLength;
    if (length < kSendNotifyThreshold) {
        self->mModem.sendSpace = kTransmitBufferSize - length;
        self->NotifyModem(MODEM_EVENT_SEND);
    }
}

void CxtModem::ReceiveDone(int result, int count, void *modem) {
    auto *self = static_cast<CxtModem *>(modem);
    --self->mEndpoints[kBulkInEndpoint].pending;
    if (!self->mConfigured || (self->mFlags & kFlagReceiving) == 0) {
        return;
    }
    if (result != 0) {
        self->mFlags &= ~kFlagReceiving;
        return;
    }
    unsigned int length = count;
    if (length >= kInputBufferSize) {
        length = kInputTransferLength;
    }
    // An empty packet wraps to a pair count above the limit.
    const unsigned int pairs = (length - 1) / 2;
    if (pairs <= kMaxInputPairs) {
        self->ParseInput(self->mInputBuffer, static_cast<int>(pairs));
    }
    self->StartReceive();
    int state;
    CpuSuspendIntr(&state);
    self->mReceivedBytes += length;
    CpuResumeIntr(state);
}

void CxtModem::ParseInput(const unsigned char *data, int pairs) {
    const unsigned char status = data[0];
    int state;
    CpuSuspendIntr(&state);
    int pair = 0;
    for (; pair < pairs; ++pair) {
        const unsigned char flags = data[1 + pair * 2];
        const unsigned char value = data[2 + pair * 2];
        if ((flags & kInputByteValid) == 0) {
            break;
        }
        if (static_cast<unsigned int>(mReceiveWrite) >= kReceiveBufferSize - 1) {
            if (mReceiveRead == 0) {
                continue; // A full buffer drops the byte.
            }
            int kept = 0;
            for (int i = mReceiveRead; i < mReceiveWrite; ++i) {
                mReceiveBuffer[kept++] = mReceiveBuffer[i];
            }
            mReceiveWrite = kept;
            mReceiveRead = 0;
        }
        mReceiveBuffer[mReceiveWrite] = value;
        ++mReceiveWrite;
    }
    if (pair > 0) {
        mReceivedBytes += mReceiveWrite; // The binary adds the write index, not the new bytes.
        if (mAlarmActive != 0) {
            CancelAlarm(AlarmHandler, this);
            mAlarmActive = 0;
        }
        if (mReceiveWrite > kReceiveNotifyThreshold) {
            mModem.receiveCount = mReceiveWrite - mReceiveRead;
            NotifyModem(MODEM_EVENT_RECEIVE);
            ++mReceiveNotifications;
        } else {
            SetAlarm(&mAlarmInterval, AlarmHandler, this);
            mAlarmActive = 1;
        }
    }
    CpuResumeIntr(state);
    UpdateFlowControl();
    mModemStatus = status;
}

bool CxtModem::HasInput() const {
    return mReceiveRead != mReceiveWrite;
}

int CxtModem::Transfer(UsbEndpoint *endpoint, void *data, int length, UsbdDoneCallback callback) {
    if (!mConfigured) {
        return kTransferNotConfigured;
    }
    ++endpoint->pending;
    return sceUsbdTransferPipe(endpoint->pipe, data, length, nullptr, callback, this);
}

void CxtModem::StartReceive() {
    UsbEndpoint &in = mEndpoints[kBulkInEndpoint];
    if (in.pending > 0 || in.blocked || !mConfigured) {
        return;
    }
    Transfer(&in, mInputBuffer, kInputTransferLength, ReceiveDone);
}

void CxtModem::FlushRegisters() {
    UsbEndpoint &registers = mEndpoints[kRegisterEndpoint];
    if (registers.pending > 0) {
        return;
    }
    while (mRegistersDirty || mStagedCount != 0) {
        if (!mConfigured || registers.blocked) {
            return;
        }
        int staged = mStagedCount;
        int index = 0;
        for (; index < kRegisterCount; ++index) {
            if (mRegisterPending[index] == 0) {
                continue;
            }
            if (static_cast<unsigned int>(staged) >= kStagedWriteCount) {
                break;
            }
            RegisterWrite &write = mStagedWrites[staged];
            write.index = index;
            write.value = mRegisterValues[index];
            write.command = mRegisterWriteCommand;
            ++staged;
            mRegisterPending[index] = 0;
        }
        if (index == kRegisterCount) {
            mRegistersDirty = false;
        }
        mStagedCount = staged;
        const int size = staged * static_cast<int>(sizeof(RegisterWrite));
        if (size != 0) {
            memcpy(mRegisterPacket, mStagedWrites, size);
            mStagedCount = 0;
            Transfer(&registers, mRegisterPacket, size, RegisterTransferDone);
        }
    }
}

void CxtModem::RegistersWritten() {
}

void CxtModem::ControlPacketDone(int result, int, void *modem) {
    auto *self = static_cast<CxtModem *>(modem);
    --self->mEndpoints[kControlEndpoint].pending;
    if (self->mConfigured && result == 0) {
        self->SendControlPacket();
    }
}

void CxtModem::SendControlPacket() {
    UsbEndpoint &control = mEndpoints[kControlEndpoint];
    if (control.pending != 0 || !mControlPacketPending || !mConfigured) {
        return;
    }
    mControlPacketPending = false;
    Transfer(&control, &mControlPacket, sizeof(ControlPacket), ControlPacketDone);
}

void CxtModem::UpdateMode(unsigned char set, unsigned char clear) {
    mControlPacketPending = true;
    mControlPacket.mode = (mControlPacket.mode & ~clear) | set;
    SendControlPacket();
}

void CxtModem::UpdateLines(unsigned char set, unsigned char clear) {
    mControlPacketPending = true;
    mControlPacket.lines = (mControlPacket.lines & ~clear) | set;
    SendControlPacket();
}

void CxtModem::Configure(int phase) {
    if (phase == kConfigureClose) {
        mFlags &= ~kFlagReceiving;
        UpdateLines(0, kLineActive);
        UpdateLines(0, kLineConnected);
        UpdateLines(0, kLineReserved);
    } else if (phase == kConfigureOpen) {
        mFlags |= kFlagReceiving;
        UpdateMode(kModeOpen, 0);
        UpdateLines(kLineActive, 0);
        DelayThread(kLineDelay);
        UpdateMode(0, kModeReset);
        DelayThread(kLineDelay);
        mClearedOnConfigure = 0;
        SetRegister(kRegisterLineControl, kLineControlDivisorLatch);
        SetRegister(kRegisterDivisorLow, kDivisorLow);
        SetRegister(kRegisterDivisorHigh, kDivisorHigh);
        SetRegister(kRegisterLineControl, kLineControlEightBits);
        SetRegister(kRegisterFifoControl, kFifoEnable);
        SetRegister(kRegisterModemControl, kModemControlOn);
        mEndpoints[kBulkInEndpoint].pending = 0;
        StartReceive();
    }
}

void CxtModem::ProgramSerialPort() {
    SetRegister(kRegisterLineControl, kLineControlDivisorLatch);
    SetRegister(kRegisterDivisorLow, kDivisorLow);
    SetRegister(kRegisterDivisorHigh, kDivisorHigh);
    DelayThread(kRegisterSettleDelay);
    SetRegister(kRegisterLineControl, kLineControlEightBits);
    SetRegister(kRegisterFifoControl, kFifoEnable);
    SetRegister(kRegisterModemControl, kModemControlOn);
}
