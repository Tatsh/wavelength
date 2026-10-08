#pragma once

#include "app/rampanimator.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "rnd/view.h"

/**
 * Icon of the power-up a player holds, with its name.
 *
 * The class is not polymorphic and emits no RTTI. The object is 0x40 bytes. The icon slides out
 * of sight while the player holds no power-up. In the tutorial the icon does not slide.
 */
class HudPowerup {
public:
    /** The icons, the indices of mIcons. */
    enum Icon {
        kIconAutocatcher = 0, /*!< `<hud> pup auto<n>.view`. */
        kIconSlowdown = 1,    /*!< `<hud> pup slow<n>.view`. */
        kIconBumper = 2,      /*!< `<hud> pup bump<n>.view`. */
        kIconCrippler = 3,    /*!< `<hud> pup crip<n>.view`. */
        kIconFreestyle = 4,   /*!< `<hud> pup free<n>.view`. */
        kIconMultiplier = 5,  /*!< `<hud> pup mult<n>.view`. */
        kNumIcons = 6,        /*!< The number of icons. */
    };

    /** The frame of the slide at which the icon is out of sight. */
    static constexpr float kHiddenFrame = 480.0f;

    /**
     * Construct an empty icon of a player.
     *
     * The scene objects are `<hud> pup<n>.mesh`, `<hud> pup<n>.view`, `<hud> pup<n> name.txt`,
     * the six icon views, and the slide `<hud> pup<n> hide.tnm`. An online game uses the `HUD1`
     * objects and draws the icon into the letterboxed view of its player.
     *
     * @param nIndex The number of the icon, from 0.
     * @param pHudView The view of the head-up display.
     * @ghidraAddress NTSC-U/C: 0x001bc730
     * @ghidraAddress PAL: 0x001c54d0
     */
    HudPowerup(int nIndex, Rnd::View *pHudView);

    /**
     * Empty the icon and move it out of sight at once.
     *
     * @ghidraAddress NTSC-U/C: 0x001bcd68
     * @ghidraAddress PAL: 0x001c5b08
     */
    void Reset();

    /**
     * Show the icon and the name of a power-up, or empty the icon.
     *
     * @param nPowerup The power-up, one of GameLogic::Powerup.
     * @ghidraAddress NTSC-U/C: 0x001bcdb8
     * @ghidraAddress PAL: 0x001c5b58
     */
    void ShowPowerup(int nPowerup);

    /**
     * Advance the slide, and show the icon while it is not out of sight.
     *
     * @param fDelta The time since the last poll, or 0 to take it from fDeltaTicks.
     * @param fUnused Not read.
     * @param fDeltaTicks The ticks since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001bcfb0
     * @ghidraAddress PAL: 0x001c5d50
     */
    void Poll(float fDelta, float fUnused, float fDeltaTicks);

    /**
     * Slide the icon into view when a power-up is held and the icon is enabled, and out of sight
     * otherwise.
     *
     * @ghidraAddress NTSC-U/C: 0x001bd038
     * @ghidraAddress PAL: 0x001c5dd8
     */
    void UpdateSlide();

    Rnd::View *mIcons[kNumIcons]; /*!< The icon of each power-up, indexed by Icon. */
    Rnd::View *mView;             /*!< `<hud> pup<n>.view`, which draws the icon shown. */
    Rnd::Text *mName;             /*!< `<hud> pup<n> name.txt`. */
    Rnd::Mesh *mMesh;             /*!< `<hud> pup<n>.mesh`. */
    RampAnimator mSlide;          /*!< The slide, from 0 (in view) to kHiddenFrame. */
    int mHolding;                 /*!< Whether a power-up is held. */
    int mEnabled;                 /*!< Whether the icon may slide into view. */
};
