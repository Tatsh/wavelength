#pragma once

#include "game/triggeraction.h"
#include "script/dataarray.h"

/**
 * The `avatar_body_animate` action, which sets the animation of the avatar of the current player.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x14 bytes. Node 1 is
 * the animation, and the optional node 2 the flag AvatarPartSet::SetBaseAnim() receives.
 */
class AvatarBodyAnimateAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x002008d0
     * @ghidraAddress PAL: 0x00209670
     */
    explicit AvatarBodyAnimateAction(DataArray *pAction);

    /**
     * Give the animation to the avatar of TriggerMgr::mPlayer.
     *
     * @ghidraAddress NTSC-U/C: 0x00202570
     * @ghidraAddress PAL: 0x0020b310
     */
    void Exec() override;

private:
    const char *mAnim;
    int mFlags;
};
