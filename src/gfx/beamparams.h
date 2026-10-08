#pragma once

#include "script/dataarray.h"

/**
 * Look of a family of beams, read from the configuration entry of its name.
 *
 * The class emits no RTTI, and the name is inferred. The object is 0x80 bytes. Only the members
 * the beam pools call are declared so far.
 */
class BeamParams {
public:
    /**
     * Construct the look of the beams of a configuration entry.
     *
     * @param pszName The entry.
     * @ghidraAddress NTSC-U/C: 0x001ecea8
     * @ghidraAddress PAL: 0x001f5c48
     */
    explicit BeamParams(const char *pszName);

    /**
     * Destroy the look.
     *
     * @ghidraAddress NTSC-U/C: 0x001ecf50
     * @ghidraAddress PAL: 0x001f5cf0
     */
    ~BeamParams();

    /**
     * Read the look from the configuration.
     *
     * @param pConfig The section of the kind of game.
     * @param pDefaults The `gfx` section.
     * @param bReload Whether the configuration was read before.
     * @param fScale The scale of the tunnel.
     * @ghidraAddress NTSC-U/C: 0x001ecf98
     * @ghidraAddress PAL: 0x001f5d38
     */
    void Load(DataArray *pConfig, DataArray *pDefaults, bool bReload, float fScale);

    unsigned char mReserved00[0x80]; // +0x00, not yet recovered.
};
