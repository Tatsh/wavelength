#include <stddef.h>

#include <eekernel.h>
#include <libusbkb.h>
#include <sifrpc.h>

enum {
    kUsbKbRpcServer = 0x80000211,
    // The bind retry loop counts down from this value before checking readiness.
    kBindDelay = 0x10000,

    // The functions of the usbkbd server.
    kRpcGetInfo = 1,
    kRpcRead = 2,
    kRpcGetLocation = 3,
    kRpcSetLedStatus = 4,
    kRpcSetLedMode = 5,
    kRpcClearRbuf = 6,

    kSendSize = 16,
    kGetInfoReplySize = 144,
    kReadReplySize = 80,
    kSetLedModeReplySize = 16,
    kReadReplyHeaderSize = 4,
    kLocationSize = 7,

    kErrorNotReady = -1,
    kErrorRpc = -2,
    kErrorArgument1 = -11,
    kErrorArgument2 = -12,

    // The report length the server sends when the keyboard has no new report.
    kNoReport = 255,
    // The USB usage a keyboard reports in every slot when too many keys are held.
    kKeyErrorRollOver = 1,

    kLedModeCount = 3,
    kCodeTypeCount = 2,
    kReadModeCount = 2,
    kArrangementCount = 3,
    kShiftStateCount = 2,
    kNumLockStateCount = 2,
    kKeyCodeCount = 256,
    kKeypadEntryCount = 21,
    kRepeatBufferSize = 4,

    // Modifier and indicator bits of a report.
    kModifierLeftShift = 0x02,
    kModifierRightShift = 0x20,
    kLedNumLock = 0x01,
    kLedCapsLock = 0x02,

    kCaseOffset = 'a' - 'A',
};

// Modes of sceUsbKbSync().
enum {
    kSyncWait = 0,
    kSyncPoll = 1,
    kSyncBusy = 1,
};

// Modes of sceUsbKbSetReadMode().
enum {
    kReadModeInputChar = 0,
    kReadModePacket = 1,
};

// Modes of sceUsbKbSetCodeType().
enum {
    kCodeTypeRaw = 0,
    kCodeTypeAscii = 1,
};

// Layouts of sceUsbKbSetArrangement().
enum {
    kArrangement101 = 0,
    kArrangement106 = 1,
    kArrangement106Kana = 2,
};

// Defaults of a keyboard after sceUsbKbInit().
enum {
    kDefaultLedMode = 1,
    kDefaultCodeType = kCodeTypeAscii,
    kDefaultArrangement = kArrangement106,
};

// Key repeat states.
enum {
    kRepeatIdle = 0,
    kRepeatFirst = 1, // Waiting for the first repeat.
    kRepeatNext = 2,  // Waiting for a later repeat.
    kRepeatStateCount = 3,
};

typedef struct {
    int nState;
    int nCounter; // Vertical blanks since the key went down or last repeated.
    int nStart;
    int nInterval;
    unsigned short nKeyCode;
    unsigned short anBuffer[kRepeatBufferSize];
    int nWrite;
    int nRead;
    int nCount;
} UsbKbRepeat;

typedef struct {
    int nLedMode;
    int nCodeType;
    int nArrangement;
    int nReadMode;
    UsbKbRepeat repeat;
} UsbKbStatus;

typedef struct {
    unsigned char nKeyboard;
    unsigned char nValue;
    unsigned char abReserved[kSendSize - 2];
} UsbKbRequest;

typedef union {
    struct {
        unsigned char nMaxConnect;
        unsigned char nNowConnect;
        unsigned char abStatus[USBKB_MAX_STATUS];
    } info;
    struct {
        unsigned char nLed;
        unsigned char nLen;
        unsigned char nMkey;
        unsigned char nReserved;
        unsigned char abKeyCode[kReadReplySize - kReadReplyHeaderSize];
    } read;
    char szLocation[kLocationSize];
    UsbKbRequest led; // The server echoes the request of an indicator change.
    unsigned char abBytes[kGetInfoReplySize];
} UsbKbReply;

typedef struct {
    unsigned char nRawCode;
    unsigned short nKeyCode;
} UsbKbKeypadEntry;

// NTSC-U/C: 0x003c2aa8, PAL: 0x00458ee8
static int g_nUsbKbSemaId = -1;

// NTSC-U/C: 0x0052d5c0
static int g_nUsbKbDeviceCount;

// NTSC-U/C: 0x0052d5c4
static UsbKbStatus *g_pUsbKbStateTable;

// NTSC-U/C: 0x0052d5c8, the keys of the previous report.
static USBKBDATA_t g_UsbKbLastKeyState;

// NTSC-U/C: 0x0052d650, the request sceUsbKbSync() finishes.
static int g_nUsbKbRpcCommand;

// NTSC-U/C: 0x0052d654, the keyboard of the request.
static unsigned int g_nUsbKbRpcArg;

// NTSC-U/C: 0x0052d658, the caller's buffer for the result of the request.
static void *g_pUsbKbRpcUserBuffer;

// NTSC-U/C: 0x0052d65c
static int g_nUsbKbIntcHandlerId;

// NTSC-U/C: 0x0052d660
static sceSifClientData g_UsbKbRpcClientData;

// NTSC-U/C: 0x0052d6c0
// SIF DMA needs the 64-byte alignment retail gives it.
static UsbKbRequest g_UsbKbRpcSendBuffer __attribute__((aligned(64)));

// NTSC-U/C: 0x0052d700
// SIF DMA needs the 64-byte alignment retail gives it.
static UsbKbReply g_UsbKbRpcRecvBuffer __attribute__((aligned(64)));

// NTSC-U/C: 0x0052d790
static UsbKbStatus g_aUsbKbState[USBKB_MAX_STATUS];

