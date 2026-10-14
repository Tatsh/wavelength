#include "dmadebug/debug.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <malloc.h>

// Errors allowed before the program ends, unless -E gives another count.
enum { DEFAULT_MAX_ERRORS = 6 };

// Warnings allowed before the program ends.
enum { MAX_WARNINGS = 25 };

// Default size of the output buffer in kilobytes.
enum { DEFAULT_BUFFER_SIZE_KB = 100 };

// Bytes that must remain free after an appended line when the output buffer is tested for room.
enum { BUFFER_MARGIN = 100 };

// Lines the output buffer discards when it scrolls.
enum { SCROLL_LINES = 20 };

// Width beyond which DEBUG_FIELD_INLINE starts a new line.
enum { LINE_WIDTH = 100 };

// Capacity of the line that collects messages without -S.
enum { MESSAGE_LINE_SIZE = 10240 };

// Capacity of a formatted field or message.
enum { FIELD_SIZE = 1024 };

// NTSC-U/C: 0x003b24d8
// Non-zero once DEBUG_init() has run.
static int g_nDebugModuleInited = 0;

// NTSC-U/C: 0x003b24dc
// Entries of g_anDebugWarnMasks in use, or -1 when every warning is masked.
static int g_nDebugWarnMaskCount = 0;

// NTSC-U/C: 0x003b24e0
// The output buffer, g_nDebugBufferSizeKb kilobytes.
static char *g_pDebugOutputBuffer = NULL;

// NTSC-U/C: 0x003b24e4
// Non-zero when a full output buffer discards its oldest lines.
static int g_nDebugScrollEnabled = 1;

// NTSC-U/C: 0x00515f18
// Messages collected without -S.
static char g_szDebugOutput[MESSAGE_LINE_SIZE];

char g_szDebugScratch[DEBUG_SCRATCH_SIZE];

// NTSC-U/C: 0x00518b18
// Errors allowed before the program ends.
static int g_nDebugMaxErrors;

// NTSC-U/C: 0x00518b1c
// Non-zero when warnings count as errors.
static int g_nDebugWarningsAsErrors;

// NTSC-U/C: 0x00518b20
// Non-zero when messages go to the output buffer.
static int g_nDebugBufferOutput;

// NTSC-U/C: 0x00518b24
// Errors reported since DEBUG_init().
static int g_nDebugErrorCount;

// NTSC-U/C: 0x00518b28
// Warnings reported since DEBUG_init().
static int g_nDebugWarnCount;

// NTSC-U/C: 0x00518b30
// Numbers of the warnings that -W masked.
static int g_anDebugWarnMasks[DEBUG_MAX_WARNING_MASKS];

// NTSC-U/C: 0x00518cc0
// The message DEBUG_message() formats.
static char g_szDebugFormatScratch[FIELD_SIZE];

// NTSC-U/C: 0x005190c0
// Size of the output buffer in kilobytes.
static int g_nDebugBufferSizeKb;

// NTSC-U/C: 0x005190c4
// The end of the text in the output buffer.
static char *g_pDebugOutputWritePtr;

// NTSC-U/C: 0x002a0d00, PAL: 0x002aac70
// Initialises the module for a caller that did not, with output buffered, and complains twice.
static void initUninitialised(void) {
    DEBUG_init("-S");
    puts(DEBUG_ANSI_ALERT "WARNING WARNING: An attempt was made to use the DEBUG module without "
                          "initialising it first!" DEBUG_ANSI_RESET);
    puts(DEBUG_ANSI_ALERT "WARNING WARNING: An attempt was made to use the DEBUG module without "
                          "initialising it first!" DEBUG_ANSI_RESET);
}

