#include "dmadebug/vif.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dmadebug/debug.h"
#include "dmadebug/dma.h"
#include "dmadebug/gif.h"

// Indices of the VIF commands in g_apszVifCommandNames.
enum {
    VIF_NOP = 0,
    VIF_STCYCL = 1,
    VIF_OFFSET = 2,
    VIF_BASE = 3,
    VIF_ITOP = 4,
    VIF_STMOD = 5,
    VIF_MSKPATH3 = 6,
    VIF_MARK = 7,
    VIF_FLUSHE = 8,
    VIF_FLUSH = 9,
    VIF_FLUSHA = 10,
    VIF_MSCAL = 11,
    VIF_MSCNT = 12,
    VIF_MSCALF = 13,
    VIF_STMASK = 14,
    VIF_STROW = 15,
    VIF_STCOL = 16,
    VIF_MPG = 17,
    VIF_DIRECT = 18,
    VIF_DIRECTHL = 19,
    VIF_UNPACK = 20,
    VIF_COMMAND_COUNT = 21,
};

// Fields of the command byte of a VIF code.
enum {
    VIF_CMD_UNPACK_BITS = 0x60,
    VIF_CMD_MASK = 0x7f,
    VIF_CMD_UNPACK_MASKED = 0x10,
    VIF_CMD_UNPACK_FORMAT = 0xf,
    VIF_CMD_VL_MASK = 3,
    VIF_CMD_VN_SHIFT = 2,
    VIF_CMD_VN_MASK = 3,
    VIF_CMD_IRQ_SHIFT = 7,
};

// Fields of the immediate of a VIF code.
enum {
    VIF_IMMEDIATE_ADDRESS_MASK = 0x1ff,
    VIF_IMMEDIATE_FLG_SHIFT = 15,
    VIF_IMMEDIATE_USN_SHIFT = 14,
    VIF_IMMEDIATE_R = 0x8000,
    VIF_IMMEDIATE_U = 0x4000,
    VIF_DIRECT_DEFAULT_SIZE = 0x10000,
    VIF_NUM_DEFAULT = 256,
};

// Indices of the UNPACK formats in g_apszVifUnpackNames.
enum {
    VIF_UNPACK_V4_5 = 12,
    VIF_UNPACK_FORMAT_COUNT = 13,
};

// The element size an UNPACK grab reads, as a power of two of its count in a word.
enum {
    UNPACK_ELEMENT_32 = 0,
    UNPACK_ELEMENT_16 = 1,
    UNPACK_ELEMENT_8 = 2,
    UNPACK_ELEMENT_V4_5 = 3,
};

// Values of the MODE register.
enum {
    VIF_MODE_NONE = 0,
    VIF_MODE_OFFSET = 1,
    VIF_MODE_DIFFERENCE = 2,
    VIF_MODE_MASK = 3,
};

// Values of a two-bit field of the MASK register.
enum {
    VIF_MASK_DATA = 0,
    VIF_MASK_ROW = 1,
    VIF_MASK_COLUMN = 2,
    VIF_MASK_PROTECT = 3,
    VIF_MASK_FIELD_BITS = 2,
};

// The DBF bit of VIF1_STAT, the double buffer selection.
enum { VIF_STAT_DBF = 0x80 };

// The last addressable quadword of VU0 and VU1 data memory.
enum {
    VU0_ADDRESS_MASK = 255,
    VU1_ADDRESS_MASK = 1023,
};

// Words in a quadword, and the index that marks the quadword as used up.
enum { QWORD_WORDS = 4 };

// Components of a vector.
enum { VECTOR_COMPONENTS = 4 };

// The last column the MASK register addresses.
enum { LAST_COLUMN = 3 };

// The data of a quadword moved with its DMA tag starts at its third word.
enum { TAG_DATA_WORD = 2 };

// Capacity of the report line of a VIF code.
enum { VIF_LINE_SIZE = 1024 };

// Capacity of the status bits of a VIF code.
enum { STATUS_SIZE = 8 };

// The double word alignment that MPG data requires, as the remainder of the code address.
enum {
    MPG_ALIGNMENT_MASK = 7,
    MPG_CODE_OFFSET = 4,
};

// Message numbers.
enum {
    MESSAGE_PREMATURE_END = 300,
    MESSAGE_UNKNOWN_FORMAT = 310,
    MESSAGE_MPG_MISALIGNED = 320,
    MESSAGE_UNKNOWN_CODE = 340,
    MESSAGE_WRONG_CHANNEL = 350,
    MESSAGE_NOT_VIF = 360,
    MESSAGE_MODE_UNDEFINED = 380,
    MESSAGE_WL_ZERO = 390,
    MESSAGE_UNPACK_INCOMPLETE = 400,
    MESSAGE_MPG_DATA = 410,
    MESSAGE_ROW_COLUMN_DATA = 420,
    MESSAGE_MASK_DATA = 430,
    MESSAGE_GIF_MISALIGNED = 740,
    MESSAGE_INDETERMINATE = 3000,
    MESSAGE_GIF_INCOMPLETE = 3010,
    MESSAGE_ADDRESS_WRAP = 3020,
};

// The registers of a VIF unit.
struct vif_registers {
    unsigned int col[VECTOR_COMPONENTS]; // COL0 to COL3.
    unsigned int row[VECTOR_COMPONENTS]; // ROW0 to ROW3.
    unsigned int cycle;                  // CYCLE, CL in the low byte and WL in the next.
    unsigned int mask;                   // MASK.
    unsigned int mode;                   // MODE.
    unsigned int reserved44;             // +0x2c, not yet identified.
    unsigned int itops;                  // ITOPS.
    unsigned int mark;                   // MARK.
    unsigned int reserved56;             // +0x38, not yet identified.
};

// The VIF code being decoded.
struct vif_code {
    const unsigned int *code; // The code.
    unsigned char cmd;        // The command byte.
    int commandIndex;         // Index in g_apszVifCommandNames, or -1.
    signed char irq;          // The interrupt bit.
    int unpackFormat;         // Index in g_apszVifUnpackNames, or -1, for UNPACK.
    int num;                  // NUM, 256 when the code stores 0.
    unsigned int immediate;   // IMMEDIATE, 0x10000 for a DIRECT of 0.
    int length;               // Words of the code and its data not yet consumed.
};

