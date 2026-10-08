#pragma once

#include "app/hideablepanel.h"
#include "rnd/animatable.h"
#include "rnd/drawable.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/view.h"

/**
 * Picture of the controller on the head-up display, with the buttons the tutorial points at.
 *
 * The RTTI records the class as deriving from HideablePanel. The object is 0xb0 bytes.
 */
class OvyController : public HideablePanel {
public:
    /** The number of buttons of the picture, one per JoypadButton from kPadL2 to kPadDLeft. */
    static constexpr int kNumButtons = 16;

    /** One button of the picture. */
    struct Button {
        /**
         * Find the animation and the mesh of a button.
         *
         * @param pszName The name of the button, such as `triangle`.
         * @ghidraAddress NTSC-U/C: 0x001c00a0
         * @ghidraAddress PAL: 0x001c8e40
         */
        void Load(const char *pszName);

        Rnd::Animatable *mAnim; /*!< `HUD controller map <name>.tnm`. */
        Rnd::Drawable *mMesh;   /*!< `HUD controller map <name>.mesh`. */
    };

    /** The materials of the picture, the indices of mMats. */
    enum Highlight {
        kHighlightOff = 0,  /*!< `HUD controller map no.mat`. */
        kHighlightOn = 1,   /*!< `HUD controller map hi.mat`. */
        kNumHighlights = 2, /*!< The number of materials. */
    };

    /**
     * Construct a hidden picture and draw it into the letterboxed view of the head-up display.
     *
     * @param pHudView The view of the head-up display.
     * @ghidraAddress NTSC-U/C: 0x001c01a0
     * @ghidraAddress PAL: 0x001c8f40
     */
    explicit OvyController(Rnd::View *pHudView);

    /**
     * Destroy the picture.
     *
     * @ghidraAddress NTSC-U/C: 0x00367488
     * @ghidraAddress PAL: 0x003d5bb8
     */
    ~OvyController() override = default;

    /**
     * Hide the picture and every button at once.
     *
     * @ghidraAddress NTSC-U/C: 0x001c05d8
     * @ghidraAddress PAL: 0x001c9378
     */
    void Reset();

    /**
     * Light or darken the picture.
     *
     * @param bHighlight Whether to light the picture.
     * @ghidraAddress NTSC-U/C: 0x001c0678
     * @ghidraAddress PAL: 0x001c9418
     */
    void SetHighlight(bool bHighlight);

    /**
     * Show or hide a button, and start or stop its animation.
     *
     * @param nButton The button, one of JoypadButton.
     * @param bShow Whether to show the button.
     * @ghidraAddress NTSC-U/C: 0x001c06a8
     * @ghidraAddress PAL: 0x001c9448
     */
    void ShowButton(int nButton, bool bShow);

    Button mButtons[kNumButtons];    /*!< The buttons, indexed by JoypadButton. */
    Rnd::View *mView;                /*!< `HUD controller map.view`. */
    Rnd::Mat *mMats[kNumHighlights]; /*!< The materials, indexed by Highlight. */
    Rnd::Mesh *mMesh;                /*!< `HUD controller map.mesh`. */
    Rnd::Animatable *mPosAnim;       /*!< `HUD controller map pos.tnm`. */
};
