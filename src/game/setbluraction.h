#pragma once

#include "game/triggeraction.h"
#include "rnd/blur.h"
#include "script/dataarray.h"

/**
 * The `set_blur` action, which changes a motion blur object.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x20 bytes. Node 1 names
 * the blur. The optional values are `length`, `rate`, and `falloff`.
 */
class SetBlurAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x002014d8
     * @ghidraAddress PAL: 0x0020a278
     */
    explicit SetBlurAction(DataArray *pAction);

    /**
     * Write the values the node set into the blur.
     *
     * @ghidraAddress NTSC-U/C: 0x00202960
     * @ghidraAddress PAL: 0x0020b700
     */
    void Exec() override;

private:
    // The values the node set.
    enum Flag {
        kFlagLength = 1,
        kFlagRate = 2,
        kFlagFalloff = 4,
    };

    int mFlags;
    Rnd::Blur *mBlur;
    int mLength;
    int mRate;
    float mFalloff;
};
