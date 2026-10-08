#pragma once

#include "game/triggeraction.h"
#include "rnd/drawable.h"
#include "script/dataarray.h"

/**
 * The `show` and `hide` actions, which show or hide a drawable.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x14 bytes. Node 1 names
 * the drawable.
 */
class ShowAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @param bShow True for `show`, false for `hide`.
     * @ghidraAddress NTSC-U/C: 0x002020d0
     * @ghidraAddress PAL: 0x0020ae70
     */
    ShowAction(DataArray *pAction, bool bShow);

    /**
     * Show or hide the drawable.
     *
     * @ghidraAddress NTSC-U/C: 0x00202e40
     * @ghidraAddress PAL: 0x0020bbe0
     */
    void Exec() override;

private:
    int mShow;
    Rnd::Drawable *mDrawable;
};