// NTSC-U/C: 0x003b6098
// Non-zero when every code line is printed, not only lines with errors.
static int g_nVifShowCodes = 1;

// NTSC-U/C: 0x003b609c
// Non-zero when code lines give word counts and VU addresses.
static int g_nVifVerbose = 1;

// NTSC-U/C: 0x003b60a0
// Non-zero when the end of a transfer reports the VIF registers.
static int g_nVifShowRegisters = 1;

// NTSC-U/C: 0x003b60a4
// Non-zero when the end of a transfer reports the code counts.
static int g_nVifShowStatistics = 1;

// NTSC-U/C: 0x003b60a8
// Non-zero when the GIF packets of DIRECT and DIRECTHL are decoded.
static int g_nVifDisassembleDirect = 1;

// NTSC-U/C: 0x003b60ac
// Non-zero when the VU memory writes of UNPACK are simulated.
static int g_nVifDisassembleUnpack = 1;

// NTSC-U/C: 0x003b60b0
// Names of the VIF commands.
static const char *g_apszVifCommandNames[VIF_COMMAND_COUNT] = {
    "NOP",    "STCYCL", "OFFSET", "BASE",   "ITOP",   "STMOD",    "MSKPATH3",
    "MARK",   "FLUSHE", "FLUSH",  "FLUSHA", "MSCAL",  "MSCNT",    "MSCALF",
    "STMASK", "STROW",  "STCOL",  "MPG",    "DIRECT", "DIRECTHL", "UNPACK"};

// NTSC-U/C: 0x003b6108
// Names of the UNPACK formats.
static const char *g_apszVifUnpackNames[VIF_UNPACK_FORMAT_COUNT] = {"S_32",
                                                                    "S_16",
                                                                    "S_8",
                                                                    "V2_32",
                                                                    "V2_16",
                                                                    "V2_8",
                                                                    "V3_32",
                                                                    "V3_16",
                                                                    "V3_8",
                                                                    "V4_32",
                                                                    "V4_16",
                                                                    "V4_8",
                                                                    "V4_5"};

// NTSC-U/C: 0x003b6140
// The format bits of each UNPACK format.
static const signed char g_abVifUnpackFormatTable[VIF_UNPACK_FORMAT_COUNT] = {
    0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x08, 0x09, 0x0a, 0x0c, 0x0d, 0x0e, 0x0f};

// NTSC-U/C: 0x003b6150
// The command byte of each VIF command, with every UNPACK as 0x60.
static const signed char g_abVifCommandCodeTable[VIF_COMMAND_COUNT] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x13,
    0x14, 0x17, 0x15, 0x20, 0x30, 0x31, 0x4a, 0x50, 0x51, 0x60};

// NTSC-U/C: 0x00519a80
// The word of g_pDmaQword to decode next, or 4 when the quadword is used up.
static int g_nVifWordIndex;

// NTSC-U/C: 0x00519a90
// VIF1_STAT.
static unsigned int g_dwVif1Stat;

// NTSC-U/C: 0x00519a98
// VIF1 BASE.
static unsigned int g_dwVif1Base;

// NTSC-U/C: 0x00519a9c
// VIF1 OFFSET.
static unsigned int g_dwVif1Offset;

// NTSC-U/C: 0x00519aa4
// VIF1 TOPS, BASE plus OFFSET when DBF is set.
static unsigned int g_dwVif1Tops;

// NTSC-U/C: 0x00519aac
// The VIF unit being decoded, 0 or 1.
static int g_nVifUnit;

// NTSC-U/C: 0x00519ab0
// The registers of VIF0 and VIF1.
static struct vif_registers g_aVifRegisters[2];

// NTSC-U/C: 0x00519b28
// The VIF code being decoded.
static struct vif_code g_vifCode;

// NTSC-U/C: 0x00519b48
// Codes decoded, by command.
static int g_anVifCommandCounts[VIF_COMMAND_COUNT];

// NTSC-U/C: 0x00519ba0
// The vector the last UNPACK grab read.
static unsigned int g_adwVifUnpackVector[VECTOR_COMPONENTS];

// NTSC-U/C: 0x00519bb0
// Non-zero for each component of g_adwVifUnpackVector that the data supplied.
static int g_anVifUnpackValid[VECTOR_COMPONENTS];

// NTSC-U/C: 0x00519bc0
// The element of the current word an UNPACK grab reads next.
static int g_nVifElementIndex;

// Returns the float with the bits of a word.
static inline float wordToFloat(unsigned int word) {
    union {
        unsigned int word;
        float value;
    } bits;
    bits.word = word;
    return bits.value;
}

// NTSC-U/C: 0x002a5ab0, PAL: 0x002afa20
// Reports a message of the decoder, prefixed with "VIF: ".
static int report(int number, int severity, const char *text) {
    char message[DEBUG_SCRATCH_SIZE] = "VIF: ";
    strcat(message, text);
    return DEBUG_message(number, severity, message);
}

// NTSC-U/C: 0x002a5b10, PAL: 0x002afa80
// Reports the registers of the current unit.
static int dumpRegisters(void) {
    const struct vif_registers *registers = &g_aVifRegisters[g_nVifUnit];
    int result = DEBUG_print("\n==== VIF REGISTERS ====");
    for (int i = 0; i < VECTOR_COMPONENTS; ++i) {
        sprintf(g_szDebugScratch,
                "COL%d  : " DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET,
                i,
                registers->col[i]);
        result |= DEBUG_print(g_szDebugScratch);
    }
    for (int i = 0; i < VECTOR_COMPONENTS; ++i) {
        sprintf(g_szDebugScratch,
                "ROW%d  : " DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET,
                i,
                registers->row[i]);
        result |= DEBUG_print(g_szDebugScratch);
    }
    sprintf(
        g_szDebugScratch, "CYCLE : " DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET, registers->cycle);
    result |= DEBUG_print(g_szDebugScratch);
    sprintf(
        g_szDebugScratch, "MASK  : " DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET, registers->mask);
    result |= DEBUG_print(g_szDebugScratch);
    sprintf(
        g_szDebugScratch, "MODE  : " DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET, registers->mode);
    result |= DEBUG_print(g_szDebugScratch);
    sprintf(
        g_szDebugScratch, "MARK  : " DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET, registers->mark);
    result |= DEBUG_print(g_szDebugScratch);
    if (g_nVifUnit == 1) {
        sprintf(
            g_szDebugScratch, "BASE  : " DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET, g_dwVif1Base);
        result |= DEBUG_print(g_szDebugScratch);
        sprintf(g_szDebugScratch,
                "OFFSET: " DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET,
                g_dwVif1Offset);
        result |= DEBUG_print(g_szDebugScratch);
    }
    return result;
}