// NTSC-U/C: 0x004310e8, PAL: 0x004cdd98, the characters of each USB usage by layout and shift
// state.
static const unsigned short g_awUsbKbAsciiTable[kArrangementCount][kShiftStateCount]
                                               [kKeyCodeCount] = {
                                                   {
                                                       {
                                                           [0x01 ... 0x03] = USBKB_RAWDAT,
                                                           [0x04] = 'a',
                                                           'b',
                                                           'c',
                                                           'd',
                                                           'e',
                                                           'f',
                                                           'g',
                                                           'h',
                                                           'i',
                                                           'j',
                                                           'k',
                                                           'l',
                                                           'm',
                                                           'n',
                                                           'o',
                                                           'p',
                                                           'q',
                                                           'r',
                                                           's',
                                                           't',
                                                           'u',
                                                           'v',
                                                           'w',
                                                           'x',
                                                           'y',
                                                           'z',
                                                           '1',
                                                           '2',
                                                           '3',
                                                           '4',
                                                           '5',
                                                           '6',
                                                           '7',
                                                           '8',
                                                           '9',
                                                           '0',
                                                           '\n',
                                                           USBKB_RAWDAT,
                                                           '\b',
                                                           '\t',
                                                           ' ',
                                                           '-',
                                                           '=',
                                                           '[',
                                                           ']',
                                                           '\\',
                                                           '#',
                                                           ';',
                                                           '\'',
                                                           '`',
                                                           ',',
                                                           '.',
                                                           '/',
                                                           [0x39 ... 0x52] = USBKB_RAWDAT,
                                                           [0x53 ... 0x63] = USBKB_KEYPAD,
                                                           [0x64] = '\\',
                                                           [0x65 ... 0x66] = USBKB_RAWDAT,
                                                           [0x67] = USBKB_KEYPAD,
                                                           [0x68 ... 0x84] = USBKB_RAWDAT,
                                                           [0x85 ... 0x86] = USBKB_KEYPAD,
                                                           [0x87 ... 0x8b] = USBKB_RAWDAT,
                                                           [0x8c] = USBKB_KEYPAD,
                                                           [0x8d ... 0xff] = USBKB_RAWDAT,
                                                       },
                                                       {
                                                           [0x01 ... 0x03] = USBKB_RAWDAT,
                                                           [0x04] = 'A',
                                                           'B',
                                                           'C',
                                                           'D',
                                                           'E',
                                                           'F',
                                                           'G',
                                                           'H',
                                                           'I',
                                                           'J',
                                                           'K',
                                                           'L',
                                                           'M',
                                                           'N',
                                                           'O',
                                                           'P',
                                                           'Q',
                                                           'R',
                                                           'S',
                                                           'T',
                                                           'U',
                                                           'V',
                                                           'W',
                                                           'X',
                                                           'Y',
                                                           'Z',
                                                           '!',
                                                           '@',
                                                           '#',
                                                           '$',
                                                           '%',
                                                           '^',
                                                           '&',
                                                           '*',
                                                           '(',
                                                           ')',
                                                           '\n',
                                                           USBKB_RAWDAT,
                                                           '\b',
                                                           '\t',
                                                           ' ',
                                                           '_',
                                                           '+',
                                                           '{',
                                                           '}',
                                                           '|',
                                                           '~',
                                                           ':',
                                                           '"',
                                                           '~',
                                                           '<',
                                                           '>',
                                                           '?',
                                                           [0x39 ... 0x52] = USBKB_RAWDAT,
                                                           [0x53 ... 0x63] = USBKB_KEYPAD,
                                                           [0x64] = '|',
                                                           [0x65 ... 0x66] = USBKB_RAWDAT,
                                                           [0x67] = USBKB_KEYPAD,
                                                           [0x68 ... 0x84] = USBKB_RAWDAT,
                                                           [0x85 ... 0x86] = USBKB_KEYPAD,
                                                           [0x87 ... 0x8b] = USBKB_RAWDAT,
                                                           [0x8c] = USBKB_KEYPAD,
                                                           [0x8d ... 0xff] = USBKB_RAWDAT,
                                                       },
                                                   },
                                                   {
                                                       {
                                                           [0x01 ... 0x03] = USBKB_RAWDAT,
                                                           [0x04] = 'a',
                                                           'b',
                                                           'c',
                                                           'd',
                                                           'e',
                                                           'f',
                                                           'g',
                                                           'h',
                                                           'i',
                                                           'j',
                                                           'k',
                                                           'l',
                                                           'm',
                                                           'n',
                                                           'o',
                                                           'p',
                                                           'q',
                                                           'r',
                                                           's',
                                                           't',
                                                           'u',
                                                           'v',
                                                           'w',
                                                           'x',
                                                           'y',
                                                           'z',
                                                           '1',
                                                           '2',
                                                           '3',
                                                           '4',
                                                           '5',
                                                           '6',
                                                           '7',
                                                           '8',
                                                           '9',
                                                           '0',
                                                           '\n',
                                                           USBKB_RAWDAT,
                                                           '\b',
                                                           '\t',
                                                           ' ',
                                                           '-',
                                                           '^',
                                                           '@',
                                                           '[',
                                                           '\\',
                                                           ']',
                                                           ';',
                                                           ':',
                                                           USBKB_RAWDAT,
                                                           ',',
                                                           '.',
                                                           '/',
                                                           [0x39 ... 0x52] = USBKB_RAWDAT,
                                                           [0x53 ... 0x63] = USBKB_KEYPAD,
                                                           [0x64] = '\\',
                                                           [0x65 ... 0x66] = USBKB_RAWDAT,
                                                           [0x67] = USBKB_KEYPAD,
                                                           [0x68 ... 0x84] = USBKB_RAWDAT,
                                                           [0x85 ... 0x86] = USBKB_KEYPAD,
                                                           [0x87 ... 0x88] = USBKB_RAWDAT,
                                                           [0x89] = '\\',
                                                           [0x8a ... 0x8b] = USBKB_RAWDAT,
                                                           [0x8c] = USBKB_KEYPAD,
                                                           [0x8d ... 0xff] = USBKB_RAWDAT,
                                                       },
                                                       {
                                                           [0x01 ... 0x03] = USBKB_RAWDAT,
                                                           [0x04] = 'A',
                                                           'B',
                                                           'C',
                                                           'D',
                                                           'E',
                                                           'F',
                                                           'G',
                                                           'H',
                                                           'I',
                                                           'J',
                                                           'K',
                                                           'L',
                                                           'M',
                                                           'N',
                                                           'O',
                                                           'P',
                                                           'Q',
                                                           'R',
                                                           'S',
                                                           'T',
                                                           'U',
                                                           'V',
                                                           'W',
                                                           'X',
                                                           'Y',
                                                           'Z',
                                                           '!',
                                                           '"',
                                                           '#',
                                                           '$',
                                                           '%',
                                                           '&',
                                                           '\'',
                                                           '(',
                                                           ')',
                                                           '~',
                                                           '\n',
                                                           USBKB_RAWDAT,
                                                           '\b',
                                                           '\t',
                                                           ' ',
                                                           '=',
                                                           '~',
                                                           '`',
                                                           '{',
                                                           '|',
                                                           '}',
                                                           '+',
                                                           '*',
                                                           USBKB_RAWDAT,
                                                           '<',
                                                           '>',
                                                           '?',
                                                           [0x39 ... 0x52] = USBKB_RAWDAT,
                                                           [0x53 ... 0x63] = USBKB_KEYPAD,
                                                           [0x64] = '|',
                                                           [0x65 ... 0x66] = USBKB_RAWDAT,
                                                           [0x67] = USBKB_KEYPAD,
                                                           [0x68 ... 0x84] = USBKB_RAWDAT,
                                                           [0x85 ... 0x86] = USBKB_KEYPAD,
                                                           [0x87] = '_',
                                                           [0x88] = USBKB_RAWDAT,
                                                           [0x89] = '|',
                                                           [0x8a ... 0x8b] = USBKB_RAWDAT,
                                                           [0x8c] = USBKB_KEYPAD,
                                                           [0x8d ... 0xff] = USBKB_RAWDAT,
                                                       },
                                                   },
                                                   // Half-width katakana in JIS X 0201.
                                                   {
                                                       {
                                                           [0x01 ... 0x03] = USBKB_RAWDAT,
                                                           [0x04] = 0xc1,
                                                           0xba,
                                                           0xbf,
                                                           0xbc,
                                                           0xb2,
                                                           0xca,
                                                           0xb7,
                                                           0xb8,
                                                           0xc6,
                                                           0xcf,
                                                           0xc9,
                                                           0xd8,
                                                           0xd3,
                                                           0xd0,
                                                           0xd7,
                                                           0xbe,
                                                           0xc0,
                                                           0xbd,
                                                           0xc4,
                                                           0xb6,
                                                           0xc5,
                                                           0xcb,
                                                           0xc3,
                                                           0xbb,
                                                           0xdd,
                                                           0xc2,
                                                           0xc7,
                                                           0xcc,
                                                           0xb1,
                                                           0xb3,
                                                           0xb4,
                                                           0xb5,
                                                           0xd4,
                                                           0xd5,
                                                           0xd6,
                                                           0xdc,
                                                           '\n',
                                                           USBKB_RAWDAT,
                                                           '\b',
                                                           '\t',
                                                           ' ',
                                                           0xce,
                                                           0xcd,
                                                           0xde,
                                                           0xdf,
                                                           0xb0,
                                                           0xd1,
                                                           0xda,
                                                           0xb9,
                                                           USBKB_RAWDAT,
                                                           0xc8,
                                                           0xd9,
                                                           0xd2,
                                                           [0x39 ... 0x52] = USBKB_RAWDAT,
                                                           [0x53 ... 0x63] = USBKB_KEYPAD,
                                                           [0x64] = '\\',
                                                           [0x65 ... 0x66] = USBKB_RAWDAT,
                                                           [0x67] = USBKB_KEYPAD,
                                                           [0x68 ... 0x84] = USBKB_RAWDAT,
                                                           [0x85 ... 0x86] = USBKB_KEYPAD,
                                                           [0x87] = 0xdb,
                                                           [0x88] = USBKB_RAWDAT,
                                                           [0x89] = 0xb0,
                                                           [0x8a ... 0x8b] = USBKB_RAWDAT,
                                                           [0x8c] = USBKB_KEYPAD,
                                                           [0x8d ... 0xff] = USBKB_RAWDAT,
                                                       },
                                                       {
                                                           [0x01 ... 0x03] = USBKB_RAWDAT,
                                                           [0x04] = 0xc1,
                                                           0xba,
                                                           0xbf,
                                                           0xbc,
                                                           0xa8,
                                                           0xca,
                                                           0xb7,
                                                           0xb8,
                                                           0xc6,
                                                           0xcf,
                                                           0xc9,
                                                           0xd8,
                                                           0xd3,
                                                           0xd0,
                                                           0xd7,
                                                           0xbe,
                                                           0xc0,
                                                           0xbd,
                                                           0xc4,
                                                           0xb6,
                                                           0xc5,
                                                           0xcb,
                                                           0xc3,
                                                           0xbb,
                                                           0xdd,
                                                           0xaf,
                                                           0xc7,
                                                           0xcc,
                                                           0xa7,
                                                           0xa9,
                                                           0xaa,
                                                           0xab,
                                                           0xac,
                                                           0xad,
                                                           0xae,
                                                           0xa6,
                                                           '\n',
                                                           USBKB_RAWDAT,
                                                           '\b',
                                                           '\t',
                                                           ' ',
                                                           0xce,
                                                           0xcd,
                                                           0xde,
                                                           0xa2,
                                                           0xb0,
                                                           0xa3,
                                                           0xda,
                                                           0xb9,
                                                           USBKB_RAWDAT,
                                                           0xa4,
                                                           0xa1,
                                                           0xa5,
                                                           [0x39 ... 0x52] = USBKB_RAWDAT,
                                                           [0x53 ... 0x63] = USBKB_KEYPAD,
                                                           [0x64] = '\\',
                                                           [0x65 ... 0x66] = USBKB_RAWDAT,
                                                           [0x67] = USBKB_KEYPAD,
                                                           [0x68 ... 0x84] = USBKB_RAWDAT,
                                                           [0x85 ... 0x86] = USBKB_KEYPAD,
                                                           [0x87] = 0xdb,
                                                           [0x88] = USBKB_RAWDAT,
                                                           [0x89] = 0xb0,
                                                           [0x8a ... 0x8b] = USBKB_RAWDAT,
                                                           [0x8c] = USBKB_KEYPAD,
                                                           [0x8d ... 0xff] = USBKB_RAWDAT,
                                                       },
                                                   },
};

