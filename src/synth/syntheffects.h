#pragma once

#include "script/dataarray.h"

/**
 * Settings of the two effect buses of the sound output, read from an `effects` entry of the
 * configuration.
 *
 * The class is not polymorphic and has no RTTI, so its name is inferred. Each bus has a filter, a
 * mode, a depth for each side, a delay, and a feedback. Only the members the metagame uses are
 * declared. The object starts with a vector that its inline destructor releases.
 */
class SynthEffects {
public:
    /**
     * Build the settings with both buses off.
     *
     * @ghidraAddress NTSC-U/C: 0x0027b668
     * @ghidraAddress PAL: 0x00285098
     */
    SynthEffects();

    /**
     * Read the settings of the buses from an `effects` entry.
     *
     * @param pConfig The entry, with its `bus_1` and `bus_2` entries.
     * @ghidraAddress NTSC-U/C: 0x0027b7a8
     * @ghidraAddress PAL: 0x002851d8
     */
    void Load(DataArray *pConfig);

    /**
     * Send the settings to the sound output.
     *
     * @ghidraAddress NTSC-U/C: 0x0027c2f0
     * @ghidraAddress PAL: 0x00285d20
     */
    void Apply();
};
