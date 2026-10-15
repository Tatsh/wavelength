#include "dmadebug/dma.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dmadebug/debug.h"

// DMA tag identifiers, the ID field of a tag.
enum {
    TAG_REFE = 0,
    TAG_CNT = 1,
    TAG_NEXT = 2,
    TAG_REF = 3,
    TAG_REFS = 4,
    TAG_CALL = 5,
    TAG_RET = 6,
    TAG_END = 7,
    TAG_COUNT = 8,
};

// Fields of the first word of a DMA tag.
enum {
    TAG_QWC_MASK = 0xffff,
    TAG_RESERVED_SHIFT = 16,
    TAG_RESERVED_MASK = 0x3ff,
    TAG_PCE_SHIFT = 26,
    TAG_PCE_MASK = 3,
    TAG_PCE_RESERVED = 1,
    TAG_ID_SHIFT = 28,
    TAG_ID_MASK = 7,
    TAG_IRQ_SHIFT = 31,
};

// Fields of CHCR.
enum {
    CHCR_MOD_SHIFT = 2,
    CHCR_MOD_MASK = 3,
    CHCR_MOD_NORMAL = 0,
    CHCR_MOD_CHAIN = 1,
    CHCR_ASP_SHIFT = 4,
    CHCR_ASP_MASK = 3,
    CHCR_ASP_FIELD = 0x30,
    CHCR_TTE_SHIFT = 6,
    CHCR_STR = 0x100,
};

// The address space bits of an EE address, and the bit of a tag address that selects the
// scratchpad.
static const unsigned int ADDRESS_SPACE_MASK = 0xf0000000;
static const unsigned int TAG_ADDRESS_SPR = 0x80000000;

// Address spaces of the EE.
enum {
    ADDRESS_SPACE_SHIFT = 28,
    ADDRESS_OFFSET_MASK = 0x0fffffff,
    ADDRESS_CACHED = 0,
    ADDRESS_UNCACHED = 0x20000000,
    ADDRESS_UNCACHED_ACCELERATED = 3,
    ADDRESS_SCRATCHPAD = 0x70000000,
    TAG_ADDRESS_QWORD_MASK = 0x0ffffff0,
    SADR_MASK = 0x3ff0,
    KERNEL_END = 0xfffff,
    SCRATCHPAD_SIZE = 0x4000,
    VU0_MICRO_MEMORY = 0x11000000,
    VU0_MICRO_MEMORY_SIZE = 0x1000,
    VU0_DATA_MEMORY_LAST = 0x11003fff,
    VU0_DATA_MEMORY_END = 0x11004fff,
    VU0_DATA_MEMORY_MIRRORS_END = 0x11007fff,
    VU1_MICRO_MEMORY = 0x11008000,
    VU1_DATA_MEMORY = 0x1100c000,
    VU1_MEMORY_SIZE = 0x4000,
};

// Bytes in a quadword.
enum { QWORD_SIZE = 16 };

// Message numbers.
enum {
    MESSAGE_PCE_RESERVED = 20,
    MESSAGE_UNALIGNED = 30,
    MESSAGE_BAD_ADDRESS = 40,
    MESSAGE_CALL_STACK_FULL = 60,
    MESSAGE_WRONG_CHANNEL = 70,
    MESSAGE_RESERVED_BITS = 1010,
    MESSAGE_CALL_STACK_NOT_EMPTY = 1030,
    MESSAGE_NULL_TAG = 1100,
};

// Most transfer ranges one tag describes, the tag and its data.
enum { MAX_RANGES = 4 };

// Capacity of the report line of a tag.
enum { TAG_LINE_SIZE = 1024 };

// NTSC-U/C: 0x003b24e8, PAL: 0x004313a8
// Names of the tag identifiers.
static const char *g_apszDmaTagNames[] = {
    "REFE", "CNT", "NEXT", "REF", "REFS", "CALL", "RET", "END"};

// NTSC-U/C: 0x003b2508, PAL: 0x004313c8
// Names of the channels.
static const char *g_apszDmaChannelNames[] = {
    "VIF0", "VIF1", "GIF", "fromIPU", "toIPU", "SIF0", "SIF1", "SIF2", "fromSPR", "toSPR"};

