#pragma once

#include "synth_s/sample.h"
#include "synth_s/sampledesc.h"

/**
 * A sample descriptor loaded into a bank, linked to the sample it plays.
 *
 * The class has no RTTI, and its name is inferred from its role. The descriptor pool hands out
 * raw storage. The constructor never runs, and Init() prepares each descriptor.
 */
class BankSampleDesc : public SampleDesc {
public:
    /**
     * Print the descriptor and its sample to the console.
     *
     * @ghidraAddress NTSC-U/C: 0x00001e70
     * @ghidraAddress PAL: 0x00001e70
     */
    void Dump() const;

    /**
     * Reset the descriptor and clear its sample.
     *
     * @ghidraAddress NTSC-U/C: 0x00001eac
     * @ghidraAddress PAL: 0x00001eac
     */
    void Init();

    /**
     * Report the sample the descriptor plays.
     *
     * @return The sample, or null before the bank links one.
     * @ghidraAddress NTSC-U/C: 0x00002094
     * @ghidraAddress PAL: 0x00002094
     */
    Sample *GetSample() const;

    Sample *mSample; /*!< The sample #mSampleIndex selects, once the bank links it. */
};
