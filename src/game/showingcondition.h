#pragma once

#include "game/triggercondition.h"
#include "rnd/drawable.h"
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
     * Read the condition.
     *
     * @param pCondition The node.
     * @ghidraAddress NTSC-U/C: 0x00203b00
     * @ghidraAddress PAL: 0x0020c8b8
     */
    explicit ShowingCondition(DataArray *pCondition);

    bool Test() override {
        return mDrawable->GetShowing() != 0;
    }

private:
    Rnd::Drawable *mDrawable;
};