// NTSC-U/C: 0x003b2530, PAL: 0x004313f0
// Names of the PCE values of a tag.
static const char *g_apszDmaPriorityNames[] = {"No priority", "*RESERVED*", "Disabled", "Enabled"};

// NTSC-U/C: 0x003b2540, PAL: 0x00431400
// The last valid address of main memory.
static unsigned int g_dwMainRamTop = 0x7ffffff;

// NTSC-U/C: 0x003b2544, PAL: 0x00431404
// Non-zero when the mirrors of VU0 data memory are valid addresses.
static int g_nDmaUseMirrors = 0;

// NTSC-U/C: 0x003b2548
// Non-zero when each tag is reported.
static int g_nDmaDebugPrintEnabled = 1;

// NTSC-U/C: 0x003b254c, PAL: 0x0043140c
// Non-zero when the end of a chain reports the tag statistics.
static int g_nDmaStatsEnabled = 1;

// NTSC-U/C: 0x005190c8
// The message report() formats.
static char g_szDebugMessageScratch[DEBUG_SCRATCH_SIZE];

// NTSC-U/C: 0x005194c8
// The report line of the current tag.
static char g_szDmaTagLine[TAG_LINE_SIZE];

int g_nDmaChannel;

const unsigned int *g_pDmaQword;

int g_nDmaQwordIsTag;

// NTSC-U/C: 0x005198d4
// The registers of the channel being checked.
static struct dma_channel *g_pDmaChannel;

// NTSC-U/C: 0x005198d8
// Address of the tag a chain reads next.
static unsigned int g_dwDmaNextTag;

// NTSC-U/C: 0x005198e0
// Tags read, by identifier.
static int g_anDmaTagCounts[TAG_COUNT];

// NTSC-U/C: 0x00519900
// Quadwords transferred, by tag identifier.
static int g_anDmaTagQwords[TAG_COUNT];

// NTSC-U/C: 0x00519948
// Non-zero once the current tag was read and its data not yet finished.
static int g_nDmaTagRead;

// Returns the words at an EE address.
static inline const unsigned int *wordsAt(unsigned int address) {
    return (const unsigned int *)address;
}

// NTSC-U/C: 0x002a15a0, PAL: 0x002ab510
// Reports the tags read and the quadwords transferred by tag identifier, and their totals.
static int dumpStatistics(void) {
    int totalTags = 0;
    int totalQwords = 0;
    int result = DEBUG_print("==== DMA STATISTICS ====");
    result |= DEBUG_print("  Tags      # Tags    QWords Txfr'd");
    result |= DEBUG_print("  ----      ------    -------------");
    for (int id = 0; id < TAG_COUNT; ++id) {
        if (g_anDmaTagCounts[id] != 0) {
            sprintf(g_szDebugScratch,
                    "  %-12s%4d%6s%6d",
                    g_apszDmaTagNames[id],
                    g_anDmaTagCounts[id],
                    "",
                    g_anDmaTagQwords[id]);
            totalTags += g_anDmaTagCounts[id];
            totalQwords += g_anDmaTagQwords[id];
            result |= DEBUG_print(g_szDebugScratch);
        }
    }
    result |= DEBUG_print("            ======    =============");
    sprintf(g_szDebugScratch, "  %12s%4d%6s%6d", "", totalTags, "", totalQwords);
    return result | DEBUG_print(g_szDebugScratch);
}

// NTSC-U/C: 0x002a1920, PAL: 0x002ab890
// Returns the address the EE reads for an address, mapping a scratchpad tag address to the
// scratchpad and cached or accelerated memory to uncached memory.
static unsigned int resolveAddress(unsigned int address) {
    if (address & TAG_ADDRESS_SPR) {
        address = (address & TAG_ADDRESS_QWORD_MASK) | ADDRESS_SCRATCHPAD;
    }
    const unsigned int space = address >> ADDRESS_SPACE_SHIFT;
    if (space == ADDRESS_CACHED || space == ADDRESS_UNCACHED_ACCELERATED) {
        address = (address & ADDRESS_OFFSET_MASK) | ADDRESS_UNCACHED;
    }
    return address;
}

