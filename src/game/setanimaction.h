#pragma once

#include "game/triggeraction.h"
#include "rnd/animatable.h"
#include "script/dataarray.h"

/**
 * The `set_anim` action, which changes the first filter of an animatable.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x24 bytes. Node 1 names
 * the animatable. The optional `scale` and `offset` require a ScaleOffset first filter, and `min`
 * and `max` a MinMaxLoop one.
 */
class SetAnimAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * A first filter of the wrong kind produces a warning.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x002003a0
     * @ghidraAddress PAL: 0x00209140
     */
    explicit SetAnimAction(DataArray *pAction);

    /**
     * Write the values the node set into the first filter.
     *
     * A new scale without a new offset moves the offset to keep the output at the current frame.
     *
     * @ghidraAddress NTSC-U/C: 0x00202390
     * @ghidraAddress PAL: 0x0020b0c8
     */
    void Exec() override;

private:
    // The values the node set.
    enum Flag {
        kFlagScale = 1,
        kFlagOffset = 2,
        kFlagMin = 4,
        kFlagMax = 8,
    };

    // Warn unless the first filter of mAnim is of a kind.
    void CheckFirstFilter(DataArray *pAction, int nType, const char *pszType);

    int mFlags;
    Rnd::Animatable *mAnim;
    float mScale;
    float mOffset;
    float mMin;
    float mMax;
};
