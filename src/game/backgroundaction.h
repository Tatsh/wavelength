#pragma once

#include "game/triggeraction.h"
#include "math/color.h"
#include "script/dataarray.h"

/**
 * The `background` action, which fades the clear colour of the renderer to a new colour.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x40 bytes. Nodes 1
 * through 3 are the red, green, and blue of the colour, and node 4 the length of the fade.
 */
class BackgroundAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * The alpha of the colour is never written.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00200948
     * @ghidraAddress PAL: 0x002096e8
     */
    explicit BackgroundAction(DataArray *pAction);

    /**
     * Start the fade from the current clear colour.
     *
     * @ghidraAddress NTSC-U/C: 0x002025b8
     * @ghidraAddress PAL: 0x0020b358
     */
    void Exec() override;

    /**
     * Set the clear colour for the time on the clock.
     *
     * @return Whether the fade is over.
     * @ghidraAddress NTSC-U/C: 0x00203188
     * @ghidraAddress PAL: 0x0020bf40
     */
    bool Poll() override;

private:
    alignas(16) Color mColor;
    alignas(16) Color mStartColor;
    float mDuration;
    float mStartTime;
};
