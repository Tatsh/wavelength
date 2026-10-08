#pragma once

#include "game/triggeraction.h"
#include "script/dataarray.h"

/**
 * The `blur` action, which switches on the screen blur of the renderer.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x2c bytes. Node 1 is
 * the amount, node 2 the count, node 3 an array of the four values of the rectangle, and node 4 the
 * time the blur lasts. A time of 0 leaves the blur on.
 */
class BlurAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x002009c8
     * @ghidraAddress PAL: 0x00209768
     */
    explicit BlurAction(DataArray *pAction);

    /**
     * Copy the parameters into ThePs and switch the blur on.
     *
     * @ghidraAddress NTSC-U/C: 0x00202608
     * @ghidraAddress PAL: 0x0020b3a8
     */
    void Exec() override;

    /**
     * Switch the blur off once its time has passed.
     *
     * @return Whether the time has passed.
     * @ghidraAddress NTSC-U/C: 0x00203288
     * @ghidraAddress PAL: 0x0020c040
     */
    bool Poll() override;

private:
    // The number of values in the rectangle.
    static constexpr int kRectSize = 4;

    float mAmount;
    int mCount;
    float mRect[kRectSize];
    float mDuration;
    float mEndTime;
};
