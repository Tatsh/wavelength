#pragma once

#include "game/triggercondition.h"
#include "rnd/rnddrawable.h"
#include "script/dataarray.h"

/**
 * The `showing` condition, which holds while a drawable shows.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1
 * names the drawable.
 */
class ShowingCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00375ab8
     * @ghidraAddress PAL: 0x003e41e8
     */
    ~ShowingCondition() override = default;

    /**
     * Read the condition.
     *
     * @param pCondition The node.
     * @ghidraAddress NTSC-U/C: 0x00203b00
     * @ghidraAddress PAL: 0x0020c8b8
     */
    explicit ShowingCondition(DataArray *pCondition);

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00375b38
     * @ghidraAddress PAL: 0x003e4268
     */
    bool Test() override {
        return mDrawable->GetShowing() != 0;
    }

private:
    RndDrawable *mDrawable;
};
