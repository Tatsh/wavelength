#pragma once

#include "game/triggeraction.h"
#include "rnd/cam.h"
#include "script/dataarray.h"

/**
 * The `set_cam` action, which changes the projection of a camera.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x20 bytes. Node 1 names
 * the camera. The optional values are `near`, `far`, and `fov` in degrees.
 */
class SetCamAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00200d90
     * @ghidraAddress PAL: 0x00209b30
     */
    explicit SetCamAction(DataArray *pAction);

    /**
     * Set the projection, keeping the values the node did not set.
     *
     * @ghidraAddress NTSC-U/C: 0x00202758
     * @ghidraAddress PAL: 0x0020b4f8
     */
    void Exec() override;

private:
    // The values the node set.
    enum Flag {
        kFlagNear = 1,
        kFlagFar = 2,
        kFlagFov = 4,
    };

    int mFlags;
    Rnd::Cam *mCam;
    float mNear;
    float mFar;
    float mFov; // In radians.
};
