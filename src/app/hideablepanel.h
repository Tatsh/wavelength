#pragma once

#include "app/rampanimator.h"
#include "rnd/animatable.h"
#include "rnd/drawable.h"

/**
 * Part of the head-up display that slides in and out of view.
 *
 * The RTTI records the class in an anonymous namespace of the overlay's translation unit, with no
 * base. The object is 0x1c bytes. An animation runs from frame 0 (hidden) to frame 240 (shown), and
 * the drawable shows while the frame is not 0.
 */
class HideablePanel {
public:
    /** The frame of the animation at which the panel is fully shown. */
    static constexpr float kShownFrame = 240.0f;

    /**
     * Construct a panel and find its animation and drawable.
     *
     * @param pszAnim The animation that slides the panel, or null.
     * @param pszDrawable The drawable shown with the panel, or null.
     * @param bShown Whether the panel starts shown.
     * @ghidraAddress NTSC-U/C: 0x001b65e8
     * @ghidraAddress PAL: 0x001bf388
     */
    HideablePanel(const char *pszAnim, const char *pszDrawable, bool bShown);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00366d00
     * @ghidraAddress PAL: 0x003d53e8
     */
    virtual ~HideablePanel() = default;

    /**
     * Slide the panel in or out, or move it at once while sSnap is set.
     *
     * @param bShow Whether to show the panel.
     * @ghidraAddress NTSC-U/C: 0x001b6778
     * @ghidraAddress PAL: 0x001bf518
     */
    virtual void Show(bool bShow);

    /**
     * Show or hide the panel at once.
     *
     * @param bShow Whether to show the panel.
     * @ghidraAddress NTSC-U/C: 0x001b67d8
     * @ghidraAddress PAL: 0x001bf578
     */
    virtual void ShowNow(bool bShow);

    /**
     * Find the animation and the drawable of the panel, and show or hide it at once.
     *
     * @param pszAnim The animation that slides the panel, or null to retain the current one.
     * @param pszDrawable The drawable shown with the panel, or null to retain the current one.
     * @param bShown Whether the panel is shown.
     * @ghidraAddress NTSC-U/C: 0x001b6670
     * @ghidraAddress PAL: 0x001bf410
     */
    void SetObjects(const char *pszAnim, const char *pszDrawable, bool bShown);

    /**
     * Advance the slide by sDeltaTime, and show the drawable while the panel is not hidden.
     *
     * @ghidraAddress NTSC-U/C: 0x001b6808
     * @ghidraAddress PAL: 0x001bf5a8
     */
    void Poll();

    /**
     * Clear sSnap.
     *
     * @ghidraAddress NTSC-U/C: 0x001b65c8
     * @ghidraAddress PAL: 0x001bf368
     */
    static void ClearSnap();

    /**
     * Set the time the next Poll() of every panel advances by.
     *
     * @param fDeltaTime The time.
     * @ghidraAddress NTSC-U/C: 0x001b65d8
     * @ghidraAddress PAL: 0x001bf378
     */
    static void SetDeltaTime(float fDeltaTime);

    /**
     * Find the animation of the two-dimensional materials and the frame it rests at.
     *
     * The frame is `enter_stop_frame` of the `panel_enter_exit` entry of the `ui` section of the
     * configuration, when the section exists. Clears sSnap.
     *
     * @ghidraAddress NTSC-U/C: 0x001b6878
     * @ghidraAddress PAL: 0x001bf618
     */
    static void Init();

    /**
     * Forget the animation of the two-dimensional materials.
     *
     * @ghidraAddress NTSC-U/C: 0x001b6940
     * @ghidraAddress PAL: 0x001bf6e0
     */
    static void Terminate();

    /**
     * Pose the animation of the two-dimensional materials at its resting frame.
     *
     * @ghidraAddress NTSC-U/C: 0x001b6950
     * @ghidraAddress PAL: 0x001bf6f0
     */
    static void PoseMaterials();

    /**
     * Whether Show() moves a panel at once rather than sliding it.
     *
     * @ghidraAddress NTSC-U/C: 0x003af908
     */
    static int sSnap;

    /**
     * The time Poll() advances every panel by.
     *
     * @ghidraAddress NTSC-U/C: 0x003af90c
     */
    static float sDeltaTime;

    /**
     * The animation of the two-dimensional materials, `mat_2d_EE.anim`, or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003af910
     */
    static Rnd::Animatable *sMaterialAnim;

    /**
     * The frame PoseMaterials() poses sMaterialAnim at.
     *
     * @ghidraAddress NTSC-U/C: 0x003af914
     */
    static float sMaterialFrame;

    RampAnimator mSlide;      /*!< The slide, from 0 (hidden) to kShownFrame (shown). */
    Rnd::Drawable *mDrawable; /*!< The drawable shown with the panel, or null. */
};
