#pragma once

#include "synth_s/banksampledesc.h"
#include "synth_s/program.h"

/**
 * A program loaded into a bank, linked to its run of sample descriptors.
 *
 * The class has no RTTI, and its name is inferred from its role. The program pool hands out raw
 * storage. The constructor never runs, and Init() prepares each program.
 */
class BankProgram : public Program {
public:
    /**
     * Reset the program and clear its descriptors.
     *
     * @ghidraAddress NTSC-U/C: 0x00001ed4
     * @ghidraAddress PAL: 0x00001ed4
     */
    void Init();

    /** The program's first descriptor in the bank's table, followed by the rest of its run. */
    BankSampleDesc **mSampleDescs;
};