// NTSC-U/C: 0x002a16f0, PAL: 0x002ab660
// Reports a message with the channel name and, given a tag address, the tag.
static int report(int number, int severity, unsigned int tagAddress, const char *text) {
    sprintf(g_szDebugMessageScratch, "%s : ", g_apszDmaChannelNames[g_nDmaChannel]);
    if (tagAddress != 0) {
        const unsigned int *tag = wordsAt(resolveAddress(tagAddress));
        sprintf(&g_szDebugMessageScratch[strlen(g_szDebugMessageScratch)],
                DEBUG_ANSI_YELLOW "%08x" DEBUG_ANSI_RESET ":[" DEBUG_ANSI_MAGENTA
                                  "%08x.%08x" DEBUG_ANSI_RESET "] ",
                (unsigned int)tag,
                tag[1],
                tag[0]);
    }
    switch (severity) {
    case DEBUG_ERROR:
    case DEBUG_FATAL:
        strcat(g_szDebugMessageScratch, DEBUG_ANSI_ALERT);
        break;
    case DEBUG_WARNING:
        strcat(g_szDebugMessageScratch, DEBUG_ANSI_CAUTION);
        break;
    }
    strcat(g_szDebugMessageScratch, text);
    strcat(g_szDebugMessageScratch, DEBUG_ANSI_RESET);
    return DEBUG_message(number, severity, g_szDebugMessageScratch);
}

// NTSC-U/C: 0x002a1840, PAL: 0x002ab7b0
// Returns whether an address lies in main memory above the kernel, the scratchpad, or a VU memory.
static int isValidAddress(unsigned int address) {
    if ((address >> ADDRESS_SPACE_SHIFT) - 2 < 2) {
        address &= ADDRESS_OFFSET_MASK;
    }
    const unsigned int vu0DataEnd =
        g_nDmaUseMirrors ? VU0_DATA_MEMORY_MIRRORS_END : VU0_DATA_MEMORY_END;
    if (address > KERNEL_END && address <= g_dwMainRamTop) {
        return 1;
    }
    if (address - ADDRESS_SCRATCHPAD < SCRATCHPAD_SIZE) {
        return 1;
    }
    if (address - VU0_MICRO_MEMORY < VU0_MICRO_MEMORY_SIZE) {
        return 1;
    }
    if (address > VU0_DATA_MEMORY_LAST && address <= vu0DataEnd) {
        return 1;
    }
    if (address - VU1_DATA_MEMORY < VU1_MEMORY_SIZE) {
        return 1;
    }
    if (address - VU1_MICRO_MEMORY < VU1_MEMORY_SIZE) {
        return 1;
    }
    return 0;
}

// NTSC-U/C: 0x002a1970, PAL: 0x002ab8e0
// Returns the tag form of an address, with a scratchpad address marked by bit 31.
static unsigned int toTagAddress(unsigned int address) {
    if ((address & ADDRESS_SPACE_MASK) == ADDRESS_SCRATCHPAD) {
        address = (address & ADDRESS_OFFSET_MASK) | TAG_ADDRESS_SPR;
    }
    return address;
}

// NTSC-U/C: 0x002a19a0, PAL: 0x002ab910
// Reports an address that is not quadword aligned or not in a valid range.
static int validateAddress(unsigned int address, const char *name) {
    int result = 0;
    if (address & (QWORD_SIZE - 1)) {
        sprintf(g_szDebugScratch, "%s at %p is not quadword aligned.", name, (void *)address);
        result = report(MESSAGE_UNALIGNED, DEBUG_FATAL, 0, g_szDebugScratch);
    }
    if (!isValidAddress(resolveAddress(address))) {
        sprintf(g_szDebugScratch, "%s (0x%08x) is not in a valid address range", name, address);
        result |= report(MESSAGE_BAD_ADDRESS, DEBUG_FATAL, 0, g_szDebugScratch);
    }
    return result;
}