// NTSC-U/C: 0x00431ce8, PAL: 0x004ce998, the keypad keys with Num Lock off and on.
static const UsbKbKeypadEntry g_aUsbKbKeypadTable[kNumLockStateCount][kKeypadEntryCount] = {
    {
        {0x53, USBKB_RAWDAT | 0x53},
        {0x54, '/'},
        {0x55, '*'},
        {0x56, '-'},
        {0x57, '+'},
        {0x58, '\n'},
        {0x59, USBKB_RAWDAT | 0x4d},
        {0x5a, USBKB_RAWDAT | 0x51},
        {0x5b, USBKB_RAWDAT | 0x4e},
        {0x5c, USBKB_RAWDAT | 0x50},
        {0x5d, USBKB_RAWDAT | 0x5d},
        {0x5e, USBKB_RAWDAT | 0x4f},
        {0x5f, USBKB_RAWDAT | 0x4a},
        {0x60, USBKB_RAWDAT | 0x52},
        {0x61, USBKB_RAWDAT | 0x4b},
        {0x62, USBKB_RAWDAT | 0x49},
        {0x63, USBKB_RAWDAT | 0x4c},
        {0x67, '='},
        {0x85, ','},
        {0x86, '='},
        {0x8c, ','},
    },
    {
        {0x53, USBKB_RAWDAT | 0x53},
        {0x54, '/'},
        {0x55, '*'},
        {0x56, '-'},
        {0x57, '+'},
        {0x58, '\n'},
        {0x59, '1'},
        {0x5a, '2'},
        {0x5b, '3'},
        {0x5c, '4'},
        {0x5d, '5'},
        {0x5e, '6'},
        {0x5f, '7'},
        {0x60, '8'},
        {0x61, '9'},
        {0x62, '0'},
        {0x63, '.'},
        {0x67, '='},
        {0x85, ','},
        {0x86, '='},
        {0x8c, ','},
    },
};