// NTSC-U/C: 0x002a5d00, PAL: 0x002afc70
// Reports the codes decoded by command, and their total.
static int dumpStatistics(void) {
    int total = 0;
    int result = DEBUG_print("\n==== VIF STATISTICS ====");
    result |= DEBUG_print("  Codes      # Codes");
    result |= DEBUG_print("  -----      -------");
    for (int i = 0; i < VIF_COMMAND_COUNT; ++i) {
        if (g_anVifCommandCounts[i] > 0) {
            sprintf(
                g_szDebugScratch, "  %-14s%4d", g_apszVifCommandNames[i], g_anVifCommandCounts[i]);
            total += g_anVifCommandCounts[i];
            result |= DEBUG_print(g_szDebugScratch);
        }
    }
    result |= DEBUG_print("               =====");
    sprintf(g_szDebugScratch, "  %14s%4d", "", total);
    return result | DEBUG_print(g_szDebugScratch);
}

// NTSC-U/C: 0x002a5e28, PAL: 0x002afd98
// Returns the index of the UNPACK format of a command byte, or -1.
static int lookupUnpackFormat(unsigned int cmd) {
    cmd &= VIF_CMD_UNPACK_FORMAT;
    for (int i = 0; i < VIF_UNPACK_FORMAT_COUNT; ++i) {
        if (cmd == (unsigned int)g_abVifUnpackFormatTable[i]) {
            return i;
        }
    }
    return -1;
}

// NTSC-U/C: 0x002a5e68, PAL: 0x002afdd8
// Returns the index of the command of a command byte, or -1.
static int lookupCommand(unsigned int cmd) {
    cmd &= VIF_CMD_MASK;
    if ((cmd & VIF_CMD_UNPACK_BITS) == VIF_CMD_UNPACK_BITS) {
        cmd = VIF_CMD_UNPACK_BITS;
    }
    for (int i = 0; i < VIF_COMMAND_COUNT; ++i) {
        if (cmd == (unsigned int)g_abVifCommandCodeTable[i]) {
            return i;
        }
    }
    return -1;
}

// NTSC-U/C: 0x002a5eb0, PAL: 0x002afe20
// Sets the words the current code and its data take.
static int computeLength(const struct vif_registers *registers) {
    int result = 0;
    int length;
    switch (g_vifCode.commandIndex) {
    case VIF_STMASK:
        length = 2;
        break;
    case VIF_STROW:
    case VIF_STCOL:
        length = 5;
        break;
    case VIF_MPG:
        length = g_vifCode.num * 2 + 1;
        break;
    case VIF_DIRECT:
    case VIF_DIRECTHL:
        length = (int)(g_vifCode.immediate * 4 + 1);
        break;
    case VIF_UNPACK:
        if (g_vifCode.unpackFormat == -1) {
            sprintf(g_szDebugScratch,
                    "Unknown UNPACK format '%x'.",
                    g_vifCode.cmd & VIF_CMD_UNPACK_FORMAT);
            result = report(MESSAGE_UNKNOWN_FORMAT, DEBUG_FATAL, g_szDebugScratch);
            length = 1;
        } else {
            const unsigned int vl = g_vifCode.cmd & VIF_CMD_VL_MASK;
            const unsigned int vn = (g_vifCode.cmd >> VIF_CMD_VN_SHIFT) & VIF_CMD_VN_MASK;
            int elementBytes;
            int components;
            if (vl == UNPACK_ELEMENT_V4_5) {
                elementBytes = 2;
                components = 1;
            } else {
                components = (int)vn + 1;
                elementBytes = 4 >> vl;
            }
            const unsigned int cl = registers->cycle & 0xff;
            const unsigned int wl = (registers->cycle >> 8) & 0xff;
            int bytes;
            if (cl >= wl) {
                bytes = elementBytes * components * g_vifCode.num;
            } else {
                const unsigned int cycles = (unsigned int)g_vifCode.num / wl;
                const unsigned int remainder = (unsigned int)g_vifCode.num % wl;
                const unsigned int written =
                    cl < remainder ? cl * cycles + cl : cl * cycles + remainder;
                bytes = elementBytes * components * (int)written;
            }
            length = (bytes + 3) / 4 + 1;
        }
        break;
    default:
        length = 1;
        break;
    }
    g_vifCode.length = length;
    return result;
}

