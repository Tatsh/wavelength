#pragma once

#include "os/prnstream.h"

/**
 * Stream for diagnostic text.
 *
 * The RTTI records the class as deriving from PrnStream. The object is 0x1c bytes. The constructor
 * at `0x00289020` clears every word and sets the word at `+0x10` to 1. The meaning of those words
 * is not yet recovered, and they are not declared.
 */
class Debug : public PrnStream {
public:
    /**
     * Release the stream.
     *
     * @ghidraAddress NTSC-U/C: 0x00289050
     * @ghidraAddress PAL: 0x00292848
     */
    ~Debug() override;

    /**
     * Write diagnostic text.
     *
     * @param pszText The text, written unchanged.
     * @ghidraAddress NTSC-U/C: 0x00288e60
     */
    void Print(const char *pszText) override;
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
