#pragma once

#include "os/binstream.h"
#include "os/prnstream.h"

/**
 * Stream for diagnostic text, written to standard output or to a log file.
 *
 * The RTTI records the class as deriving from PrnStream, and the vtable is at `0x003d68c8`. The
 * object is 0x1c bytes.
 */
class Debug : public PrnStream {
public:
    /**
     * Construct an enabled stream with no log file.
     *
     * @ghidraAddress NTSC-U/C: 0x00289020
     * @ghidraAddress PAL: 0x00292818
     */
    Debug();

    /**
     * Close the log file and release the stream.
     *
     * @ghidraAddress NTSC-U/C: 0x00289050
     * @ghidraAddress PAL: 0x00292848
     */
    ~Debug() override;

    /**
     * Write diagnostic text, to the log file with each newline as a carriage return and a line
     * feed, or to standard output. A disabled stream writes nothing.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x00288e60
     */
    void Print(const char *pszText) override;

    /**
     * Write the text to a log file from now on, replacing any log file. A file that cannot be
     * opened reports a notice and leaves no log file.
     *
     * The name is inferred.
     *
     * @param pszFile The file.
     * @ghidraAddress NTSC-U/C: 0x00288f38
     */
    void OpenLog(const char *pszFile);

    /**
     * Close the log file, if any.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00288fd8
     * @ghidraAddress PAL: 0x002927c8
     */
    void CloseLog();

private:
    int mReserved08; // +0x08, cleared by the constructor and not yet identified.
    int mReserved0C; // +0x0c, cleared by the constructor and not yet identified.
    int mEnabled;    // Non-zero while the stream writes.
    BinStream *mLog; // The log file, or null for standard output.
    int mReserved18; // +0x18, cleared by the constructor and not yet identified.
};

/**
 * The diagnostic stream.
 *
 * @ghidraAddress NTSC-U/C: 0x0047fa78
 */
extern Debug TheDebug;

/**
 * Format a diagnostic message that the shipped build discards.
 *
 * The body is empty. Each of the 140 call sites still evaluates its arguments and makes the call.
 *
 * @param pszFormat The `printf` format.
 * @ghidraAddress NTSC-U/C: 0x00333b08
 * @ghidraAddress PAL: 0x003a10b8
 */
void DebugPrint(const char *pszFormat, ...);

/**
 * Format a warning that the shipped build discards.
 *
 * The body is empty. Each of the 159 call sites still evaluates its arguments and makes the call.
 * The callers pass messages about invalid data and unsupported requests. The name is inferred.
 *
 * @param pszFormat The `printf` format.
 * @ghidraAddress NTSC-U/C: 0x00333d50
 * @ghidraAddress PAL: 0x003a1300
 */
void DebugWarn(const char *pszFormat, ...);

/**
 * Format a notice that the shipped build discards.
 *
 * The body is empty. The callers pass reports of missing data, skipped objects, and failed loads.
 * The name is inferred.
 *
 * @param pszFormat The `printf` format.
 * @ghidraAddress NTSC-U/C: 0x00339f48
 * @ghidraAddress PAL: 0x003a7480
 */
void DebugNotify(const char *pszFormat, ...);

/**
 * Format an error that the shipped build discards.
 *
 * The body is empty. The callers pass errors in the song files and the start-up configuration
 * that no error handler takes. The name is inferred.
 *
 * @param pszFormat The `printf` format.
 * @ghidraAddress NTSC-U/C: 0x00339f80
 * @ghidraAddress PAL: 0x003a74b8
 */
void DebugError(const char *pszFormat, ...);
