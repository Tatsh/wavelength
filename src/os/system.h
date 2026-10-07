#pragma once

#include "os/timer.h"
#include "script/dataarray.h"

/**
 * Bring up the operating layer from the command line and the configuration file.
 *
 * The configuration file is read before every other subsystem starts, and the system clock is
 * reset along the way.
 *
 * @param argc The argument count main() received.
 * @param argv The argument vector main() received.
 * @param pszConfigFile The configuration file to read, relative to the disc root.
 * @ghidraAddress NTSC-U/C: 0x0028cb38
 * @ghidraAddress PAL: 0x002964e8
 */
void SystemInit(int argc, char **argv, const char *pszConfigFile);

/**
 * Shut down what SystemInit() started, in reverse order, and release the configuration.
 *
 * @ghidraAddress NTSC-U/C: 0x0028ccc8
 * @ghidraAddress PAL: 0x00296678
 */
void SystemTerminate();

/**
 * Service the operating layer once per frame, controllers included.
 *
 * @ghidraAddress NTSC-U/C: 0x0028c560
 * @ghidraAddress PAL: 0x00295ed0
 */
void SystemPoll();

/**
 * Turn the periodic controller connection check of SystemPoll() on or off.
 *
 * The name is inferred.
 *
 * @param bEnabled Whether SystemPoll() checks which controllers are connected.
 * @ghidraAddress NTSC-U/C: 0x0028c4c0
 * @ghidraAddress PAL: 0x00295e10
 */
void SystemSetPadCheck(bool bEnabled);

/**
 * Report the configuration SystemInit() read.
 *
 * @return The root of the configuration.
 * @ghidraAddress NTSC-U/C: 0x0028c6a0
 * @ghidraAddress PAL: 0x00296028
 */
DataArray *SystemConfig();

/**
 * Report the language the console is set to.
 *
 * @return The language key, for example "english".
 * @ghidraAddress NTSC-U/C: 0x0028caf8
 * @ghidraAddress PAL: 0x002964a8
 */
const char *GetSystemLanguage();

/**
 * Report the configuration SystemInit() read.
 *
 * @return The root of the configuration.
 * @ghidraAddress NTSC-U/C: 0x0028c6a0
 * @ghidraAddress PAL: 0x00296028
 */
DataArray *SystemConfig();

/**
 * The clock SystemMs() reads.
 *
 * @ghidraAddress NTSC-U/C: 0x00491a10
 */
extern Timer gSystemTimer;

/**
 * Cycles accumulated across every SystemMs() call since the clock was last reset.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2260
 */
extern unsigned long long gSystemCycles;

/**
 * Milliseconds per EE cycle.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2258
 */
extern float gSystemCycles2Ms;

/**
 * Report the milliseconds the system clock has measured since it was last reset.
 *
 * The body is here rather than in a source file because the engine expands it inline at most call
 * sites. The address is the one out-of-line copy.
 *
 * @return The elapsed milliseconds.
 * @ghidraAddress NTSC-U/C: 0x001afae0
 * @ghidraAddress PAL: 0x001b8880
 */
inline float SystemMs() {
    gSystemTimer.Split();
    gSystemCycles += gSystemTimer.mCycles;
    return static_cast<float>(gSystemCycles) * gSystemCycles2Ms;
}
