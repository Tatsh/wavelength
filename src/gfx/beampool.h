#pragma once

#include "gfx/beam.h"
#include "gfx/beamparams.h"
#include "gfx/fxpool.h"
#include "rnd/drawable.h"
#include "script/dataarray.h"

/**
 * Pool of the beams of one family, which share a look and draw under one parent.
 *
 * The RTTI names the template. The object is 0x90 bytes.
 *
 * @tparam bArg The second template argument of FxPool. Its meaning is not yet recovered.
 */
template <bool bArg>
class BeamPool : public FxPool<Beam, bArg> {
public:
    /**
     * Construct a pool and draw every beam under a parent.
     *
     * @param nCount The number of beams.
     * @param pParent The drawable the beams draw under, or null.
     * @param pszName The configuration entry of the look of the beams.
     * @ghidraAddress NTSC-U/C: 0x00367f58
     * @ghidraAddress PAL: 0x003d6688
     */
    BeamPool(int nCount, Rnd::Drawable *pParent, const char *pszName)
        : FxPool<Beam, bArg>(nCount), mParams(pszName) {
        for (Beam *pBeam = this->mItems; pBeam != this->mItems + this->mCount; ++pBeam) {
            pParent->AddDraw(pBeam->mLine, nullptr);
        }
    }

    /**
     * Destroy the pool.
     *
     * @ghidraAddress NTSC-U/C: 0x00368000
     * @ghidraAddress PAL: 0x003d6730
     */
    ~BeamPool() override = default;

    /**
     * Read the look of the beams and give it to every beam.
     *
     * @param pConfig The section of the kind of game.
     * @param pDefaults The `gfx` section.
     * @param bReload Whether the configuration was read before.
     * @param fScale The scale of the tunnel.
     * @ghidraAddress NTSC-U/C: 0x00367cb8
     * @ghidraAddress PAL: 0x003d63e8
     */
    void LoadConfig(DataArray *pConfig, DataArray *pDefaults, bool bReload, float fScale) {
        mParams.Load(pConfig, pDefaults, bReload, fScale);
        for (Beam *pBeam = this->mItems; pBeam != this->mItems + this->mCount; ++pBeam) {
            pBeam->Configure(&mParams);
        }
    }

    BeamParams mParams; /*!< The look of the beams. */
};
