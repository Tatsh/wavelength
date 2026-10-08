#pragma once

#include "game/triggeraction.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * The `change_screen` action, which moves the front end to a screen.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x10 bytes. Node 1 names
 * the screen.
 */
class ChangeScreenAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x002021b8
     * @ghidraAddress PAL: 0x0020af58
     */
    explicit ChangeScreenAction(DataArray *pAction);

    /**
     * Move TheUI to the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00202e70
     * @ghidraAddress PAL: 0x0020bc10
     */
    void Exec() override;

private:
    UIScreen *mScreen;
};