// NTSC-U/C: 0x002a1a78, PAL: 0x002ab9e8
// Sets the address of the next quadword of data.
static void setDataAddress(unsigned int address) {
    if ((unsigned int)(g_nDmaChannel - DMA_CHANNEL_FROM_SPR) < 2) {
        g_pDmaChannel->sadr = address & SADR_MASK;
    } else {
        g_pDmaChannel->madr = address;
    }
}

// NTSC-U/C: 0x002a1ab0, PAL: 0x002aba20
// Returns the address of the next quadword of data.
static unsigned int getDataAddress(void) {
    if ((unsigned int)(g_nDmaChannel - DMA_CHANNEL_FROM_SPR) < 2) {
        return g_pDmaChannel->sadr | TAG_ADDRESS_SPR;
    }
    return g_pDmaChannel->madr;
}

// NTSC-U/C: 0x002a1af0, PAL: 0x002aba60
// Moves the data address to the next quadword.
static void advanceDataAddress(void) {
    setDataAddress(getDataAddress() + QWORD_SIZE);
}

// NTSC-U/C: 0x002a1b18, PAL: 0x002aba88
// Records the quadword a step moved.
static void setQword(unsigned int address, int isTag) {
    g_pDmaQword = wordsAt(address);
    g_nDmaQwordIsTag = isTag;
}

// NTSC-U/C: 0x002a1b30, PAL: 0x002abaa0
// Ends the transfer, reporting the tag statistics at the end of a chain.
static void stop(void) {
    g_pDmaChannel->chcr &= ~CHCR_STR;
    g_pDmaQword = NULL;
    g_nDmaQwordIsTag = 0;
    if (g_nDmaStatsEnabled &&
        ((g_pDmaChannel->chcr >> CHCR_MOD_SHIFT) & CHCR_MOD_MASK) == CHCR_MOD_CHAIN) {
        (void)dumpStatistics(); // Yes, the binary discards the flags of the statistics report.
    }
}

// NTSC-U/C: 0x002a1b98, PAL: 0x002abb08
// Moves one quadword of data.
static int transferData(void) {
    const unsigned int address = getDataAddress();
    const int result = validateAddress(address, "DMA data");
    setQword(resolveAddress(address), 0);
    advanceDataAddress();
    --g_pDmaChannel->qwc;
    return result;
}