void DEBUG_init(const char *options) {
    g_szDebugOutput[0] = '\0';
    if (g_pDebugOutputBuffer != NULL) {
        free(g_pDebugOutputBuffer);
    }
    g_nDebugErrorCount = 0;
    g_nDebugWarnCount = 0;
    g_nDebugMaxErrors = DEFAULT_MAX_ERRORS;
    g_nDebugBufferSizeKb = DEFAULT_BUFFER_SIZE_KB;
    g_nDebugScrollEnabled = 1;
    g_nDebugWarningsAsErrors = 0;
    g_nDebugBufferOutput = 0;
    g_nDebugWarnMaskCount = 0;
    if (options != NULL) {
        if (strstr(options, "-w") != NULL) {
            g_nDebugWarnMaskCount = -1;
        }
        const char *option = strstr(options, "-E");
        if (option != NULL) {
            g_nDebugMaxErrors = 0;
            for (option += 2; (unsigned int)(*option - '0') < 10; ++option) {
                // Yes, the binary adds the character code rather than the digit value.
                g_nDebugMaxErrors = g_nDebugMaxErrors * 10 + *option;
            }
        }
        option = strstr(options, "-W");
        if (option != NULL) {
            option += 2;
            if (*option == '\0') {
                DEBUG_ABORT(210, "Invalid debug initialisation string at '-W'!");
            }
            if (*option == 'e') {
                g_nDebugWarningsAsErrors = 1;
            } else {
                int mask = 0;
                for (; isdigit((int)*option); ++option) {
                    mask = mask * 10 + (*option - '0');
                }
                if (mask != 0 && g_nDebugWarnMaskCount >= 0) {
                    if (g_nDebugWarnMaskCount >= DEBUG_MAX_WARNING_MASKS) {
                        DEBUG_ABORT(
                            222, "Invalid initialisation string at '-W'! Too many warning masks!");
                    }
                    g_anDebugWarnMasks[g_nDebugWarnMaskCount++] = mask;
                }
            }
        }
        if (strstr(options, "-S") != NULL) {
            g_nDebugBufferOutput = 1;
        }
        option = strstr(options, "-B");
        if (option != NULL) {
            g_nDebugBufferSizeKb = 0;
            for (option += 2; isdigit((int)*option); ++option) {
                g_nDebugBufferSizeKb = g_nDebugBufferSizeKb * 10 + (*option - '0');
            }
            if (g_nDebugBufferSizeKb == 0) {
                DEBUG_ABORT(239, "Invalid parameter for buffer size (-B)! Must be a number > 0");
            }
        }
        if (strstr(options, "-noscroll") != NULL) {
            g_nDebugScrollEnabled = 0;
        }
    }
    g_pDebugOutputBuffer = memalign(4, g_nDebugBufferSizeKb << 10);
    if (g_pDebugOutputBuffer == NULL) {
        DEBUG_ABORT(248, "ERROR in Debug module! Could not allocate output buffer");
    }
    g_nDebugModuleInited = 1;
    DEBUG_reset();
}

void DEBUG_reset(void) {
    if (!g_nDebugModuleInited) {
        initUninitialised();
    }
    g_pDebugOutputBuffer[0] = '\0';
    g_szDebugOutput[0] = '\0';
    g_pDebugOutputWritePtr = g_pDebugOutputBuffer;
}

int DEBUG_print(const char *text) {
    if (!g_nDebugModuleInited) {
        initUninitialised();
    }
    const int length = (int)strlen(text);
    if (g_pDebugOutputBuffer + (g_nDebugBufferSizeKb << 10) <
        g_pDebugOutputWritePtr + length + BUFFER_MARGIN) {
        if (!g_nDebugScrollEnabled) {
            strcat(g_pDebugOutputWritePtr,
                   "\n\n" DEBUG_ANSI_ALERT "DEBUG MODULE - OUTPUT BUFFER FULL" DEBUG_ANSI_RESET
                   "\n");
            return DEBUG_FATAL;
        }
        char *keep = g_pDebugOutputBuffer;
        int lines = SCROLL_LINES;
        while (lines > 0) {
            if (*keep++ == '\n') {
                --lines;
            }
            if (keep >= g_pDebugOutputWritePtr) {
                break;
            }
        }
        if (lines > 0 || keep >= g_pDebugOutputWritePtr) {
            keep = g_pDebugOutputWritePtr;
        }
        memmove(g_pDebugOutputBuffer, keep, strlen(keep) + 1);
        g_pDebugOutputWritePtr = g_pDebugOutputBuffer + strlen(g_pDebugOutputBuffer);
        if (g_pDebugOutputBuffer + (g_nDebugBufferSizeKb << 10) <
            g_pDebugOutputWritePtr + length + BUFFER_MARGIN) {
            DEBUG_ABORT(319,
                        "Error - could not fit string into Output Buffer, even after scrolling.");
        }
    }
    strcat(g_pDebugOutputWritePtr, text);
    strcat(g_pDebugOutputWritePtr, DEBUG_ANSI_RESET "\n");
    g_pDebugOutputWritePtr += strlen(g_pDebugOutputWritePtr);
    return 0;
}