// NTSC-U/C: 0x002a6068, PAL: 0x002affd8
// Decodes and reports the VIF code at code, and applies it to the registers.
static int decodeCode(const unsigned int *code, char *line) {
    struct vif_registers *registers = &g_aVifRegisters[g_nVifUnit];
    int result = 0;
    const unsigned int word = code[0];
    g_vifCode.cmd = word >> 24;
    g_vifCode.num = (word >> 16) & 0xff;
    g_vifCode.irq = (signed char)((word >> 24) >> VIF_CMD_IRQ_SHIFT);
    g_vifCode.immediate = word & 0xffff;
    g_vifCode.code = code;
    g_vifCode.commandIndex = lookupCommand(g_vifCode.cmd);
    if (g_vifCode.num == 0) {
        g_vifCode.num = VIF_NUM_DEFAULT;
    }
    if (g_vifCode.commandIndex == VIF_UNPACK) {
        g_vifCode.unpackFormat = lookupUnpackFormat(g_vifCode.cmd);
    }
    if ((g_vifCode.commandIndex == VIF_DIRECT || g_vifCode.commandIndex == VIF_DIRECTHL) &&
        g_vifCode.immediate == 0) {
        g_vifCode.immediate = VIF_DIRECT_DEFAULT_SIZE;
    }
    DEBUG_addField(DEBUG_FIELD_COLUMN,
                   line,
                   DEBUG_ANSI_ON_WHITE "VIF %-9s" DEBUG_ANSI_RESET " [" DEBUG_ANSI_CYAN
                                       "%08x" DEBUG_ANSI_RESET "]",
                   g_vifCode.commandIndex != -1 ? g_apszVifCommandNames[g_vifCode.commandIndex] :
                                                  DEBUG_ANSI_ALERT
                       "*UNKNOWN VIF COMMAND*" DEBUG_ANSI_RESET,
                   code);
    DEBUG_addField(
        DEBUG_FIELD_INLINE, line, "[" DEBUG_ANSI_MAGENTA "%08x" DEBUG_ANSI_RESET "]", word);
    if (g_vifCode.commandIndex == -1) {
        result = report(MESSAGE_UNKNOWN_CODE, DEBUG_FATAL, "Unknown VIF code");
    } else {
        ++g_anVifCommandCounts[g_vifCode.commandIndex];
    }
    if (g_vifCode.commandIndex == VIF_MPG &&
        ((unsigned int)g_vifCode.code & MPG_ALIGNMENT_MASK) != MPG_CODE_OFFSET) {
        result |= report(
            MESSAGE_MPG_MISALIGNED, DEBUG_ERROR, "MPG data is not on a 64 bit aligned address.");
    }
    if (g_nDmaChannel != DMA_CHANNEL_VIF1) {
        switch (g_vifCode.commandIndex) {
        case VIF_OFFSET:
        case VIF_BASE:
        case VIF_MSKPATH3:
        case VIF_FLUSH:
        case VIF_FLUSHA:
        case VIF_DIRECT:
        case VIF_DIRECTHL:
            sprintf(g_szDebugScratch,
                    "A VIF code was found (%s) that is not valid for this DMA channel.",
                    g_apszVifCommandNames[g_vifCode.commandIndex]);
            result |= report(MESSAGE_WRONG_CHANNEL, DEBUG_ERROR, g_szDebugScratch);
            break;
        }
    }
    if (result & (DEBUG_ERROR | DEBUG_FATAL)) {
        return result;
    }
    result |= computeLength(registers);
    if (result & (DEBUG_ERROR | DEBUG_FATAL)) {
        return result;
    }
    char status[STATUS_SIZE] = "[";
    if (g_vifCode.irq) {
        strcat(status, "i");
    }
    if (g_vifCode.commandIndex == VIF_UNPACK) {
        if (g_vifCode.cmd & VIF_CMD_UNPACK_MASKED) {
            strcat(status, "m");
        }
        if (g_vifCode.immediate & VIF_IMMEDIATE_R) {
            strcat(status, "r");
        }
        if (g_vifCode.immediate & VIF_IMMEDIATE_U) {
            strcat(status, "u");
        }
    }
    strcat(status, "]");
    if (strlen(status) != 2) {
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "Status bits=" DEBUG_ANSI_GREEN "%s" DEBUG_ANSI_RESET,
                       status);
    }
    switch (g_vifCode.commandIndex) {
    case VIF_NOP:
    case VIF_FLUSHE:
    case VIF_FLUSH:
    case VIF_FLUSHA:
    case VIF_STMASK:
    case VIF_STROW:
    case VIF_STCOL:
        break;
    case VIF_STCYCL:
        registers->cycle = g_vifCode.immediate & 0xffff;
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%02x" DEBUG_ANSI_RESET,
                       "WL",
                       (g_vifCode.immediate >> 8) & 0xff);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%02x" DEBUG_ANSI_RESET,
                       "CL",
                       g_vifCode.immediate & 0xff);
        break;
    case VIF_OFFSET:
        g_dwVif1Stat &= ~VIF_STAT_DBF;
        g_dwVif1Tops = g_dwVif1Base;
        g_dwVif1Offset = g_vifCode.immediate & VIF_IMMEDIATE_ADDRESS_MASK;
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%04x" DEBUG_ANSI_RESET,
                       "OFFSET",
                       g_vifCode.immediate & VIF_IMMEDIATE_ADDRESS_MASK);
        break;
    case VIF_BASE:
        g_dwVif1Base = g_vifCode.immediate & VIF_IMMEDIATE_ADDRESS_MASK;
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%04x" DEBUG_ANSI_RESET,
                       "BASE",
                       g_dwVif1Base);
        g_dwVif1Tops = g_dwVif1Base;
        if (g_dwVif1Stat & VIF_STAT_DBF) {
            g_dwVif1Tops = g_dwVif1Base + g_dwVif1Offset;
        }
        break;
    case VIF_ITOP:
        registers->itops = g_vifCode.immediate & VIF_IMMEDIATE_ADDRESS_MASK;
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%04x" DEBUG_ANSI_RESET,
                       "ITOPS",
                       registers->itops);
        break;
    case VIF_STMOD: {
        const char *modeNames[] = {"No addition processing",
                                   "Offset Mode (Row+dV->VUMem)",
                                   "Difference Mode (Row+dV->Row->VUMem)",
                                   DEBUG_ANSI_ALERT "*UNDEFINED*" DEBUG_ANSI_RESET};
        registers->mode = g_vifCode.immediate & VIF_MODE_MASK;
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "%s" DEBUG_ANSI_RESET,
                       "MODE",
                       modeNames[registers->mode]);
        break;
    }
    case VIF_MSKPATH3:
        DEBUG_addField(
            DEBUG_FIELD_INLINE,
            line,
            DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN "%s" DEBUG_ANSI_RESET,
            "Masked",
            ((g_vifCode.immediate >> VIF_IMMEDIATE_FLG_SHIFT) & 1) ? "DISABLE" : "ENABLE");
        break;
    case VIF_MARK:
        registers->mark = g_vifCode.immediate & 0xffff;
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%04x" DEBUG_ANSI_RESET,
                       "MARK",
                       registers->mark);
        break;
    case VIF_MSCAL:
    case VIF_MSCNT:
    case VIF_MSCALF:
        // Yes, the binary flips the double buffer of VIF1 for either unit.
        g_dwVif1Stat ^= VIF_STAT_DBF;
        if (g_vifCode.commandIndex != VIF_MSCNT) {
            DEBUG_addField(DEBUG_FIELD_INLINE,
                           line,
                           DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                            "0x%04x" DEBUG_ANSI_RESET,
                           "VU PC",
                           g_vifCode.immediate & 0xffff);
        }
        break;
    case VIF_MPG:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%02x" DEBUG_ANSI_RESET " instructions",
                       "Len",
                       g_vifCode.num);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%04x" DEBUG_ANSI_RESET,
                       "Addr",
                       g_vifCode.immediate & 0xffff);
        break;
    case VIF_DIRECT:
    case VIF_DIRECTHL:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%06x" DEBUG_ANSI_RESET " qwords",
                       "Len",
                       g_vifCode.immediate);
        break;
    case VIF_UNPACK: {
        const unsigned int address = g_vifCode.immediate & VIF_IMMEDIATE_ADDRESS_MASK;
        const unsigned int useTops = (g_vifCode.immediate >> VIF_IMMEDIATE_FLG_SHIFT) & 1;
        const unsigned int isUnsigned = (g_vifCode.immediate >> VIF_IMMEDIATE_USN_SHIFT) & 1;
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_GREEN "%s" DEBUG_ANSI_RESET,
                       g_apszVifUnpackNames[g_vifCode.unpackFormat]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%04x" DEBUG_ANSI_RESET,
                       "Addr",
                       address);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%02x" DEBUG_ANSI_RESET,
                       "Len",
                       g_vifCode.num);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "%d" DEBUG_ANSI_RESET,
                       "USN",
                       isUnsigned);
        if (g_nVifUnit == 1) {
            DEBUG_addField(DEBUG_FIELD_INLINE,
                           line,
                           DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                            "%d" DEBUG_ANSI_RESET,
                           "FLG",
                           useTops);
        }
        if (g_nVifVerbose) {
            if (useTops) {
                DEBUG_addField(DEBUG_FIELD_LINE,
                               line,
                               DEBUG_ANSI_WHITE "VUMem addr" DEBUG_ANSI_RESET
                                                ":V1_TOPS(" DEBUG_ANSI_GREEN
                                                "0x%04x" DEBUG_ANSI_RESET ")+ADDR(" DEBUG_ANSI_GREEN
                                                "0x%04x" DEBUG_ANSI_RESET ")=" DEBUG_ANSI_GREEN
                                                "0x%04x" DEBUG_ANSI_RESET,
                               g_dwVif1Tops,
                               address,
                               g_dwVif1Tops + address);
            } else {
                DEBUG_addField(DEBUG_FIELD_LINE,
                               line,
                               DEBUG_ANSI_WHITE "VUMem addr" DEBUG_ANSI_RESET ":" DEBUG_ANSI_GREEN
                                                "0x%04x" DEBUG_ANSI_RESET,
                               address);
            }
            DEBUG_addField(DEBUG_FIELD_INLINE,
                           line,
                           "Len:" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET " qwords",
                           g_vifCode.num);
        }
        break;
    }
    default:
        DEBUG_ABORT(732, "Logic error, this VIFcode should be caught.");
        break;
    }
    if (g_nVifUnit == 1) {
        g_dwVif1Tops = g_dwVif1Base;
        if (g_dwVif1Stat & VIF_STAT_DBF) {
            g_dwVif1Tops = g_dwVif1Base + g_dwVif1Offset;
        }
    }
    if (g_nVifVerbose) {
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "%d" DEBUG_ANSI_RESET " word%s",
                       "Code+data",
                       g_vifCode.length,
                       g_vifCode.length < 2 ? "" : "s");
    }
    return result;
}