// NTSC-U/C: 0x002a1c10, PAL: 0x002abb80
// Reads, validates, and reports the current tag of a chain, and follows it.
static int readTag(void) {
    int result = 0;
    const unsigned int tagAddress = resolveAddress(g_pDmaChannel->tadr);
    const unsigned int *tag = wordsAt(tagAddress);
    const unsigned int header = tag[0];
    unsigned int address = tag[1];
    const unsigned int qwc = header & TAG_QWC_MASK;
    const unsigned int irq = header >> TAG_IRQ_SHIFT;
    const unsigned int id = (header >> TAG_ID_SHIFT) & TAG_ID_MASK;
    const unsigned int pce = (header >> TAG_PCE_SHIFT) & TAG_PCE_MASK;
    const unsigned int chcr = g_pDmaChannel->chcr;
    const unsigned int asp = (chcr >> CHCR_ASP_SHIFT) & CHCR_ASP_MASK;
    const unsigned int tte = (chcr >> CHCR_TTE_SHIFT) & 1;
    g_szDmaTagLine[0] = '\0';
    DEBUG_addField(DEBUG_FIELD_COLUMN,
                   g_szDmaTagLine,
                   DEBUG_ANSI_ON_GREEN "DMA %s%s %-4s" DEBUG_ANSI_RESET " [" DEBUG_ANSI_CYAN
                                       "%08x" DEBUG_ANSI_RESET "]",
                   asp != 0 ? "->" : "  ",
                   asp == 2 ? "->" : "  ",
                   g_apszDmaTagNames[id],
                   tagAddress);
    DEBUG_addField(DEBUG_FIELD_INLINE,
                   g_szDmaTagLine,
                   "[" DEBUG_ANSI_MAGENTA "%08x.%08x" DEBUG_ANSI_RESET "]",
                   address,
                   header);
    DEBUG_addField(DEBUG_FIELD_INLINE,
                   g_szDmaTagLine,
                   "CHAN=" DEBUG_ANSI_GREEN "%s" DEBUG_ANSI_RESET,
                   g_apszDmaChannelNames[g_nDmaChannel]);
    DEBUG_addField(
        DEBUG_FIELD_INLINE, g_szDmaTagLine, "TTE=" DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET, tte);
    DEBUG_addField(
        DEBUG_FIELD_INLINE, g_szDmaTagLine, "IRQ=" DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET, irq);
    DEBUG_addField(DEBUG_FIELD_INLINE,
                   g_szDmaTagLine,
                   "PCE=" DEBUG_ANSI_GREEN "%11s" DEBUG_ANSI_RESET,
                   g_apszDmaPriorityNames[pce]);
    DEBUG_addField(DEBUG_FIELD_INLINE,
                   g_szDmaTagLine,
                   "QWC:" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET DEBUG_ANSI_RESET,
                   qwc);
    if (pce == TAG_PCE_RESERVED) {
        result = report(
            MESSAGE_PCE_RESERVED, DEBUG_ERROR, tagAddress, "PCE field is 1, which is reserved.");
    }
    if ((header >> TAG_RESERVED_SHIFT) & TAG_RESERVED_MASK) {
        result |=
            report(MESSAGE_RESERVED_BITS,
                   DEBUG_WARNING,
                   tagAddress,
                   "Bits 16-25 in this DMA tag are not 0. You may be trying to transfer >= 1Mb?");
    }
    if (id == TAG_REFS && g_nDmaChannel != DMA_CHANNEL_VIF1 && g_nDmaChannel != DMA_CHANNEL_SIF1 &&
        g_nDmaChannel != DMA_CHANNEL_GIF) {
        result |=
            report(MESSAGE_WRONG_CHANNEL,
                   DEBUG_ERROR,
                   tagAddress,
                   "DMA tag of type REFS encountered, but this is only permitted on VIF1, GIF "
                   "& SIF1 channels!");
    }
    if ((id == TAG_CALL || id == TAG_RET) && (unsigned int)g_nDmaChannel > DMA_CHANNEL_GIF) {
        result |=
            report(MESSAGE_WRONG_CHANNEL,
                   DEBUG_ERROR,
                   tagAddress,
                   "DMA tag of type CALL/RET encountered, but this is only permitted on VIF0, "
                   "VIF1 & GIF channels!");
    }
    if (id != TAG_CNT && id != TAG_RET && id != TAG_END) {
        if (qwc == 0 && id == TAG_REFE && address == 0) {
            result |= report(MESSAGE_NULL_TAG,
                             DEBUG_WARNING,
                             tagAddress,
                             "REFE tag's QWC=0 and trying to transfer from ADDR=0 - accidentally "
                             "nulled DMA tag?");
        } else {
            result |= validateAddress(address, "ADDR field");
            address = resolveAddress(address);
            if (result & (DEBUG_ERROR | DEBUG_FATAL)) {
                result |= report(
                    MESSAGE_BAD_ADDRESS, DEBUG_ERROR, tagAddress, "DMA tag ADDR field is invalid");
            }
        }
    }
    unsigned int starts[MAX_RANGES];
    unsigned int ends[MAX_RANGES];
    int ranges = 0;
    if (id == TAG_REF || id == TAG_REFE || id == TAG_REFS) {
        if (tte) {
            starts[ranges] = tagAddress;
            ends[ranges] = tagAddress + QWORD_SIZE;
            ranges = 1;
        }
        if (qwc != 0) {
            const unsigned int data = resolveAddress(address);
            starts[ranges] = data;
            ends[ranges] = data + qwc * QWORD_SIZE;
            ++ranges;
        }
    } else {
        starts[0] = tte ? tagAddress : tagAddress + QWORD_SIZE;
        if (qwc != 0 || tte) {
            ends[0] = tagAddress + qwc * QWORD_SIZE + QWORD_SIZE;
            ranges = 1;
        }
    }
    for (int i = 0; i < ranges; ++i) {
        int check = validateAddress(toTagAddress(starts[i]), "Transfer start address");
        check |= validateAddress(toTagAddress(ends[i] - QWORD_SIZE), "Transfer end address");
        if (check & (DEBUG_ERROR | DEBUG_FATAL)) {
            sprintf(g_szDebugScratch,
                    "Data to be transferred falls outside main mem/scratch pad RAM. [%08x -> %08x]",
                    starts[i],
                    ends[i]);
            result |= report(MESSAGE_BAD_ADDRESS, DEBUG_ERROR, 0, g_szDebugScratch);
            break;
        }
    }
    if (ranges == 0) {
        DEBUG_addField(DEBUG_FIELD_LINE,
                       g_szDmaTagLine,
                       "Transfers from: [" DEBUG_ANSI_YELLOW
                       "No data is transferred" DEBUG_ANSI_RESET "]");
    } else {
        for (int i = 0; i < ranges; ++i) {
            DEBUG_addField(DEBUG_FIELD_LINE,
                           g_szDmaTagLine,
                           "Transfers: [" DEBUG_ANSI_YELLOW "%08x" DEBUG_ANSI_RESET
                           "->" DEBUG_ANSI_YELLOW "%08x" DEBUG_ANSI_RESET
                           ") [len(qwords):" DEBUG_ANSI_GREEN "0x%x" DEBUG_ANSI_RESET "]%s",
                           starts[i],
                           ends[i],
                           (ends[i] - starts[i]) >> 4,
                           (i == 0 && tte) ? " (Includes DMA tag)" : "");
        }
    }
    ++g_anDmaTagCounts[id];
    for (int i = 0; i < ranges; ++i) {
        g_anDmaTagQwords[id] += (ends[i] - starts[i]) >> 4;
    }
    g_pDmaChannel->qwc = qwc;
    const unsigned int afterData = tagAddress + qwc * QWORD_SIZE + QWORD_SIZE;
    switch (id) {
    case TAG_CNT:
        g_dwDmaNextTag = afterData;
        g_pDmaChannel->madr = toTagAddress(tagAddress + QWORD_SIZE);
        break;
    case TAG_REFE:
    case TAG_REF:
    case TAG_REFS:
        g_dwDmaNextTag = id != TAG_REFE ? toTagAddress(tagAddress + QWORD_SIZE) : 0;
        g_pDmaChannel->madr = toTagAddress(address);
        break;
    case TAG_END:
        g_dwDmaNextTag = 0;
        g_pDmaChannel->madr = toTagAddress(tagAddress + QWORD_SIZE);
        break;
    case TAG_RET: {
        const unsigned int depth = (g_pDmaChannel->chcr >> CHCR_ASP_SHIFT) & CHCR_ASP_MASK;
        if (depth == 0) {
            g_dwDmaNextTag = 0;
            // Yes, the binary reports a stack that is not empty when the stack is empty.
            result |= report(MESSAGE_CALL_STACK_NOT_EMPTY,
                             DEBUG_WARNING,
                             tagAddress,
                             "RET tag encountered, but the call stack is not empty!");
        } else if (depth == 1) {
            g_dwDmaNextTag = g_pDmaChannel->asr0;
        } else if (depth == 2) {
            g_dwDmaNextTag = g_pDmaChannel->asr1;
        } else {
            DEBUG_ABORT(612, "Logic error - ASP of CHCR is > 2\n");
        }
        g_pDmaChannel->madr = toTagAddress(tagAddress + QWORD_SIZE);
        break;
    }
    case TAG_NEXT:
        g_dwDmaNextTag = toTagAddress(address);
        g_pDmaChannel->madr = toTagAddress(tagAddress + QWORD_SIZE);
        break;
    case TAG_CALL: {
        g_dwDmaNextTag = toTagAddress(address);
        const unsigned int control = g_pDmaChannel->chcr;
        const unsigned int depth = (control >> CHCR_ASP_SHIFT) & CHCR_ASP_MASK;
        if (depth == 0) {
            g_pDmaChannel->chcr = (control & ~CHCR_ASP_FIELD) | (1 << CHCR_ASP_SHIFT);
            g_pDmaChannel->asr0 = afterData;
        } else if (depth == 1) {
            g_pDmaChannel->asr1 = afterData;
            g_pDmaChannel->chcr = (control & ~CHCR_ASP_FIELD) | (2 << CHCR_ASP_SHIFT);
        } else if (depth == 2) {
            g_dwDmaNextTag = 0;
            result |= report(MESSAGE_CALL_STACK_FULL,
                             DEBUG_FATAL,
                             tagAddress,
                             "DMA tag of type CALL encountered, but call stack is full!");
        }
        g_pDmaChannel->madr = toTagAddress(tagAddress + QWORD_SIZE);
        break;
    }
    }
    if (g_nDmaDebugPrintEnabled) {
        result |= DEBUG_print(g_szDmaTagLine);
    }
    return result;
}

