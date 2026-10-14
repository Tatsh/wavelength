#ifndef DMADEBUG_DEBUG_H
#define DMADEBUG_DEBUG_H

/**
 * Report output of the DMAdebug packet checker.
 *
 * The messages of the image ("DMAdebug - too many errors, exiting." and "LOGIC ERROR in
 * DEBUG_addField") establish that the checker is vendored into it. The routine names other than
 * DEBUG_addField() are inferred.
 *
 * Reports are built a field at a time into a line with DEBUG_addField() and then appended to the
 * output buffer with DEBUG_print(). Messages that the checker raises go through DEBUG_message().
 * DEBUG_message() counts them and returns DEBUG_FATAL once a limit is passed.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Terminal escape sequences the reports are coloured with. */
#define DEBUG_ANSI_RESET "\033[0m"
#define DEBUG_ANSI_BOLD "\033[1m"
#define DEBUG_ANSI_YELLOW "\033[33m"
#define DEBUG_ANSI_GREEN "\033[32m"
#define DEBUG_ANSI_MAGENTA "\033[35m"
#define DEBUG_ANSI_CYAN "\033[36m"
#define DEBUG_ANSI_WHITE "\033[37m"
#define DEBUG_ANSI_ON_GREEN "\033[42m\033[30m"
#define DEBUG_ANSI_ON_MAGENTA "\033[45m\033[30m"
#define DEBUG_ANSI_ON_CYAN "\033[46m\033[30m"
#define DEBUG_ANSI_ON_WHITE "\033[47m\033[30m"
#define DEBUG_ANSI_ON_RED "\033[41m\033[30m"
#define DEBUG_ANSI_ALERT "\033[41m\033[30m\033[5m"
#define DEBUG_ANSI_CAUTION "\033[43m\033[30m\033[5m"

/**
 * Print a source line and a message, and end the program.
 *
 * @param line The line of the original source file the message names.
 * @param message A format with no conversions.
 */
#define DEBUG_ABORT(line, message)                                                                 \
    do {                                                                                           \
        printf("Line %d: ", line);                                                                 \
        printf(message);                                                                           \
        printf("\n");                                                                              \
        exit(1);                                                                                   \
    } while (0)

/** Severity of a message, and the flags DEBUG_message() and DEBUG_print() return. */
enum {
    DEBUG_ERROR = 2,   /*!< A message counted as an error. */
    DEBUG_FATAL = 4,   /*!< Too many messages, or no room for the output. */
    DEBUG_WARNING = 8, /*!< A message counted as a warning, unless warnings are errors. */
};

/** How DEBUG_addField() places a field in a line. */
enum {
    DEBUG_FIELD_COLUMN = 0, /*!< Append, padded to a 24-character column. */
    DEBUG_FIELD_INLINE = 1, /*!< Append after a space, wrapping a line longer than 100. */
    DEBUG_FIELD_LINE = 2,   /*!< Start a new line indented to the first column. */
};

/** Width of the first column of a report line. */
enum { DEBUG_COLUMN_WIDTH = 24 };

/** Capacity of the warning numbers the -W option can mask. */
enum { DEBUG_MAX_WARNING_MASKS = 100 };

/** Capacity of g_szDebugScratch. */
enum { DEBUG_SCRATCH_SIZE = 1024 };

/**
 * Text the checker formats a message into before reporting it.
 *
 * @ghidraAddress NTSC-U/C: 0x00518718
 */
extern char g_szDebugScratch[DEBUG_SCRATCH_SIZE];

/**
 * Configure the module and allocate the output buffer.
 *
 * The options are found anywhere in the text: -w masks every warning, -E followed by a number sets
 * how many errors end the program, -W followed by a number masks that warning, -We counts warnings
 * as errors, -S sends messages to the output buffer, -B followed by a number sets the buffer size
 * in kilobytes, and -noscroll stops the buffer from discarding old lines when it fills.
 *
 * @param options The options, or null for the defaults.
 * @ghidraAddress NTSC-U/C: 0x002a0920
 * @ghidraAddress PAL: 0x002aa890
 */
void DEBUG_init(const char *options);

/**
 * Empty the output buffer and the pending message line.
 *
 * @ghidraAddress NTSC-U/C: 0x002a0d40
 * @ghidraAddress PAL: 0x002aacb0
 */
void DEBUG_reset(void);

/**
 * Append a line to the output buffer.
 *
 * When the line does not fit, the oldest 20 lines are discarded, unless -noscroll was given.
 *
 * @param text The line, without its newline.
 * @return 0, or DEBUG_FATAL when the buffer is full and may not scroll.
 * @ghidraAddress NTSC-U/C: 0x002a0d88
 * @ghidraAddress PAL: 0x002aacf8
 */
int DEBUG_print(const char *text);

/**
 * Report an error or a warning.
 *
 * A warning whose number -W masked, or any warning after -w, is not reported.
 *
 * @param number The message number.
 * @param severity DEBUG_WARNING, or another value for an error.
 * @param text The message.
 * @return severity, with DEBUG_FATAL added when the limit of messages is passed, and with the
 *     result of DEBUG_print() added after -S. 0 for a masked warning.
 * @ghidraAddress NTSC-U/C: 0x002a0f48
 * @ghidraAddress PAL: 0x002aaeb8
 */
int DEBUG_message(int number, int severity, const char *text);

/**
 * Format a field and add it to a report line.
 *
 * Each conversion of the format reads one int argument. The routine writes to the format while it
 * formats the field and restores it afterwards. The format must not be read-only.
 *
 * @param placement DEBUG_FIELD_COLUMN, DEBUG_FIELD_INLINE, or DEBUG_FIELD_LINE.
 * @param line The line to add the field to.
 * @param format The field format.
 * @ghidraAddress NTSC-U/C: 0x002a1200
 * @ghidraAddress PAL: 0x002ab170
 */
void DEBUG_addField(int placement, char *line, char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
