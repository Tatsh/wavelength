#pragma once

#include <list>

#include "game/triggeraction.h"
#include "game/triggercondition.h"
#include "script/dataarray.h"

/**
 * One trigger of a trigger file, the conditions that must hold and the actions that follow.
 *
 * The RTTI includes the name through the list of handlers TriggerEvent keeps. The object is 0x10
 * bytes. Each `conditions` entry of the trigger adds one group of conditions. The actions start
 * when every condition of any one group holds, or at once when the trigger has no group.
 */
class TriggerHandler {
public:
    /**
     * Build the trigger from its entry.
     *
     * Every `conditions` node adds a group. The `actions` node is required.
     *
     * @param pTrigger The entry.
     * @ghidraAddress NTSC-U/C: 0x001ffb00
     * @ghidraAddress PAL: 0x002088a0
     */
    explicit TriggerHandler(DataArray *pTrigger);

    /**
     * Delete every condition and every action.
     *
     * @ghidraAddress NTSC-U/C: 0x001ffc50
     * @ghidraAddress PAL: 0x002089f0
     */
    ~TriggerHandler();

    /**
     * Test the groups of conditions and start the actions when one group holds.
     *
     * @ghidraAddress NTSC-U/C: 0x001ffdb0
     * @ghidraAddress PAL: 0x00208b50
     */
    void Fire();

private:
    /**
     * Read the actions node.
     *
     * A number before an action sets its delay, and `realtime` makes it and every later action
     * run on the real-time clock.
     *
     * @param pActions The node.
     * @ghidraAddress NTSC-U/C: 0x001ff650
     * @ghidraAddress PAL: 0x002083f0
     */
    void AddActions(DataArray *pActions);

    /**
     * Read a conditions node into a new group.
     *
     * A symbol that starts with `!` negates the condition after it.
     *
     * @param pConditions The node.
     * @ghidraAddress NTSC-U/C: 0x001ff808
     * @ghidraAddress PAL: 0x002085a8
     */
    void AddConditions(DataArray *pConditions);

    std::list<std::list<TriggerCondition *>> mConditions;
    std::list<TriggerAction *> mActions;
};
