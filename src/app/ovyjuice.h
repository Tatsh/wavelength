#pragma once

#include "app/hideablepanel.h"
#include "math/interpolator.h"
#include "rnd/animatable.h"
#include "rnd/drawable.h"
#include "rnd/view.h"

/**
 * Energy bar of the solo head-up display.
 *
 * The RTTI records the class as deriving from HideablePanel. The object is 0x64 bytes. A drop in
 * energy leaves a ghost of the old level that fades out, the bar blinks faster as it empties, and
 * a warning blinks while the player is about to fail.
 */
class OvyJuice : public HideablePanel {
public:
    /**
     * Construct a hidden, full bar from `HUD1 energy.tnm` and `HUD energy.view`.
     *
     * @ghidraAddress NTSC-U/C: 0x001b9bf0
     * @ghidraAddress PAL: 0x001c2990
     */
    OvyJuice();

    /**
     * Destroy the bar.
     *
     * @ghidraAddress NTSC-U/C: 0x003671b0
     * @ghidraAddress PAL: 0x003d58e0
     */
    ~OvyJuice() override = default;

    /**
     * Slide the bar in or out. The bar shows only in a solo game outside the victory lap.
     *
     * @param bShow Whether to show the bar.
     * @ghidraAddress NTSC-U/C: 0x001b9ed8
     * @ghidraAddress PAL: 0x001c2c78
     */
    void Show(bool bShow) override;

    /**
     * Empty the bar, hide the warning and the ghost, and settle the display.
     *
     * @ghidraAddress NTSC-U/C: 0x001b9f30
     * @ghidraAddress PAL: 0x001c2cd0
     */
    void Reset();

    /**
     * Set the level of the bar, leaving a fading ghost when it drops.
     *
     * @param fLevel The level, clamped to 0 through 1.
     * @param nColor The colour of the bar, a frame of `HUD energy.mnm` in steps of 500.
     * @ghidraAddress NTSC-U/C: 0x001ba010
     * @ghidraAddress PAL: 0x001c2db0
     */
    void SetLevel(float fLevel, int nColor);

    /**
     * Start or stop the blinking warning.
     *
     * @param bWarn Whether the warning shows.
     * @ghidraAddress NTSC-U/C: 0x001ba0f0
     * @ghidraAddress PAL: 0x001c2e90
     */
    void SetWarning(bool bWarn);

    /**
     * Advance the slide, the warning, the ghost, and the blink of a low bar.
     *
     * @param fDelta The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001ba148
     * @ghidraAddress PAL: 0x001c2ee8
     */
    void Poll(float fDelta);

    /**
     * The time the ghost of a dropped level takes to fade, `juice_ghost_fade_time`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af91c
     */
    static float sGhostFadeTime;

    /**
     * The half period of the warning's blink, `juice_dying_blink_time`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af920
     */
    static float sWarningBlinkTime;

    /**
     * The blinks per unit of time of a low bar against its level, from `juice_blink_range` and
     * `juice_blink_frequency`. A level above the end of the range does not blink.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b110
     */
    static LinearInterpolator sBlinkRate;

    /**
     * The part of the current blink of a low bar still to run.
     *
     * @ghidraAddress NTSC-U/C: 0x003af94c
     */
    static float sBlinkLeft;

    Rnd::View *mView;                /*!< `HUD energy.view`, posed on the level. */
    Rnd::Animatable *mColorAnim;     /*!< `HUD energy.mnm`, posed on the colour. */
    Rnd::Drawable *mBar;             /*!< `HUD energy bar draw.view`. */
    Rnd::Drawable *mWarning;         /*!< `HUD ! bg.mesh`. */
    Rnd::Drawable *mGhost;           /*!< `HUD energy ghost.mesh`. */
    Rnd::Animatable *mGhostAnim;     /*!< `HUD energy ghost.msnm`, posed on the old level. */
    Rnd::Animatable *mGhostFadeAnim; /*!< `HUD energy ghost fade.mnm`. */
    LinearInterpolator mGhostFade;   /*!< The fade of the ghost against the song time. */
    float mLevel;                    /*!< The level, or -1 after Reset(). */
    int mWarn;                       /*!< Whether the warning blinks. */
    int mHideBar;                    /*!< Whether the bar is hidden. Reset() clears it. */
    float mWarningLeft;              /*!< The part of the warning's blink still to run. */
};
