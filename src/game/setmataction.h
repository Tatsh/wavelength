#pragma once

#include "game/triggeraction.h"
#include "math/color.h"
#include "rnd/mat.h"
#include "rnd/tex.h"
#include "script/dataarray.h"

/**
 * The `set_mat` action, which changes the colours, the alpha, and the stages of a material.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x70 bytes. Node 1 names
 * the material. The optional values are `base_color`, `light_color`, `edge_color`, `alpha`, the
 * blend of each of the first three stages (`blend1` through `blend3`), and the texture of each
 * (`tex1` through `tex3`).
 */
class SetMatAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * A stage value for a stage the material does not have produces a warning.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00200f08
     * @ghidraAddress PAL: 0x00209ca8
     */
    explicit SetMatAction(DataArray *pAction);

    /**
     * Write the values the node set into the material.
     *
     * @ghidraAddress NTSC-U/C: 0x002027d0
     * @ghidraAddress PAL: 0x0020b570
     */
    void Exec() override;

private:
    // The values the node set.
    enum Flag {
        kFlagBaseColor = 0x1,
        kFlagLightColor = 0x2,
        kFlagEdgeColor = 0x4,
        kFlagAlpha = 0x20,
        kFlagBlend1 = 0x40,
        kFlagBlend2 = 0x80,
        kFlagBlend3 = 0x100,
        kFlagTex1 = 0x200,
        kFlagTex2 = 0x400,
        kFlagTex3 = 0x800,
    };

    // The number of stages the action can change.
    static constexpr int kNumStages = 3;

    // Warn unless the material has a stage, counted from 1.
    void CheckStage(DataArray *pAction, int nStage);

    int mFlags;
    Rnd::Mat *mMat;
    alignas(16) Color mBaseColor;
    alignas(16) Color mLightColor;
    alignas(16) Color mEdgeColor;
    float mAlpha;
    int mBlend[kNumStages];
    Rnd::Tex *mTex[kNumStages];
};
