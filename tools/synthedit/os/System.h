#pragma once

#include "utl/Data.h"
#include "utl/OptionProcessor.h"
#include "utl/Str.h"

/**
 * Split a command line into arguments and start the systems, as the argument form of SystemInit()
 * does.
 *
 * @param commandLine The command line.
 * @param configFile The configuration file in the program's directory, or null for
 * `default_config.txt`.
 * @ghidraAddress 0x1000a680
 */
void SystemInit(const char *commandLine, const char *configFile);

/**
 * Start the systems: options, timer, strings, archive, configuration, memory, joypads, and
 * cheats.
 *
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @param configFile The configuration file in the program's directory, or null for
 * `default_config.txt`.
 * @ghidraAddress 0x1000a6d0
 */
void SystemInit(int argc, char **argv, const char *configFile);

/**
 * Stop the systems SystemInit() started and release the configuration.
 *
 * @ghidraAddress 0x1000a7d0
 */
void SystemTerminate();

/**
 * Report whether files are read from the disc rather than the host.
 *
 * @return Whether the `cd` option is set.
 * @ghidraAddress 0x1000b640
 */
bool UsingCD();

/**
 * Choose whether files are read from the disc.
 *
 * @param usingCD Whether files come from the disc.
 * @ghidraAddress 0x1000b650
 */
void SetUsingCD(bool usingCD);

/**
 * Register the system options (`host_config` and `file_order`) and process the arguments.
 *
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @ghidraAddress 0x1000b600
 */
void SystemProcessOptions(int argc, char **argv);

/**
 * Read the configuration file into SystemConfig().
 *
 * @param path The directory of the file.
 * @param file The file, or null for `default_config.txt`.
 * @ghidraAddress 0x1000b660
 */
void SystemConfigInit(const char *path, const char *file);

/**
 * Report the configuration SystemConfigInit() read.
 *
 * @return The configuration.
 * @ghidraAddress 0x1000b760
 */
DataArray *SystemConfig();

/**
 * Report the option processor that SystemInit() runs over the arguments.
 *
 * @return The option processor.
 * @ghidraAddress 0x1000b750
 */
OptionProcessor *SystemOptions();

/**
 * Value of the `file_order` option, also the name of the file log.
 *
 * @ghidraAddress 0x1003fdc0
 */
extern String gFileOrder;

/**
 * Measure the cycle counter against the performance counter and reset the clock.
 *
 * @ghidraAddress 0x1000cce0
 */
void TimerInit();

/**
 * Create the named timers the `timer` array of the configuration lists.
 *
 * @ghidraAddress 0x1000cde0
 */
void TimerConfigInit();

/**
 * Advance the clock by the cycles since the last call and poll the input.
 *
 * @ghidraAddress 0x1000cb90
 */
void TimerPoll();

/**
 * Stop the input polling the clock drives.
 *
 * @ghidraAddress 0x1000cc30
 */
void TimerTerminate();

/**
 * Seconds of cycles counted since TimerInit().
 *
 * @ghidraAddress 0x10040c80
 */
extern float gSystemTime;

/**
 * Map players to controller ports for a single controller.
 *
 * @ghidraAddress 0x1000b7b0
 */
void PadMapInit();

/**
 * Map players to controller ports and slots.
 *
 * @param multitap Whether the four players share port 0 through a multitap, one slot each.
 * @param secondPort Without a multitap, whether the second player uses port 1.
 * @ghidraAddress 0x1000b7e0
 */
void PadMapSet(bool multitap, bool secondPort);

/**
 * Release the controller map. The body is empty.
 *
 * @ghidraAddress 0x1000b7d0
 */
void PadMapTerminate();

/**
 * Do nothing.
 *
 * The linker merged every empty routine of the program into this one, and several start-up and
 * shut-down steps call it.
 *
 * @ghidraAddress 0x1000eec0
 */
void EmptyRoutine();

/**
 * Return zero.
 *
 * The linker merged every routine that only returns zero into this one. The input and disc
 * polling of this build call it.
 *
 * @return Zero.
 * @ghidraAddress 0x1000eed0
 */
int ReturnZero();