// NTSC-U/C: 0x00310cf0, PAL: 0x0037d2a0
static void rpccall_end(void *pData) {
    (void)pData;
    iSignalSema(g_nUsbKbSemaId);
}

// NTSC-U/C: 0x00311450, PAL: 0x0037da00
static int clear_repbuf(int nKeyboard) {
    UsbKbRepeat *pRepeat = &g_pUsbKbStateTable[nKeyboard].repeat;

    pRepeat->nCounter = 0;
    pRepeat->nCount = 0;
    pRepeat->nWrite = 0;
    pRepeat->nRead = 0;
    return 0;
}

// NTSC-U/C: 0x00311480, PAL: 0x0037da30
static int push_repbuf(int nKeyboard, unsigned short nKeyCode) {
    UsbKbRepeat *pRepeat = &g_pUsbKbStateTable[nKeyboard].repeat;

    if (pRepeat->nCount >= kRepeatBufferSize) {
        return -1;
    }
    pRepeat->anBuffer[pRepeat->nWrite++] = nKeyCode;
    if (pRepeat->nWrite >= kRepeatBufferSize) {
        pRepeat->nWrite = 0;
    }
    ++pRepeat->nCount;
    return 0;
}

// NTSC-U/C: 0x003114f0, PAL: 0x0037daa0
static int pop_repbuf(int nKeyboard, unsigned short *pKeyCode) {
    UsbKbRepeat *pRepeat = &g_pUsbKbStateTable[nKeyboard].repeat;

    if (pRepeat->nCount <= 0) {
        return -1;
    }
    *pKeyCode = pRepeat->anBuffer[pRepeat->nRead++];
    if (pRepeat->nRead >= kRepeatBufferSize) {
        pRepeat->nRead = 0;
    }
    --pRepeat->nCount;
    return 0;
}

// NTSC-U/C: 0x00311800, PAL: 0x0037ddb0
static unsigned short cnv_keypad(unsigned char nLed, unsigned char nRawCode) {
    const UsbKbKeypadEntry *pTable = g_aUsbKbKeypadTable[nLed & kLedNumLock];
    int i;

    for (i = 0; i < kKeypadEntryCount; ++i) {
        if (pTable[i].nRawCode == nRawCode) {
            return pTable[i].nKeyCode | USBKB_KEYPAD;
        }
    }
    return nRawCode;
}

