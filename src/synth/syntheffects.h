#pragma once

#include <vector>

#include "script/dataarray.h"
#include "synth/softfxfilter.h"

/**
 * Settings of the two effect buses and the software effect of the sound output, read from an
 * `effects` entry of the configuration.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object is 0x6c bytes.
 * Each bus has a mode, a depth for each side, a delay, and a feedback. Load() packs them into
 * control change messages on the bus's MIDI channel. The first bus also provides the software
 * effect.
 */
class SynthEffects {
public:
    /** The number of effect buses. */
    static constexpr int kNumBuses = 2;

    /**
     * Build the settings with both buses off.
     *
     * @ghidraAddress NTSC-U/C: 0x0027b668
     * @ghidraAddress PAL: 0x00285098
     */
    SynthEffects();

    /**
     * Build the settings with both buses off, then read them from an `effects` entry.
     *
     * @param pConfig The entry, as Load() takes it.
     * @ghidraAddress NTSC-U/C: 0x0027b6f8
     * @ghidraAddress PAL: 0x00285128
     */
    explicit SynthEffects(DataArray *pConfig);

    /**
     * Read the settings of the buses from an `effects` entry.
     *
     * The entry of bus n is `remix_bus_<n>` while a remix plays, then `bus_<n>`, and then the
     * first child entry whose `bus` is n. Each bus entry lists `mode`, `depth_right`,
     * `depth_left`, `delay`, and `feedback`. The entry of bus 1 also lists `filter`,
     * `soft_fx_output`, `fx_bus_to_softfx`, and `fx_bus_to_corefx`.
     *
     * @param pConfig The entry.
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

    std::vector<unsigned int> mMessages; /*!< The packed control changes of the buses. */
    SoftFxFilter mFilter;                /*!< The software effect, from `filter` of bus 1. */
    int mSoftFxOutput;                   /*!< `soft_fx_output` of bus 1. */
    int mBusToSoftFx;                    /*!< `fx_bus_to_softfx` of bus 1. */
    int mBusToCoreFx;                    /*!< `fx_bus_to_corefx` of bus 1. */
};
