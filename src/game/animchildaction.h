#pragma once

#include "game/triggeraction.h"
#include "rnd/animatable.h"
#include "script/dataarray.h"

/**
 * The `add_anim` and `remove_anim` actions, which attach an animatable to another or detach it.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x18 bytes. Node 1 names
 * the child and node 2 the parent.
 */
class AnimChildAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * One warning reports the first of the two names that is missing.
     *
     * @param pAction The node.
     * @param bAdd True for `add_anim`, false for `remove_anim`.
     * @ghidraAddress NTSC-U/C: 0x00201f68
     * @ghidraAddress PAL: 0x0020ad08
     */
    AnimChildAction(DataArray *pAction, bool bAdd);

    /**
     * Add the child to the parent, or remove it.
     *
     * @ghidraAddress NTSC-U/C: 0x00202e08
     * @ghidraAddress PAL: 0x0020bba8
     */
    void Exec() override;

private:
    int mAdd;
    Rnd::Animatable *mChild;
    Rnd::Animatable *mParent;
};
