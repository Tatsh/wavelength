#pragma once

#include "game/triggeraction.h"
#include "math/color.h"
#include "rnd/environ.h"
#include "script/dataarray.h"

/**
 * The `set_env` action, which changes the fog of an environment.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x30 bytes. Node 1 names
 * the environment. The optional values are `fog_start`, `fog_end`, `fog_density`, and
 * `fog_color`.
 */
class SetEnvAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00200c10
     * @ghidraAddress PAL: 0x002099b0
     */
    explicit SetEnvAction(DataArray *pAction);

    /**
     * Write the values the node set into the environment.
     *
     * @ghidraAddress NTSC-U/C: 0x002026d8
     * @ghidraAddress PAL: 0x0020b448
     */
    void Exec() override;

private:
    // The values the node set.
    enum Flag {
        kFlagFogStart = 1,
        kFlagFogEnd = 2,
        kFlagFogDensity = 4,
        kFlagFogColor = 8,
    };

    int mFlags;
    Rnd::Environ *mEnviron;
    float mFogStart;
    float mFogEnd;
    float mFogDensity;
    alignas(16) Color mFogColor;
};