// NTSC-U/C: 0x002a68e0, PAL: 0x002b0850
// Reads the next element of UNPACK data, sign-extending a signed 16-bit or 8-bit element.
static int grabElement(int sizeShift, int isUnsigned, unsigned int *element) {
    const int perWord = 1 << sizeShift;
    int result = 0;
    if (g_nVifWordIndex >= QWORD_WORDS) {
        result = DMA_step();
        if ((result & (DEBUG_ERROR | DEBUG_FATAL)) || g_pDmaQword == NULL) {
            result |= report(
                MESSAGE_PREMATURE_END, DEBUG_FATAL, "DMA prematurely ended when UNPACKing data.");
        }
        g_nVifWordIndex = g_nDmaQwordIsTag ? TAG_DATA_WORD : 0;
    }
    if (result & (DEBUG_ERROR | DEBUG_FATAL)) {
        return result;
    }
    const unsigned int word = g_pDmaQword[g_nVifWordIndex];
    *element = word;
    switch (sizeShift) {
    case UNPACK_ELEMENT_32:
        break;
    case UNPACK_ELEMENT_16: {
        const unsigned int value = (word >> (g_nVifElementIndex * 16)) & 0xffff;
        *element = value;
        if (!isUnsigned && (value & 0x8000)) {
            *element = value | 0xffff0000;
        }
        break;
    }
    case UNPACK_ELEMENT_8: {
        const unsigned int value = (word >> (g_nVifElementIndex * 8)) & 0xff;
        *element = value;
        if (!isUnsigned && (value & 0x80)) {
            *element = value | 0xffffff00;
        }
        break;
    }
    default:
        DEBUG_ABORT(813, "LOGIC ERROR - This case should never be reached (grabUnpackElement)\n");
        break;
    }
    if (++g_nVifElementIndex >= perWord) {
        g_nVifElementIndex = 0;
        ++g_nVifWordIndex;
        --g_vifCode.length;
    }
    return result;
}

