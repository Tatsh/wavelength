#pragma once

#include "softfx_s/sampleblock.h"

/**
 * The softFX module captures the output of SPU2 core 0 block by block, runs one effect over each
 * block, and writes the result back to SPU2 memory. The EE controls it through a SIF RPC server.
 */

/** SIF RPC server identifier of the module. */
constexpr unsigned int kSoftFxRpcServer = 0x75433776;

/** RPC function numbers. The argument buffer is an int unless noted. */
enum SoftFxCommand {
    kSoftFxCommandInit = 100,          /*!< Start the module. The buffer is an InitArgs. */
    kSoftFxCommandShutdown = 101,      /*!< Does nothing. */
    kSoftFxCommandStart = 102,         /*!< Start capturing. */
    kSoftFxCommandStop = 103,          /*!< Stop capturing after a few more interrupts. */
    kSoftFxCommandIgnored = 104,       /*!< Does nothing. */
    kSoftFxCommandStreamData = 105,    /*!< Append the buffer to the stream; empty ends it. */
    kSoftFxCommandSetEffect = 106,     /*!< Select an Effect. */
    kSoftFxCommandSetParams = 107,     /*!< Two EffectParams, left then right. */
    kSoftFxCommandConcatenation = 108, /*!< A run of CommandHeader records and their data. */
    kSoftFxCommandRouteInput = 109,    /*!< Select the core 0 mixer input pair, 0 to 2. */
    kSoftFxCommandRouteVoices = 110,   /*!< Nonzero sets mixer bits 8 and 9 of core 0. */
    kSoftFxCommandSetVolume = 111,     /*!< Set the captured input volume. */
    kSoftFxCommandSetMonoStream = 112, /*!< Nonzero mixes the stream to mono. */
};

/** Effects a captured block can run through. */
enum Effect {
    kEffectSilence = 0,         /*!< Output silence. */
    kEffectStream = 1,          /*!< Output the stream the EE sends. */
    kEffectSweptFilter = 2,     /*!< The resonant filter; new parameters set interpolation steps. */
    kEffectStutter = 3,         /*!< Gate the input on a phase accumulator. */
    kEffectBypass = 4,          /*!< Copy the input. */
    kEffectFilter = 5,          /*!< The resonant filter. */
    kEffectEcho = 6,            /*!< A mono echo with two taps. */
    kEffectFilterVariant = 7,   /*!< The resonant filter. */
    kEffectTrackingEcho = 8,    /*!< An echo whose taps follow zero crossings. */
    kEffectDistortedFilter = 9, /*!< The resonant filter with a hard clip. */
};

/** Argument of #kSoftFxCommandInit. */
struct InitArgs {
    unsigned char mOption; /*!< Recorded and never read. */
    void *mStreamStatus;   /*!< EE address that receives the stream's free space. */
};

/** Header of one command in a #kSoftFxCommandConcatenation buffer. Its data follows. */
struct CommandHeader {
    int mCommand; /*!< RPC function number. */
    int mSize;    /*!< Byte count of the data. */
};

/**
 * One transfer of captured samples. A capture reads three consecutive blocks of SPU2 memory, and
 * only the left and right blocks belong to the half of the capture buffer that was filled.
 */
struct CaptureFrame {
    SampleBlock mLeft;    /*!< The filled half of the left capture buffer. */
    SampleBlock mOverlap; /*!< The block between them, refilled from the previous frame. */
    SampleBlock mRight;   /*!< The filled half of the right capture buffer. */
};

/** The selected Effect. */
extern int g_nEffect;

/**
 * Module entry. A negative argument count unloads the module.
 *
 * @param argc Argument count, negated to unload.
 * @param argv Arguments.
 * @return A ModuleStartResult.
 * @ghidraAddress NTSC-U/C: 0x00000114
 * @ghidraAddress PAL: 0x00000114
 */
extern "C" int start(int argc, char **argv);
