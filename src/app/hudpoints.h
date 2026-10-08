#pragma once

#include "app/rampanimator.h"
#include "rnd/blur.h"
#include "rnd/mat.h"
#include "rnd/text.h"
#include "rnd/transformable.h"
#include "rnd/view.h"

/**
 * Points a player is building on the current phrase, and the points that fly off when the phrase
 * is captured or lost.
 *
 * The class is not polymorphic and emits no RTTI. The object is 0x58 bytes. The title follows the
 * points display of the previous game, which shares the scene objects.
 */
class HudPoints {
public:
    /** The frame of the exit animation at which it rests. */
    static constexpr float kExitRestFrame = 100.0f;

    /**
     * Construct an empty display of a player.
     *
     * The scene objects are `<hud> pts_exit<n>.blur`, `<hud> pts_exit<n>.view`,
     * `<hud> pts_exit<n>.txt`, `<hud> pts<n>.txt`, and `<hud> ptsmult<n>.txt`. An online game uses
     * the `HUDn` objects of number 0.
     *
     * @param nIndex The number of the display, from 0.
     * @ghidraAddress NTSC-U/C: 0x001be398
     * @ghidraAddress PAL: 0x001c7138
     */
    explicit HudPoints(int nIndex);

    /**
     * Destroy the display.
     *
     * @ghidraAddress NTSC-U/C: 0x001be840
     * @ghidraAddress PAL: 0x001c75e0
     */
    ~HudPoints() = default;

    /**
     * Empty the display.
     *
     * @ghidraAddress NTSC-U/C: 0x001be770
     * @ghidraAddress PAL: 0x001c7510
     */
    void Reset();

    /**
     * Enable or disable the display, and empty it.
     *
     * @param bEnabled Whether the display is enabled.
     * @ghidraAddress NTSC-U/C: 0x001be820
     * @ghidraAddress PAL: 0x001c75c0
     */
    void SetEnabled(bool bEnabled);

    /**
     * Advance the flight of the exiting points and the glow of the pending points.
     *
     * @param fUnused Not read.
     * @param fDelta The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001be890
     * @ghidraAddress PAL: 0x001c7630
     */
    void Poll(float fUnused, float fDelta);

    /**
     * Scale a text by the size of the points and the swell of the glow.
     *
     * @param pText The text.
     * @ghidraAddress NTSC-U/C: 0x001be9f8
     * @ghidraAddress PAL: 0x001c7798
     */
    void ScaleText(Rnd::Transformable *pText);

    /**
     * Show the pending points.
     *
     * @param nPoints The points.
     * @param pszText The text of the points.
     * @ghidraAddress NTSC-U/C: 0x001bea88
     * @ghidraAddress PAL: 0x001c7828
     */
    void ShowPoints(int nPoints, const char *pszText);

    /**
     * Show the multiplier of the pending points, or hide it for an empty text.
     *
     * @param nMultiplier The multiplier.
     * @param bHot Whether the points take the hot colour.
     * @param pszText The text of the multiplier.
     * @ghidraAddress NTSC-U/C: 0x001beaf8
     * @ghidraAddress PAL: 0x001c7898
     */
    void SetMultiplier(int nMultiplier, int bHot, const char *pszText);

    /**
     * Hide the multiplier.
     *
     * @ghidraAddress NTSC-U/C: 0x001beb90
     * @ghidraAddress PAL: 0x001c7930
     */
    void HideMultiplier();

    /**
     * Flash the pending points, swelling them from a minimum size.
     *
     * @param fMinSwell The swell the points shrink back to.
     * @ghidraAddress NTSC-U/C: 0x001bebc0
     * @ghidraAddress PAL: 0x001c7960
     */
    void Flash(float fMinSwell);

    /**
     * End the pending points: fly off the points multiplied, fly off the plain points, or clear
     * them.
     *
     * A lost phrase of a duel flies off like a captured one, 200 frames later.
     *
     * @param nResult One of GfxManager::PendingPointsResult.
     * @ghidraAddress NTSC-U/C: 0x001bebe8
     * @ghidraAddress PAL: 0x001c7988
     */
    void End(int nResult);

    float mGlow;            /*!< The glow of the pending points, from 1 down to 0. */
    float mSwell;           /*!< The swell of the pending points. */
    float mMinSwell;        /*!< The swell the points shrink back to. */
    int mMultiplier;        /*!< The multiplier of the pending points. */
    int mPoints;            /*!< The pending points. */
    int mPending;           /*!< Whether points are pending. */
    int mHot;               /*!< Whether the points take the hot colour. */
    int mEnabled;           /*!< Whether the display is enabled. */
    Rnd::View *mExitView;   /*!< `<hud> pts_exit<n>.view`, the flight of the exiting points. */
    Rnd::Text *mExitText;   /*!< `<hud> pts_exit<n>.txt`. */
    Rnd::Blur *mExitBlur;   /*!< `<hud> pts_exit<n>.blur`, or null. */
    Rnd::Text *mPointsText; /*!< `<hud> pts<n>.txt`. */
    Rnd::Text *mMultText;   /*!< `<hud> ptsmult<n>.txt`. */
    Rnd::Mat *mPointsMat;   /*!< The material of the points' font. */
    Rnd::Mat *mMultMat;     /*!< The material of the multiplier's font. */
    Rnd::Mat *mColdMat;     /*!< `HUD ptstmp.mat`, the colour of plain points. */
    Rnd::Mat *mHotMat;      /*!< `HUD ptstmphot.mat`, the colour of hot points. */
    RampAnimator mExit;     /*!< The flight of the exiting points. */
};