// NTSC-U/C: 0x002a6ae0, PAL: 0x002b0a50
// Reads the next vector of UNPACK data into g_adwVifUnpackVector.
static int grabVector(void) {
    unsigned int elements[VECTOR_COMPONENTS];
    int result = 0;
    int sizeShift = g_vifCode.cmd & VIF_CMD_VL_MASK;
    int lastComponent = (g_vifCode.cmd >> VIF_CMD_VN_SHIFT) & VIF_CMD_VN_MASK;
    const int isUnsigned = (g_vifCode.immediate >> VIF_IMMEDIATE_USN_SHIFT) & 1;
    if (g_vifCode.unpackFormat == VIF_UNPACK_V4_5) {
        lastComponent = 0;
        sizeShift = UNPACK_ELEMENT_16;
    }
    const int count = lastComponent + 1;
    for (int i = 0; i < count; ++i) {
        result |= grabElement(sizeShift, isUnsigned, &elements[i]);
        if (result & (DEBUG_ERROR | DEBUG_FATAL)) {
            return result | report(MESSAGE_PREMATURE_END,
                                   DEBUG_FATAL,
                                   "Error obtaining next field for UNPACK");
        }
    }
    g_anVifUnpackValid[0] = 1;
    g_anVifUnpackValid[3] = 1;
    g_anVifUnpackValid[2] = 1;
    g_anVifUnpackValid[1] = 1;
    if (g_vifCode.unpackFormat == VIF_UNPACK_V4_5) {
        const unsigned int packed = elements[0];
        g_adwVifUnpackVector[0] = (packed & 0x1f) << 3;
        g_adwVifUnpackVector[1] = (packed >> 2) & 0xf8;
        g_adwVifUnpackVector[3] = (packed >> 8) & 0x80;
        g_adwVifUnpackVector[2] = (packed >> 7) & 0xf8;
    } else if (lastComponent == 0) {
        g_adwVifUnpackVector[0] = elements[0];
        g_adwVifUnpackVector[1] = elements[0];
        g_adwVifUnpackVector[3] = elements[0];
        g_adwVifUnpackVector[2] = elements[0];
    } else {
        for (int i = 0; i < VECTOR_COMPONENTS; ++i) {
            if (i < count) {
                g_adwVifUnpackVector[i] = elements[i];
            } else {
                g_anVifUnpackValid[i] = 0;
            }
        }
    }
    return result;
}

// NTSC-U/C: 0x002a6cb0, PAL: 0x002b0c20
// Reports the write of g_adwVifUnpackVector to a VU address under the mask bits of its cycle.
static int writeVector(unsigned int address, unsigned int maskBits, int useTops, int column) {
    const struct vif_registers *registers = &g_aVifRegisters[g_nVifUnit];
    unsigned int mode = registers->mode & VIF_MODE_MASK;
    int result = 0;
    if (useTops && g_nVifUnit != 1) {
        return report(MESSAGE_NOT_VIF, DEBUG_FATAL, "FLG flag is set on a VIF0 channel!");
    }
    const unsigned int limit =
        g_nDmaChannel == DMA_CHANNEL_VIF0 ? VU0_ADDRESS_MASK : VU1_ADDRESS_MASK;
    if (address > limit) {
        result = report(MESSAGE_ADDRESS_WRAP,
                        DEBUG_WARNING,
                        "Attempting to write to invalid VU address! (will wrap around)");
        address &= limit;
    }
    if ((unsigned int)column >= VECTOR_COMPONENTS) {
        column = LAST_COLUMN;
    }
    char *out = g_szDebugScratch;
    if (useTops) {
        out += sprintf(out,
                       "                        : TOPS(" DEBUG_ANSI_YELLOW "%04x" DEBUG_ANSI_RESET
                       ")+" DEBUG_ANSI_YELLOW "%04x" DEBUG_ANSI_RESET "=%04x : [",
                       g_dwVif1Tops,
                       address,
                       (g_dwVif1Tops + address) & limit);
    } else {
        out +=
            sprintf(out,
                    "                        :" DEBUG_ANSI_YELLOW " %04x" DEBUG_ANSI_RESET " : [",
                    address);
    }
    for (int i = 0; i < VECTOR_COMPONENTS; ++i) {
        out += sprintf(out, DEBUG_ANSI_WHITE "%c" DEBUG_ANSI_RESET "=", "XYZW"[i]);
        switch (maskBits & VIF_MASK_PROTECT) {
        case VIF_MASK_DATA:
            if (g_vifCode.unpackFormat == VIF_UNPACK_V4_5) {
                mode = VIF_MODE_NONE;
            }
            switch (mode) {
            case VIF_MODE_NONE:
                if (!g_anVifUnpackValid[i]) {
                    result |=
                        report(MESSAGE_INDETERMINATE,
                               DEBUG_WARNING,
                               "You are writing an indeterminant value to an unmasked field!");
                    out += sprintf(out, DEBUG_ANSI_ALERT "INDETERMINATE" DEBUG_ANSI_RESET);
                } else {
                    out += sprintf(
                        out, DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET, g_adwVifUnpackVector[i]);
                }
                break;
            case VIF_MODE_OFFSET:
                out += sprintf(out,
                               "[+R%d]=" DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET,
                               i,
                               g_adwVifUnpackVector[i] + g_aVifRegisters[g_nVifUnit].row[i]);
                break;
            case VIF_MODE_DIFFERENCE:
                g_aVifRegisters[g_nVifUnit].row[i] += g_adwVifUnpackVector[i];
                out += sprintf(out,
                               "[+=R%d]=" DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET,
                               i,
                               g_aVifRegisters[g_nVifUnit].row[i]);
                break;
            default:
                result |= report(MESSAGE_MODE_UNDEFINED,
                                 DEBUG_FATAL,
                                 "MODE register is set to 3, which is 'undefined'");
                out += sprintf(out, "INVALID - MODE INCORRECT");
                break;
            }
            break;
        case VIF_MASK_ROW:
            out += sprintf(out,
                           "R%d:" DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET,
                           i,
                           g_aVifRegisters[g_nVifUnit].row[i]);
            break;
        case VIF_MASK_COLUMN:
            out += sprintf(out,
                           "C%d:" DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET,
                           column,
                           g_aVifRegisters[g_nVifUnit].col[column]);
            break;
        case VIF_MASK_PROTECT:
            out += sprintf(out, DEBUG_ANSI_ON_RED "**MASKED** " DEBUG_ANSI_RESET);
            break;
        }
        if (i != VECTOR_COMPONENTS - 1) {
            out += sprintf(out, ".");
        }
        maskBits >>= VIF_MASK_FIELD_BITS;
    }
    strcat(out, "]");
    return result | DEBUG_print(g_szDebugScratch);
}