unsigned short sceUsbKbCnvRawCode(int arrange, int mkey, int led, unsigned short rawcode) {
    const unsigned short *pTable = NULL;
    int nShift = (mkey & (kModifierLeftShift | kModifierRightShift)) != 0;
    unsigned short nKeyCode;

    switch (arrange) {
    case kArrangement101:
        pTable = g_awUsbKbAsciiTable[kArrangement101][nShift];
        break;
    case kArrangement106:
        pTable = g_awUsbKbAsciiTable[kArrangement106][nShift];
        break;
    case kArrangement106Kana:
        pTable = g_awUsbKbAsciiTable[kArrangement106Kana][nShift];
        break;
    default:
        scePrintf("Arrangement error!\n");
        break;
    }
    if (rawcode == 0) {
        return 0;
    }
    if (rawcode >= kKeyCodeCount) {
        return rawcode;
    }
    nKeyCode = pTable[rawcode]; // Yes, the binary reads a null table after an arrangement error.
    if (nKeyCode == USBKB_KEYPAD) {
        nKeyCode = cnv_keypad((unsigned char)led, (unsigned char)rawcode);
    }
    if (nKeyCode == USBKB_RAWDAT) {
        nKeyCode = rawcode | USBKB_RAWDAT;
    }
    if (nKeyCode >= 'a' && nKeyCode <= 'z' && (led & kLedCapsLock) != 0) {
        nKeyCode -= kCaseOffset;
    }
    if (nKeyCode >= 'A' && nKeyCode <= 'Z' && (led & kLedCapsLock) != 0 && nShift != 0) {
        nKeyCode += kCaseOffset;
    }
    return nKeyCode;
}

// NTSC-U/C: 0x00311558, PAL: 0x0037db08
// Converts the codes of a report up to the first empty slot and returns how many it converted.
static int cnv_keycode(int nKeyboard,
                       unsigned char nMkey,
                       unsigned char nLed,
                       int nLen,
                       const unsigned short *pSrc,
                       unsigned short *pDst) {
    const UsbKbStatus *pStatus = &g_pUsbKbStateTable[nKeyboard];
    int i = 0;

    if (pStatus->nCodeType == kCodeTypeRaw) {
        for (i = 0; i < nLen && pSrc[i] != 0; ++i) {
            pDst[i] = pSrc[i];
        }
    } else if (pStatus->nCodeType == kCodeTypeAscii) {
        for (i = 0; i < nLen && pSrc[i] != 0; ++i) {
            pDst[i] = sceUsbKbCnvRawCode(pStatus->nArrangement, nMkey, nLed, pSrc[i]);
        }
    }
    return i;
}

// NTSC-U/C: 0x00310d68, PAL: 0x0037d318
static int SetResult_GetInfo(void) {
    USBKBINFO_t *pInfo = g_pUsbKbRpcUserBuffer;
    int i;

    pInfo->max_connect = g_UsbKbRpcRecvBuffer.info.nMaxConnect;
    pInfo->now_connect = g_UsbKbRpcRecvBuffer.info.nNowConnect;
    for (i = 0; i < pInfo->max_connect; ++i) {
        pInfo->status[i] = g_UsbKbRpcRecvBuffer.info.abStatus[i];
    }
    return 0;
}

// NTSC-U/C: 0x00310dc0, PAL: 0x0037d370
static int SetResult_Read(void) {
    const UsbKbReply *pReply = &g_UsbKbRpcRecvBuffer;
    USBKBDATA_t *pData = g_pUsbKbRpcUserBuffer;
    UsbKbRepeat *pRepeat = &g_pUsbKbStateTable[g_nUsbKbRpcArg].repeat;
    USBKBDATA_t current;
    unsigned short anKeyCodes[USBKB_MAX_KEYCODES];
    unsigned short nKeyCode;
    int nCount;
    int i;
    int j;

    if (pReply->read.nLen == kNoReport) {
        return -1;
    }
    if (g_pUsbKbStateTable[g_nUsbKbRpcArg].nReadMode == kReadModePacket) {
        pData->led = pReply->read.nLed;
        pData->len = pReply->read.nLen;
        pData->mkey = pReply->read.nMkey;
        if (pReply->read.nLen == 0) {
            return 0;
        }
        nCount = 0;
        for (i = 0; i < pReply->read.nLen; ++i) {
            if (pReply->read.abKeyCode[i] != 0) {
                pData->keycode[nCount++] = pReply->read.abKeyCode[i];
            }
        }
        pData->len = nCount;
        if (nCount == 0) {
            pData->keycode[0] = 0;
            pData->len = 1;
        }
        cnv_keycode((int)g_nUsbKbRpcArg,
                    (unsigned char)pData->mkey,
                    (unsigned char)pData->led,
                    pData->len,
                    pData->keycode,
                    pData->keycode);
        return 0;
    }

    if (pRepeat->nState == kRepeatFirst) {
        if (pRepeat->nCounter >= pRepeat->nStart) {
            pRepeat->nCounter = 0;
            clear_repbuf((int)g_nUsbKbRpcArg);
            if (pRepeat->nStart >= 2) {
                push_repbuf((int)g_nUsbKbRpcArg, 0);
            }
            push_repbuf((int)g_nUsbKbRpcArg, pRepeat->nKeyCode);
            if (pRepeat->nInterval >= 2) {
                push_repbuf((int)g_nUsbKbRpcArg, 0);
            }
            pRepeat->nState = kRepeatNext;
        }
    } else if (pRepeat->nState == kRepeatNext) {
        if (pRepeat->nCounter >= pRepeat->nInterval) {
            pRepeat->nCounter = 0;
            push_repbuf((int)g_nUsbKbRpcArg, pRepeat->nKeyCode);
            if (pRepeat->nInterval >= 2) {
                push_repbuf((int)g_nUsbKbRpcArg, 0);
            }
        }
    }

    if (pReply->read.nLen == 0) {
        if (pop_repbuf((int)g_nUsbKbRpcArg, &nKeyCode) != 0) {
            pData->len = 0;
            return 0;
        }
        pData->len = 1;
        pData->keycode[0] = nKeyCode;
        pData->led = g_UsbKbLastKeyState.led;
        pData->mkey = g_UsbKbLastKeyState.mkey;
        return 0;
    }
    if (pReply->read.abKeyCode[0] == kKeyErrorRollOver) {
        pRepeat->nState = kRepeatIdle;
        pData->len = 0;
        return 0;
    }

    current.led = pReply->read.nLed;
    current.mkey = pReply->read.nMkey;
    current.len = pReply->read.nLen;
    nCount = 0;
    for (i = 0; i < current.len; ++i) {
        if (pReply->read.abKeyCode[i] != 0) {
            current.keycode[nCount++] = pReply->read.abKeyCode[i];
        }
    }
    current.len = nCount;
    clear_repbuf((int)g_nUsbKbRpcArg);

    // Yes, the binary reads the first code even when the report has none.
    if (current.keycode[0] == 0 || current.mkey != g_UsbKbLastKeyState.mkey) {
        pData->keycode[0] = 0;
        pData->len = 1;
        pRepeat->nKeyCode = 0;
        pData->led = current.led;
        pData->mkey = current.mkey;
        pRepeat->nState = kRepeatIdle;
    } else {
        if (g_UsbKbLastKeyState.keycode[0] != 0) {
            // Report only the keys the previous report did not have.
            nCount = 0;
            for (i = 0; i < current.len; ++i) {
                for (j = 0; j < g_UsbKbLastKeyState.len; ++j) {
                    if (current.keycode[i] == g_UsbKbLastKeyState.keycode[j]) {
                        break;
                    }
                }
                if (j == g_UsbKbLastKeyState.len) {
                    pData->keycode[nCount++] = current.keycode[i];
                }
            }
            pData->len = nCount;
        } else {
            for (i = 0; i < current.len; ++i) {
                pData->keycode[i] = current.keycode[i];
            }
            pData->len = i;
        }
        pData->led = current.led;
        pData->mkey = current.mkey;
        // Yes, the binary discards the count of converted codes.
        cnv_keycode((int)g_nUsbKbRpcArg,
                    (unsigned char)pData->mkey,
                    (unsigned char)pData->led,
                    pData->len,
                    pData->keycode,
                    pData->keycode);
        if (pData->len > 0) {
            pRepeat->nKeyCode = pData->keycode[pData->len - 1];
            if (pRepeat->nStart > 0 && pRepeat->nInterval > 0) {
                pRepeat->nState = kRepeatFirst;
            }
        } else {
            // No new key went down. Stop the repeat once its key is released.
            cnv_keycode((int)g_nUsbKbRpcArg,
                        (unsigned char)current.mkey,
                        (unsigned char)current.led,
                        current.len,
                        current.keycode,
                        anKeyCodes);
            for (i = 0; i < current.len; ++i) {
                if (anKeyCodes[i] == pRepeat->nKeyCode) {
                    break;
                }
            }
            if (i == current.len) {
                pRepeat->nState = kRepeatIdle;
                pData->len = 1;
                pData->keycode[0] = 0;
                pData->led = current.led;
                pData->mkey = current.mkey;
            }
        }
    }
    g_UsbKbLastKeyState = current;
    return 0;
}

