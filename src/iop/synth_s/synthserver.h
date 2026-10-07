#pragma once

#include <sifrpc.h>

#include "synth_s/synthcommand.h"

/**
 * The synthesiser's SIF RPC server, identifier 0x75433178.
 *
 * The class has no RTTI, and its name is inferred from its role. Every member is static. The EE
 * calls it with a SynthCommand function number and the command's argument.
 */
class SynthServer {
public:
    static constexpr unsigned int kServerId = 0x75433178; /*!< The server identifier. */
    static constexpr int kBufferSize = 65536; /*!< Bytes of argument a call can include. */

    /**
     * Register the server and serve calls forever, as the server thread.
     *
     * @ghidraAddress NTSC-U/C: 0x00000264
     * @ghidraAddress PAL: 0x00000264
     */
    static void Thread();

    /**
     * Unregister the server and its queue. It is never called.
     *
     * @ghidraAddress NTSC-U/C: 0x000002d0
     * @ghidraAddress PAL: 0x000002d0
     */
    static void Remove();

    /**
     * Run one command.
     *
     * @param command A SynthCommand.
     * @param buffer The command's argument.
     * @param size Bytes of argument.
     * @return The command's status word as the reply buffer. Every command reports zero.
     * @ghidraAddress NTSC-U/C: 0x00000310
     * @ghidraAddress PAL: 0x00000310
     */
    static void *Dispatch(unsigned int command, void *buffer, int size);

    /**
     * Start the reverb thread, record the EE's notification addresses, and start the
     * synthesiser and its tick thread.
     *
     * @param command The command's argument.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00000640
     * @ghidraAddress PAL: 0x00000640
     */
    static int Initialize(const SynthInitCommand *command);

    /**
     * Stop the tick thread and free the bank pools.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x000006d8
     * @ghidraAddress PAL: 0x000006d8
     */
    static int Terminate();

private:
    static int sConcatenating;                 /*!< Nonzero while a concatenation runs. */
    static sceSifQueueData sQueue;             /*!< The server thread's queue. */
    static sceSifServeData sServer;            /*!< The registered server. */
    static unsigned char sBuffer[kBufferSize]; /*!< Receives each call's argument. */
};