// NTSC-U/C: 0x002a70b0, PAL: 0x002b1020
// Simulates the VU memory writes of the current UNPACK through its write cycle.
static int simulateUnpack(char *line) {
    const struct vif_registers *registers = &g_aVifRegisters[g_nVifUnit];
    int result = 0;
    unsigned int cycle = 0;
    unsigned int clCount = 0;
    const int useTops = (g_vifCode.immediate >> VIF_IMMEDIATE_FLG_SHIFT) & 1;
    unsigned int address = g_vifCode.immediate & VIF_IMMEDIATE_ADDRESS_MASK;
    const unsigned int cl = registers->cycle & 0xff;
    const unsigned int wl = (registers->cycle >> 8) & 0xff;
    g_nVifElementIndex = 0;
    if (cl >= wl && wl == 0) {
        result = report(
            MESSAGE_WL_ZERO, DEBUG_FATAL, "CL >= WL (Skipping write), but WL == 0! WL must be > 0");
    }
    DEBUG_addField(DEBUG_FIELD_LINE,
                   line,
                   "Using: " DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                   "0x%02x" DEBUG_ANSI_RESET,
                   "WL",
                   wl);
    DEBUG_addField(DEBUG_FIELD_INLINE,
                   line,
                   DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                    "0x%02x" DEBUG_ANSI_RESET,
                   "CL",
                   cl);
    DEBUG_addField(DEBUG_FIELD_INLINE,
                   line,
                   DEBUG_ANSI_WHITE "Mask is" DEBUG_ANSI_RESET " " DEBUG_ANSI_GREEN
                                    "%s" DEBUG_ANSI_RESET,
                   (g_vifCode.cmd & VIF_CMD_UNPACK_MASKED) ? "Enabled" : "Disabled");
    if (g_vifCode.cmd & VIF_CMD_UNPACK_MASKED) {
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%08x" DEBUG_ANSI_RESET,
                       "MASK",
                       g_aVifRegisters[g_nVifUnit].mask);
    }
    DEBUG_addField(DEBUG_FIELD_INLINE,
                   line,
                   DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                    "%d" DEBUG_ANSI_RESET,
                   "MODE",
                   g_aVifRegisters[g_nVifUnit].mode & VIF_MODE_MASK);
    (void)DEBUG_print(line); // Yes, the binary discards the flags of this line.
    line[0] = '\0';
    if (!(result & (DEBUG_ERROR | DEBUG_FATAL)) && g_vifCode.num > 0) {
        const int filling = cl < wl;
        do {
            g_anVifUnpackValid[3] = 0;
            g_anVifUnpackValid[2] = 0;
            g_anVifUnpackValid[1] = 0;
            g_anVifUnpackValid[0] = 0;
            unsigned int maskBits = 0;
            if (g_vifCode.cmd & VIF_CMD_UNPACK_MASKED) {
                const unsigned int field = cycle < VECTOR_COMPONENTS ? cycle : LAST_COLUMN;
                maskBits = g_aVifRegisters[g_nVifUnit].mask >> (field * 8);
            }
            const int fillCycle = cl < wl;
            int grab;
            int write;
            if (!filling) {
                write = cycle < wl;
                grab = write;
            } else {
                write = 1;
                grab = clCount < cl;
            }
            if (write) {
                if (grab) {
                    result |= grabVector();
                    if (result & (DEBUG_ERROR | DEBUG_FATAL)) {
                        break;
                    }
                }
                result |= writeVector(address, maskBits, useTops, (int)cycle);
                --g_vifCode.num;
            }
            ++address;
            ++cycle;
            ++clCount;
            if (fillCycle ? cycle >= wl : clCount >= cl) {
                clCount = 0;
                cycle = 0;
            }
            if (result & (DEBUG_ERROR | DEBUG_FATAL)) {
                break;
            }
        } while (g_vifCode.num > 0);
    }
    if (g_nVifElementIndex != 0 && g_vifCode.length == 1) {
        g_vifCode.length = 0;
        ++g_nVifWordIndex;
    }
    return result;
}

// NTSC-U/C: 0x002a7410, PAL: 0x002b1380
// Moves the next quadword of the transfer into g_pDmaQword for decoding.
static int refill(void) {
    int result = DMA_step();
    if (g_pDmaQword == NULL && g_vifCode.length > 0) {
        result |= report(MESSAGE_PREMATURE_END,
                         DEBUG_FATAL,
                         "VIF tried to obtain more DMA data for decoding, but DMA has ended!");
    }
    g_nVifWordIndex = g_nDmaQwordIsTag ? TAG_DATA_WORD : 0;
    return result;
}