// NTSC-U/C: 0x00311338, PAL: 0x0037d8e8
static int SetResult_GetLocation(void) {
    char *pLocation = g_pUsbKbRpcUserBuffer;
    int i;

    if (g_UsbKbRpcRecvBuffer.szLocation[kLocationSize - 1] != '\0') {
        return -1;
    }
    for (i = 0; i < kLocationSize; ++i) {
        pLocation[i] = g_UsbKbRpcRecvBuffer.szLocation[i];
    }
    return 0;
}

// NTSC-U/C: 0x00311388, PAL: 0x0037d938
static int SetResult_SetLEDStatus(void) {
    if (g_UsbKbRpcRecvBuffer.led.nValue != g_UsbKbRpcSendBuffer.nValue) {
        return -1;
    }
    return 0;
}

// NTSC-U/C: 0x003113a8, PAL: 0x0037d958
static int SetResult_SetLEDMode(void) {
    if (g_UsbKbRpcRecvBuffer.led.nValue != g_UsbKbRpcSendBuffer.nValue) {
        return -1;
    }
    return 0;
}

// NTSC-U/C: 0x003c2a90, PAL: 0x00458ed0, the result handlers by request.
static int (*g_apfnUsbKbResultHandlers[])(void) = {
    [kRpcGetInfo] = SetResult_GetInfo,
    [kRpcRead] = SetResult_Read,
    [kRpcGetLocation] = SetResult_GetLocation,
    [kRpcSetLedStatus] = SetResult_SetLEDStatus,
    [kRpcSetLedMode] = SetResult_SetLEDMode,
};

// NTSC-U/C: 0x00310d00, PAL: 0x0037d2b0
static int DispatchUsbKbRpcResult(void) {
    int nResult = 0;

    if (g_nUsbKbRpcCommand >= kRpcGetInfo && g_nUsbKbRpcCommand <= kRpcSetLedMode) {
        nResult = g_apfnUsbKbResultHandlers[g_nUsbKbRpcCommand]();
    }
    g_pUsbKbRpcUserBuffer = NULL;
    g_nUsbKbRpcCommand = 0;
    return nResult;
}

// NTSC-U/C: 0x003113c8, PAL: 0x0037d978
// Counts the vertical blanks of every keyboard with a key repeating.
static int vblank_e_handler(int nCause) {
    UsbKbRepeat *pRepeat;
    int i;

    (void)nCause;
    for (i = 0; i < g_nUsbKbDeviceCount; ++i) {
        pRepeat = &g_pUsbKbStateTable[i].repeat;
        if (pRepeat->nStart > 0 && pRepeat->nInterval > 0 && pRepeat->nState > kRepeatIdle &&
            pRepeat->nState < kRepeatStateCount) {
            ++pRepeat->nCounter;
        }
    }
    ExitHandler();
    return 0;
}

