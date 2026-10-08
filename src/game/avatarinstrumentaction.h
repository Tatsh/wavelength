#pragma once

#include "game/triggeraction.h"
#include "script/dataarray.h"

/**
 * The `avatar_instrument` action, which sets the pose of the avatar of the current player.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x10 bytes. Node 1 is
 * the pose animation.
 */
class AvatarInstrumentAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00200890
     * @ghidraAddress PAL: 0x00209630
     */
    explicit AvatarInstrumentAction(DataArray *pAction);

    /**
     * Give the pose to the avatar of TriggerMgr::mPlayer.
     *
     * @ghidraAddress NTSC-U/C: 0x00202528
     * @ghidraAddress PAL: 0x0020b2c8
     */
    void Exec() override;

private:
    const char *mPose;
};
