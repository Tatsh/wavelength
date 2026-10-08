#pragma once

#include "utl/FileStream.h"
#include "utl/PrnStream.h"

/**
 * Handler that replaces the message box of Debug::Modal().
 *
 * The handler may change the kind of message. The program exits when the kind is
 * Debug::kModalFail after the handler returns.
 *
 * @param type The kind of message.
 * @param message The text.
 */
typedef void (*ModalCallbackFunc)(int &type, const char *message);

/**
 * Stream for diagnostic text and failure reports.
 *
 * The RTTI records the class as deriving from PrnStream. The object is 0x14 bytes. Text goes to
 * the log file when one is open, and otherwise to the console and the debugger.
 */
class Debug : public PrnStream {
public:
    /** Kinds of message Modal() shows. */
    enum ModalType {
        kModalFail = 0,       /*!< A failure. The program exits after the message. */
        kModalNotify = 1,     /*!< A notice the user acknowledges. */
        kModalCancelable = 2, /*!< A notice the user may turn into a failure with Cancel. */
    };

    /**
     * Create the stream with output enabled and no log.
     *
     * @ghidraAddress 0x1000a5b0
     */
    Debug();

    /**
     * Close the log.
     *
     * @ghidraAddress 0x1000a5f0
     */
    virtual ~Debug();

    /**
     * Write text to the log, converting each line feed to a carriage return and a line feed, or
     * to the console and the debugger when no log is open.
     *
     * @param str The text.
     * @ghidraAddress 0x1000a440
     */
    virtual void Print(const char *str);

    /**
     * Report a failure and exit, or throw the message when #mThrowOnFail is set.
     *
     * A failure while one is being reported is ignored.
     *
     * @param fmt The `printf` format.
     * @ghidraAddress 0x1000a310
     */
    void Fail(const char *fmt, ...);

    /**
     * Show a notice.
     *
     * @param fmt The `printf` format.
     * @ghidraAddress 0x1000a2f0
     */
    void Notify(const char *fmt, ...);

    /**
     * Show a message through #mModalCallback or a message box, and exit when the message
     * remains a failure.
     *
     * @param type The kind of message, one of ModalType.
     * @param message The text.
     * @ghidraAddress 0x1000a360
     */
    void Modal(int type, const char *message);

    /**
     * Close any log and open a new one.
     *
     * When the file does not open, the stream reports it and continues without a log.
     *
     * @param file The path of the log.
     * @ghidraAddress 0x1000a4b0
     */
    void StartLog(const char *file);

    /**
     * Close the log.
     *
     * @ghidraAddress 0x1000a550
     */
    void StopLog();

    bool mFailing;                    /*!< Whether a failure is being reported. */
    bool mThrowOnFail;                /*!< Whether Fail() throws the message instead of exiting. */
    bool mEnabled;                    /*!< Whether Print() writes anything. */
    FileStream *mLog;                 /*!< The log, or null. */
    ModalCallbackFunc mModalCallback; /*!< The handler of Modal(), or null for message boxes. */
};

/**
 * The diagnostic stream.
 *
 * @ghidraAddress 0x1003e918
 */
extern Debug TheDebug;

/**
 * Format a diagnostic message that this build discards.
 *
 * The body is empty. The linker shares it with every other empty routine of the program.
 *
 * @param fmt The `printf` format.
 * @ghidraAddress 0x1000fb60
 */
void DebugPrint(const char *fmt, ...);

/**
 * Report a failure when a condition is false.
 *
 * @param cond The condition.
 */
#define ASSERT(cond)                                                                               \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            TheDebug.Fail("File: %s Line: %d Error: %s\n", __FILE__, __LINE__, #cond);             \
        }                                                                                          \
    } while (false)

/**
 * Report a failure when a value is outside a half-open range.
 *
 * @param x The value.
 * @param low The lowest valid value.
 * @param high One past the highest valid value.
 */
#define ASSERT_RANGE(x, low, high) ASSERT((low) <= (x) && (x) < (high))