int sceUsbKbInit(int *pMaxConnect) {
    struct SemaParam sema; // Yes, the binary sets only two of the fields.
    USBKBINFO_t info;
    UsbKbStatus *pStatus;
    int nResult = 0; // sceUsbKbSync() always writes it, but the compiler cannot prove so.
    int nDelay;
    unsigned int i;

    sceSifInitRpc(0);
    for (;;) {
        if (sceSifBindRpc(&g_UsbKbRpcClientData, kUsbKbRpcServer, 0) < 0) {
            scePrintf("libusbkb : bind errr\n");
            return -1;
        }
        if (g_UsbKbRpcClientData.serve != NULL) {
            break;
        }
        for (nDelay = kBindDelay; nDelay != -1; --nDelay) {
            __asm__ volatile("nop\n\tnop\n\tnop\n\tnop");
        }
    }

    sema.maxCount = 1;
    sema.initCount = 1;
    g_nUsbKbSemaId = CreateSema(&sema);
    if (g_nUsbKbSemaId == -1) {
        scePrintf("libusbkb : CreateSema Error\n");
        return -1;
    }
    if (sceUsbKbGetInfo(&info) != 0) {
        DeleteSema(g_nUsbKbSemaId);
        g_nUsbKbSemaId = -1;
        return -1;
    }
    sceUsbKbSync(kSyncWait, &nResult);
    if (nResult != 0 || info.max_connect <= 0 || info.max_connect > USBKB_MAX_STATUS) {
        DeleteSema(g_nUsbKbSemaId);
        g_nUsbKbSemaId = -1;
        return -1;
    }
    g_nUsbKbDeviceCount = info.max_connect;
    *pMaxConnect = info.max_connect;
    g_pUsbKbStateTable = g_aUsbKbState;
    if (g_pUsbKbStateTable == NULL) {
        DeleteSema(g_nUsbKbSemaId);
        g_nUsbKbSemaId = -1;
        return -1;
    }
    for (i = 0; i < (unsigned int)info.max_connect; ++i) {
        pStatus = &g_pUsbKbStateTable[i];
        pStatus->nArrangement = kDefaultArrangement;
        pStatus->repeat.nCount = 0;
        pStatus->nLedMode = kDefaultLedMode;
        pStatus->nCodeType = kDefaultCodeType;
        pStatus->nReadMode = kReadModeInputChar;
        pStatus->repeat.nState = kRepeatIdle;
        pStatus->repeat.nCounter = 0;
        pStatus->repeat.nStart = 0;
        pStatus->repeat.nInterval = 0;
        pStatus->repeat.nWrite = 0;
        pStatus->repeat.nRead = 0;
        pStatus->repeat.nKeyCode = 0;
    }
    g_UsbKbLastKeyState.len = 0;
    g_nUsbKbIntcHandlerId = AddIntcHandler(INTC_VBLANK_E, vblank_e_handler, -1);
    if (g_nUsbKbIntcHandlerId < 0) {
        DeleteSema(g_nUsbKbSemaId);
        g_nUsbKbSemaId = -1;
        return -1;
    }
    EnableIntc(INTC_VBLANK_E);
    return 0;
}

int sceUsbKbEnd(void) {
    int nErrors = 0;
    int nInterrupts = DIntr();
    int nHandlers = RemoveIntcHandler(INTC_VBLANK_E, g_nUsbKbIntcHandlerId);

    if (nHandlers == 0) {
        DisableIntc(INTC_VBLANK_E);
    }
    if (nInterrupts != 0) {
        EIntr();
    }
    if (nHandlers < 0) {
        scePrintf("libusbkb.a : RemoveIntcHandler() --> error\n");
        nErrors = 1;
    }
    if (g_nUsbKbSemaId != -1) {
        if (DeleteSema(g_nUsbKbSemaId) >= 0) {
            g_nUsbKbSemaId = -1;
        } else {
            scePrintf("libusbkb.a : DeleteSema() --> error\n");
            ++nErrors;
        }
    }
    return nErrors != 0 ? -1 : 0;
}

int sceUsbKbGetInfo(USBKBINFO_t *pInfo) {
    int nResult;

    if (g_nUsbKbSemaId == -1) {
        return kErrorNotReady;
    }
    WaitSema(g_nUsbKbSemaId);
    nResult = sceSifCallRpc(&g_UsbKbRpcClientData,
                            kRpcGetInfo,
                            SIF_RPCM_NOWAIT,
                            &g_UsbKbRpcSendBuffer,
                            kSendSize,
                            &g_UsbKbRpcRecvBuffer,
                            kGetInfoReplySize,
                            rpccall_end,
                            NULL);
    if (nResult != 0) {
        // Yes, the binary returns without releasing the semaphore.
        scePrintf("sceUsbKbGetInfo: sceSifCallRpc -> %d\n", nResult);
        return kErrorRpc;
    }
    g_pUsbKbRpcUserBuffer = pInfo;
    g_nUsbKbRpcCommand = kRpcGetInfo;
    return 0;
}

int sceUsbKbRead(unsigned int nNo, USBKBDATA_t *pData) {
    int nResult;

    if (g_nUsbKbSemaId == -1) {
        return kErrorNotReady;
    }
    if (nNo >= (unsigned int)g_nUsbKbDeviceCount) {
        return kErrorArgument1;
    }
    g_UsbKbRpcSendBuffer.nKeyboard = (unsigned char)nNo;
    WaitSema(g_nUsbKbSemaId);
    nResult = sceSifCallRpc(&g_UsbKbRpcClientData,
                            kRpcRead,
                            SIF_RPCM_NOWAIT,
                            &g_UsbKbRpcSendBuffer,
                            kSendSize,
                            &g_UsbKbRpcRecvBuffer,
                            kReadReplySize,
                            rpccall_end,
                            NULL);
    if (nResult != 0) {
        scePrintf("sceUsbKbRead: sceSifCallRpc -> %d\n", nResult);
        return kErrorRpc;
    }
    g_pUsbKbRpcUserBuffer = pData;
    g_nUsbKbRpcCommand = kRpcRead;
    g_nUsbKbRpcArg = nNo;
    return 0;
}

