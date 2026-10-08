#pragma once

#include "game/triggeraction.h"
#include "script/dataarray.h"

/**
 * The `avatar_gem_catch` action, which plays a gem catch on the avatar of the current player.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x14 bytes. Node 1 is
 * the flag the avatar receives. The optional node 2 is the lane, and without it the lane of the
 * event applies.
 */
class AvatarGemCatchAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00200818
     * @ghidraAddress PAL: 0x002095b8
     */
    explicit AvatarGemCatchAction(DataArray *pAction);

    /**
     * Pass the lane and the flag to the avatar of TriggerMgr::mPlayer.
     *
     * @ghidraAddress NTSC-U/C: 0x002024b8
     * @ghidraAddress PAL: 0x0020b258
     */
    void Exec() override;

private:
    // The mLane value that selects the lane of the event.
    static constexpr int kEventLane = 5;

    int mCatch;
    int mLane;
};
