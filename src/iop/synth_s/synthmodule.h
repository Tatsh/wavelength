#pragma once

/**
 * Loading and unloading of the synth module.
 *
 * The class has no RTTI, and its name is inferred from its role. Every member is static. The module
 * never runs its global constructors. The synthesiser's tables start zeroed, and Synth::Init()
 * prepares them.
 */
class SynthModule {
public:
    /**
     * Enable the SPU2 DMA interrupts, start the RPC layer, and start the server thread.
     *
     * @param argc Argument count.
     * @param argv Arguments.
     * @return REMOVABLE_RESIDENT_END once the server thread runs, otherwise NO_RESIDENT_END.
     * @ghidraAddress NTSC-U/C: 0x00000130
     * @ghidraAddress PAL: 0x00000130
     */
    static int Load(int argc, char **argv);

    /**
     * Stop and delete the server thread when the first argument is "other".
     *
     * @param argc Argument count.
     * @param argv Arguments.
     * @return NO_RESIDENT_END when the module unloaded or the argument differs, otherwise
     * REMOVABLE_RESIDENT_END.
     * @ghidraAddress NTSC-U/C: 0x000001c0
     * @ghidraAddress PAL: 0x000001c0
     */
    static int Unload(int argc, char **argv);

private:
    static int sServerThreadId; /*!< The server thread, or -1 before Load(). */
};

/**
 * The module's entry point. A negative @p argc requests an unload.
 *
 * @param argc Argument count, negated for an unload.
 * @param argv Arguments.
 * @return A ModuleStartResult.
 * @ghidraAddress NTSC-U/C: 0x00000230
 * @ghidraAddress PAL: 0x00000230
 */
extern "C" int start(int argc, char **argv);