int VIF_disassemble(void) {
    char line[VIF_LINE_SIZE];
    int result = 0;
    g_vifCode.length = 0;
    g_nVifWordIndex = QWORD_WORDS;
    for (int i = VIF_COMMAND_COUNT - 1; i >= 0; --i) {
        g_anVifCommandCounts[i] = 0;
    }
    if ((unsigned int)g_nDmaChannel > DMA_CHANNEL_VIF1) {
        return report(MESSAGE_NOT_VIF,
                      DEBUG_FATAL,
                      "Cannot examine VIF packets on a non VIF DMA channel (this is not "
                      "SCE_DMA_VIF0 or SCE_DMA_VIF1!!)");
    }
    g_nVifUnit = g_nDmaChannel != DMA_CHANNEL_VIF0;
    for (;;) {
        if (g_nVifWordIndex >= QWORD_WORDS) {
            result |= refill();
            if ((result & (DEBUG_ERROR | DEBUG_FATAL)) || g_pDmaQword == NULL) {
                break;
            }
        }
        line[0] = '\0';
        result |= decodeCode(&g_pDmaQword[g_nVifWordIndex], line);
        if (result & (DEBUG_ERROR | DEBUG_FATAL)) {
            (void)DEBUG_print(line); // Yes, the binary discards the flags of this line.
            break;
        }
        ++g_nVifWordIndex;
        --g_vifCode.length;
        switch (g_vifCode.commandIndex) {
        case VIF_STMASK:
            if (g_nVifWordIndex == QWORD_WORDS) {
                result |= refill();
                if ((result & (DEBUG_ERROR | DEBUG_FATAL)) || g_pDmaQword == NULL) {
                    result |= report(MESSAGE_MASK_DATA,
                                     DEBUG_FATAL,
                                     "Error obtaining more data for STMASK VIFcode. Premature DMA "
                                     "termination?");
                }
            }
            if (!(result & (DEBUG_ERROR | DEBUG_FATAL))) {
                g_aVifRegisters[g_nVifUnit].mask = g_pDmaQword[g_nVifWordIndex];
                DEBUG_addField(DEBUG_FIELD_INLINE,
                               line,
                               DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                                "0x%08x" DEBUG_ANSI_RESET,
                               "MASK",
                               g_pDmaQword[g_nVifWordIndex]);
                ++g_nVifWordIndex;
                --g_vifCode.length;
            }
            break;
        case VIF_STROW:
        case VIF_STCOL:
            for (int i = 0; i < VECTOR_COMPONENTS; ++i) {
                if (g_nVifWordIndex == QWORD_WORDS) {
                    result |= refill();
                    if ((result & (DEBUG_ERROR | DEBUG_FATAL)) || g_pDmaQword == NULL) {
                        result |= report(MESSAGE_ROW_COLUMN_DATA,
                                         DEBUG_FATAL,
                                         g_vifCode.commandIndex == VIF_STROW ?
                                             "Error obtaining more data for STROW VIFcode. "
                                             "Premature DMA termination?" :
                                             "Error obtaining more data for STCOL VIFcode. "
                                             "Premature DMA termination?");
                    }
                }
                // Yes, the binary continues with the next value after a failed read.
                if (result & (DEBUG_ERROR | DEBUG_FATAL)) {
                    continue;
                }
                if (g_vifCode.commandIndex == VIF_STROW) {
                    g_aVifRegisters[g_nVifUnit].row[i] = g_pDmaQword[g_nVifWordIndex];
                } else {
                    g_aVifRegisters[g_nVifUnit].col[i] = g_pDmaQword[g_nVifWordIndex];
                }
                sprintf(g_szDebugScratch, "%f", (double)wordToFloat(g_pDmaQword[g_nVifWordIndex]));
                DEBUG_addField(DEBUG_FIELD_LINE,
                               line,
                               DEBUG_ANSI_WHITE "%c%d" DEBUG_ANSI_RESET " :" DEBUG_ANSI_GREEN
                                                "0x%08x" DEBUG_ANSI_RESET " (" DEBUG_ANSI_GREEN
                                                "%s" DEBUG_ANSI_RESET ")",
                               g_vifCode.commandIndex == VIF_STROW ? 'R' : 'C',
                               i,
                               g_pDmaQword[g_nVifWordIndex],
                               g_szDebugScratch);
                ++g_nVifWordIndex;
                --g_vifCode.length;
            }
            break;
        case VIF_MPG:
            if (g_nVifWordIndex & 1) {
                result |= report(MESSAGE_MPG_MISALIGNED,
                                 DEBUG_ERROR,
                                 "MPG data is not on a 64 bit aligned address.");
                break;
            }
            DEBUG_addField(DEBUG_FIELD_LINE,
                           line,
                           "[Skipping " DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET " words]",
                           g_vifCode.length);
            while (g_vifCode.length > 0) {
                if (g_nVifWordIndex == QWORD_WORDS) {
                    result |= refill();
                    if ((result & (DEBUG_ERROR | DEBUG_FATAL)) || g_pDmaQword == NULL) {
                        result |= report(MESSAGE_MPG_DATA,
                                         DEBUG_FATAL,
                                         "Error obtaining more data for MPG VIFcode. Premature DMA "
                                         "termination?");
                        break;
                    }
                }
                g_nVifWordIndex += 2;
                g_vifCode.length -= 2;
            }
            break;
        case VIF_UNPACK:
            if (g_nVifDisassembleUnpack) {
                result |= simulateUnpack(line);
                if (g_vifCode.length > 0) {
                    result |= report(MESSAGE_UNPACK_INCOMPLETE,
                                     DEBUG_FATAL,
                                     "UNPACK Error - unpack did not complete. Premature DMA "
                                     "termination?");
                }
                break;
            }
            (void)DEBUG_print(line); // Yes, the binary discards the flags of this line.
            line[0] = '\0';
            while (g_vifCode.length > 0) {
                if (g_nVifWordIndex == QWORD_WORDS) {
                    result |= refill();
                    if ((result & (DEBUG_ERROR | DEBUG_FATAL)) || g_pDmaQword == NULL) {
                        break;
                    }
                }
                ++g_nVifWordIndex;
                --g_vifCode.length;
            }
            // Yes, the binary spins forever when the transfer ends before the data does.
            while (g_vifCode.length > 0) {
            }
            break;
        case VIF_DIRECT:
        case VIF_DIRECTHL:
            (void)DEBUG_print(line); // Yes, the binary discards the flags of this line.
            line[0] = '\0';
            if (g_nVifWordIndex != QWORD_WORDS) {
                result |= report(MESSAGE_GIF_MISALIGNED,
                                 DEBUG_FATAL,
                                 "GIF tag is not aligned to a quadword boundary!");
            }
            if (g_vifCode.length & 3) {
                DEBUG_ABORT(1259,
                            "Logic error! length for DIRECT/HL should be a multiple of 4!!\n");
            }
            while (g_vifCode.length > 0 && !(result & (DEBUG_ERROR | DEBUG_FATAL))) {
                result |= refill();
                if ((result & (DEBUG_ERROR | DEBUG_FATAL)) || g_pDmaQword == NULL) {
                    break;
                }
                if (g_nVifDisassembleDirect) {
                    g_pGifData = g_pDmaQword;
                    result |= GIF_processQword();
                }
                g_nVifWordIndex = QWORD_WORDS;
                g_vifCode.length -= QWORD_WORDS;
            }
            if (g_nVifDisassembleDirect && !(result & (DEBUG_ERROR | DEBUG_FATAL)) &&
                !GIF_isTagDone()) {
                result |= report(MESSAGE_GIF_INCOMPLETE,
                                 DEBUG_WARNING,
                                 "DIRECT/DIRECTHL ended, but GIF is expecting more data.");
            }
            break;
        }
        const int failed = result & (DEBUG_ERROR | DEBUG_FATAL);
        if ((g_nVifShowCodes || failed) && strlen(line) != 0) {
            (void)DEBUG_print(line); // Yes, the binary discards the flags of this line.
        }
        if (result == 0 && g_vifCode.length != 0) {
            DEBUG_ABORT(1306,
                        "Logic error, length should be zero when exiting this switch statement.\n");
        }
        if (failed) {
            break;
        }
    }
    if (g_nVifShowRegisters) {
        result |= dumpRegisters();
    }
    if (g_nVifShowStatistics) {
        result |= dumpStatistics();
    }
    return result;
}
