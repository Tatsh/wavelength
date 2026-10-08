#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `note` condition, which holds when the last beat was a note.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the note, a letter from A through G.
 */
class NoteCondition : public TriggerCondition {
public:
    /**
     * Read the condition.
     *
     * A note outside A through G produces a warning.
     *
     * @param pCondition The node.
     * @ghidraAddress NTSC-U/C: 0x00203a80
     * @ghidraAddress PAL: 0x0020c838
     */
    explicit NoteCondition(DataArray *pCondition);

    bool Test() override {
        return mNote == TheTriggerMgr.mNote;
    }

private:
    char mNote;
};