int DEBUG_message(int number, int severity, const char *text) {
    int result = severity;
    if (!g_nDebugModuleInited) {
        initUninitialised();
    }
    if (severity == DEBUG_WARNING) {
        if (g_nDebugWarnMaskCount == -1) {
            return 0;
        }
        for (int i = 0; i < g_nDebugWarnMaskCount; ++i) {
            if (g_anDebugWarnMasks[i] == number) {
                return 0;
            }
        }
        sprintf(g_szDebugFormatScratch,
                DEBUG_ANSI_CAUTION "warning (%d) : %s" DEBUG_ANSI_RESET,
                number,
                text);
        if (g_nDebugWarningsAsErrors) {
            if (g_nDebugMaxErrors < ++g_nDebugErrorCount) {
                result = severity | DEBUG_FATAL;
                puts(DEBUG_ANSI_ALERT "DMAdebug - too many errors (treating warnings as errors), "
                                      "exiting." DEBUG_ANSI_RESET);
            }
        } else if (++g_nDebugWarnCount > MAX_WARNINGS) {
            result = severity | DEBUG_FATAL;
            puts(DEBUG_ANSI_ALERT "DMAdebug - too many warnings, exiting." DEBUG_ANSI_RESET);
        }
    } else {
        sprintf(g_szDebugFormatScratch,
                DEBUG_ANSI_ALERT "ERROR (%d) : %s" DEBUG_ANSI_RESET,
                number,
                text);
        if (g_nDebugMaxErrors < ++g_nDebugErrorCount) {
            result |= DEBUG_FATAL;
            puts(DEBUG_ANSI_ALERT "DMAdebug - too many errors, exiting." DEBUG_ANSI_RESET);
        }
    }
    if (g_nDebugBufferOutput) {
        result |= DEBUG_print(g_szDebugFormatScratch);
    } else {
        strcat(g_szDebugOutput, g_szDebugFormatScratch);
        strcat(g_szDebugOutput, "\n");
    }
    return result;
}

// NTSC-U/C: 0x002a1130, PAL: 0x002ab0a0
// Returns the length of text without its escape sequences. Each sequence ends at an 'm'.
static int visibleLength(const char *text) {
    int length = (int)strlen(text);
    while (*text != '\0') {
        if (*text == '\033') {
            do {
                ++text;
                --length;
            } while (*text != 'm');
            --length;
        }
        ++text;
    }
    return length;
}

// NTSC-U/C: 0x002a11b8, PAL: 0x002ab128
// Returns the next conversion of format, skipping each "%%", or null.
static char *nextConversion(char *format) {
    char *percent;
    while ((percent = strchr(format, '%')) != NULL) {
        if (percent[1] != '%') {
            return percent;
        }
        format = &percent[2];
    }
    return NULL;
}

void DEBUG_addField(int placement, char *line, char *format, ...) {
    char field[FIELD_SIZE];
    int conversions = 0;
    for (const char *scan = format; *scan != '\0'; ++scan) {
        if (*scan == '%') {
            if (scan[1] != '%') {
                ++conversions;
            } else {
                ++scan;
            }
        }
    }
    if (conversions == 0) {
        sprintf(field, format);
    } else {
        // Each conversion is formatted with its stretch of the format, cut off at the next one.
        va_list arguments;
        va_start(arguments, format);
        char *out = field;
        for (; conversions > 0; --conversions) {
            const int argument = va_arg(arguments, int);
            char *cut = NULL;
            if (conversions != 1) {
                cut = nextConversion(format);
                if (cut == NULL) {
                    DEBUG_ABORT(488, "Logic error - should have found at least 1 '%%'\n");
                }
                cut = nextConversion(&cut[1]);
                if (cut == NULL) {
                    DEBUG_ABORT(491, "Logic error - should have found at least 2 '%%'\n");
                }
                *cut = '\0';
            }
            out += sprintf(out, format, argument);
            if (cut != NULL) {
                *cut = '%';
                format = cut;
            }
        }
        va_end(arguments);
    }
    char *end = &line[strlen(line)];
    switch (placement) {
    case DEBUG_FIELD_COLUMN: {
        int padding = DEBUG_COLUMN_WIDTH - visibleLength(field);
        strcat(end, field);
        for (; padding > 0; --padding) {
            strcat(end, " ");
        }
        strcat(end, ":");
        break;
    }
    case DEBUG_FIELD_INLINE: {
        char *lineStart = end;
        while (line < lineStart && *lineStart != '\n') {
            --lineStart;
        }
        if (*lineStart == '\n') {
            ++lineStart;
        }
        if (visibleLength(lineStart) + visibleLength(field) > LINE_WIDTH &&
            visibleLength(lineStart) > DEBUG_COLUMN_WIDTH + 1) {
            strcat(lineStart, "\n                        :");
        }
        strcat(lineStart, " ");
        strcat(lineStart, field);
        break;
    }
    case DEBUG_FIELD_LINE:
        if (line < end && end[-1] != '\n') {
            strcat(end, "\n");
        }
        strcat(end, "                        : ");
        strcat(end, field);
        break;
    default:
        DEBUG_ABORT(549, "LOGIC ERROR in DEBUG_addField - this type does not exist.");
        break;
    }
}