int sceUsbKbSetLEDMode(unsigned int nNo, int nMode) {
    int nResult;

    if (g_nUsbKbSemaId == -1) {
        return kErrorNotReady;
    }
    if ((int)nNo >= g_nUsbKbDeviceCount) {
        return kErrorArgument1;
    }
    if ((unsigned int)nMode >= kLedModeCount) {
        return kErrorArgument2;
    }
    g_pUsbKbStateTable[nNo].nLedMode = nMode;
    g_UsbKbRpcSendBuffer.nKeyboard = (unsigned char)nNo;
    g_UsbKbRpcSendBuffer.nValue = (unsigned char)nMode;
    WaitSema(g_nUsbKbSemaId);
    nResult = sceSifCallRpc(&g_UsbKbRpcClientData,
                            kRpcSetLedMode,
                            SIF_RPCM_NOWAIT,
                            &g_UsbKbRpcSendBuffer,
                            kSendSize,
                            &g_UsbKbRpcRecvBuffer,
                            kSetLedModeReplySize,
                            rpccall_end,
                            NULL);
    if (nResult != 0) {
        scePrintf("sceUsbKbSetLEDMode: sceSifCallRpc -> %d\n", nResult);
        return kErrorRpc;
    }
    g_nUsbKbRpcArg = nNo;
    g_nUsbKbRpcCommand = kRpcSetLedMode;
    g_pUsbKbRpcUserBuffer = NULL;
    return 0;
}

int sceUsbKbSetRepeat(unsigned int nNo, int nStart, int nInterval) {
    UsbKbRepeat *pRepeat;

    // Yes, the binary checks the start delay where the other setters check the keyboard.
    if (nStart < 0) {
        return kErrorArgument1;
    }
    if (nInterval < 0) {
        return kErrorArgument2;
    }
    pRepeat = &g_pUsbKbStateTable[nNo].repeat;
    pRepeat->nInterval = nInterval;
    pRepeat->nStart = nStart;
    pRepeat->nCounter = 0;
    return 0;
}

int sceUsbKbSetCodeType(unsigned int nNo, int nType) {
    if ((int)nNo >= g_nUsbKbDeviceCount) {
        return kErrorArgument1;
    }
    if ((unsigned int)nType >= kCodeTypeCount) {
        return kErrorArgument2;
    }
    g_pUsbKbStateTable[nNo].nCodeType = nType;
    return 0;
}

int sceUsbKbSetArrangement(unsigned int nNo, int nArrangement) {
    if ((int)nNo >= g_nUsbKbDeviceCount) {
        return kErrorArgument1;
    }
    if ((unsigned int)nArrangement >= kArrangementCount) {
        return kErrorArgument2;
    }
    g_pUsbKbStateTable[nNo].nArrangement = nArrangement;
    return 0;
}

int sceUsbKbSync(int nMode, int *pResult) {
    if (g_nUsbKbSemaId == -1) {
        return kErrorNotReady;
    }
    if (nMode == kSyncWait) {
        WaitSema(g_nUsbKbSemaId);
        *pResult = DispatchUsbKbRpcResult();
        SignalSema(g_nUsbKbSemaId);
        return 0;
    }
    if (nMode == kSyncPoll) {
        if (sceSifCheckStatRpc(&g_UsbKbRpcClientData.rpcd) != 0) {
            return kSyncBusy;
        }
        *pResult = DispatchUsbKbRpcResult();
        return 0;
    }
    return kErrorArgument1;
}

int sceUsbKbSetReadMode(unsigned int nNo, int nMode) {
    UsbKbStatus *pStatus;

    if ((int)nNo >= g_nUsbKbDeviceCount) {
        return kErrorArgument1;
    }
    if ((unsigned int)nMode >= kReadModeCount) {
        return kErrorArgument2;
    }
    pStatus = &g_pUsbKbStateTable[nNo];
    if (nMode != pStatus->nReadMode) {
        pStatus->repeat.nCounter = 0;
        pStatus->repeat.nWrite = 0;
        pStatus->repeat.nRead = 0;
        g_UsbKbLastKeyState.len = 0;
        pStatus->repeat.nKeyCode = 0;
    }
    pStatus->nReadMode = nMode;
    return 0;
}

int sceUsbKbClearRbuf(unsigned int nNo) {
    UsbKbStatus *pStatus;
    int nResult;

    if (g_nUsbKbSemaId == -1) {
        return kErrorNotReady;
    }
    if (nNo >= (unsigned int)g_nUsbKbDeviceCount) {
        return kErrorArgument1;
    }
    g_UsbKbRpcSendBuffer.nKeyboard = (unsigned char)nNo;
    WaitSema(g_nUsbKbSemaId);
    nResult = sceSifCallRpc(&g_UsbKbRpcClientData,
                            kRpcClearRbuf,
                            SIF_RPCM_NOWAIT,
                            &g_UsbKbRpcSendBuffer,
                            kSendSize,
                            &g_UsbKbRpcRecvBuffer,
                            0,
                            rpccall_end,
                            NULL);
    if (nResult != 0) {
        // Yes, the binary reports the failure under the name of sceUsbKbRead().
        scePrintf("sceUsbKbRead: sceSifCallRpc -> %d\n", nResult);
        return kErrorRpc;
    }
    pStatus = &g_pUsbKbStateTable[nNo];
    pStatus->repeat.nCount = 0;
    pStatus->repeat.nState = kRepeatIdle;
    pStatus->repeat.nCounter = 0;
    pStatus->repeat.nKeyCode = 0;
    pStatus->repeat.nWrite = 0;
    pStatus->repeat.nRead = 0;
    return 0;
}
