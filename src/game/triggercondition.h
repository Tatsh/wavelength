#pragma once

#include "script/dataarray.h"

/**
 * A test a trigger makes before its actions start.
 *
 * The RTTI includes the name. Each kind of condition is a subclass, built by New() from one node
 * of a `conditions` entry. Most compare a value TheTriggerMgr recorded for the current event.
 */
class TriggerCondition {
public:
    /**
     * Build the condition a node describes.
     *
     * Node 0 names the kind, and the nodes after it are its parameters.
     *
     * @param pCondition The node.
     * @return The condition, or null for an unrecognised name.
     * @ghidraAddress NTSC-U/C: 0x00203428
     * @ghidraAddress PAL: 0x0020c1e0
     */
    static TriggerCondition *New(DataArray *pCondition);

    virtual ~TriggerCondition() {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds, before mNegate applies.
     */
    virtual bool Test() = 0;

    int mNegate; /*!< Non-zero when the trigger needs the condition not to hold. */
};