// NTSC-U/C: 0x002a2410, PAL: 0x002ac380
// Steps a chain transfer, reading tags until a quadword moves or the chain ends.
static int stepChain(void) {
    int result = 0;
    int moved = 0;
    for (;;) {
        if (g_pDmaChannel->qwc != 0) {
            result |= transferData();
            moved = 1;
        } else if (!g_nDmaTagRead) {
            const unsigned int next = g_dwDmaNextTag;
            g_pDmaChannel->tadr = next;
            result |= validateAddress(next, "DMA Tag");
            if (result & (DEBUG_ERROR | DEBUG_FATAL)) {
                return result;
            }
            const unsigned int tagAddress = resolveAddress(next);
            result |= readTag();
            if (((g_pDmaChannel->chcr >> CHCR_TTE_SHIFT) & 1) == 1) {
                setQword(tagAddress, 1);
                moved = 1;
            }
            g_nDmaTagRead = 1;
        } else {
            const unsigned int id =
                (wordsAt(resolveAddress(g_pDmaChannel->tadr))[0] >> TAG_ID_SHIFT) & TAG_ID_MASK;
            const unsigned int depth = (g_pDmaChannel->chcr >> CHCR_ASP_SHIFT) & CHCR_ASP_MASK;
            if (id == TAG_REFE || id == TAG_END || (id == TAG_RET && depth == 0)) {
                if (id != TAG_RET && depth != 0) {
                    result |= report(MESSAGE_CALL_STACK_NOT_EMPTY,
                                     DEBUG_WARNING,
                                     g_pDmaChannel->tadr,
                                     "END/REFE tag encountered, but the call stack is not empty!");
                }
                stop();
                moved = 1;
            } else if (id == TAG_RET) {
                const unsigned int control = g_pDmaChannel->chcr;
                const int remaining = (int)((control >> CHCR_ASP_SHIFT) & CHCR_ASP_MASK);
                if (remaining > 0) {
                    g_pDmaChannel->chcr = (control & ~CHCR_ASP_FIELD) |
                                          (((remaining - 1) & CHCR_ASP_MASK) << CHCR_ASP_SHIFT);
                }
            }
            g_nDmaTagRead = 0;
        }
        if (result & (DEBUG_ERROR | DEBUG_FATAL)) {
            return result;
        }
        if (moved) {
            return result;
        }
    }
}

// NTSC-U/C: 0x002a25e8, PAL: 0x002ac558
// Steps a normal transfer, ending it when no data remains.
static int stepNormal(void) {
    if (g_pDmaChannel->qwc == 0) {
        stop();
        return 0;
    }
    return transferData();
}

int DMA_step(void) {
    g_pDmaQword = NULL;
    g_nDmaQwordIsTag = 0;
    if (((g_pDmaChannel->chcr >> CHCR_MOD_SHIFT) & CHCR_MOD_MASK) == CHCR_MOD_NORMAL) {
        return stepNormal();
    }
    return stepChain();
}
