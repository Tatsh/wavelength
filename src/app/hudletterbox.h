#pragma once

#include "app/rampanimator.h"
#include "rnd/view.h"

/**
 * Bars that close in over the top and the bottom of the screen.
 *
 * The class is not polymorphic and emits no RTTI. The object is 0x18 bytes.
 */
class HudLetterbox {
public:
    /**
     * Construct open bars from `HUD letterbox.view`.
     *
     * @param pHudView The view the bars are removed from.
     * @ghidraAddress NTSC-U/C: 0x001bf718
     * @ghidraAddress PAL: 0x001c84b8
     */
    explicit HudLetterbox(Rnd::View *pHudView);

    /**
     * Advance the bars, and show them while they are not open.
     *
     * @param fDelta The time since the last poll, or 0 to take it from fDeltaTicks.
     * @param fDeltaTicks The ticks since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001bf800
     * @ghidraAddress PAL: 0x001c85a0
     */
    void Poll(float fDelta, float fDeltaTicks);

    /**
     * Move the bars toward a part of the way closed.
     *
     * @param fClosed The part, from 0 (open) to 1 (closed).
     * @ghidraAddress NTSC-U/C: 0x001bf890
     * @ghidraAddress PAL: 0x001c8630
     */
    void SetClosed(float fClosed);

    Rnd::View *mView;    /*!< `HUD letterbox.view`. */
    RampAnimator mSlide; /*!< The bars, from 0 (open) to 100 (closed). */
};
