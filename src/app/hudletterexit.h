#pragma once

#include "math/vector3.h"
#include "rnd/text.h"
#include "rnd/transanim.h"
#include "rnd/transformable.h"
#include "rnd/view.h"

/**
 * Banner of the letters of `AMPLITUDE` a duel player has won, which flies to the player's score.
 *
 * The class is not polymorphic and emits no RTTI. The object is 0xc0 bytes. A reveal that arrives
 * while the banner flies waits until the flight ends.
 */
class HudLetterExit {
public:
    /** The number of letters of `AMPLITUDE`. */
    static constexpr int kNumLetters = 9;

    /** The number of sides of a duel. */
    static constexpr int kNumSides = 2;

    /**
     * Construct an empty banner from `<hud> letter_exit.view`, the letters and the paths of each
     * side, and the flight path of each letter.
     *
     * @ghidraAddress NTSC-U/C: 0x001bedd8
     * @ghidraAddress PAL: 0x001c7b78
     */
    HudLetterExit();

    /**
     * Destroy the banner. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x001bf090
     * @ghidraAddress PAL: 0x001c7e30
     */
    ~HudLetterExit() {
    }

    /**
     * Hide the banner and stop its flight.
     *
     * @ghidraAddress NTSC-U/C: 0x001bf0b8
     * @ghidraAddress PAL: 0x001c7e58
     */
    void Reset();

    /**
     * Pulse the banner, advance its flight, and fade it.
     *
     * A flight that ends shows the reveal that waited for it.
     *
     * @ghidraAddress NTSC-U/C: 0x001bf130
     * @ghidraAddress PAL: 0x001c7ed0
     */
    void Poll();

    /**
     * Show the letter a player won at the centre of the banner.
     *
     * @param nPlayer The player.
     * @param nPoints The number of letters the player has, from 1.
     * @param pszText The text of the points. The banner does not read it.
     * @ghidraAddress NTSC-U/C: 0x001bf388
     * @ghidraAddress PAL: 0x001c8128
     */
    void ShowReveal(int nPlayer, int nPoints, const char *pszText);

    /**
     * Scale a transform by the pulse of the banner.
     *
     * The title is inferred.
     *
     * @param pTrans The transform.
     * @ghidraAddress NTSC-U/C: 0x001bf518
     * @ghidraAddress PAL: 0x001c82b8
     */
    void ScaleByPulse(Rnd::Transformable *pTrans);

    /**
     * Start a pulse that fades to a level.
     *
     * @param fLevel The level the pulse fades to.
     * @ghidraAddress NTSC-U/C: 0x001bf5a8
     * @ghidraAddress PAL: 0x001c8348
     */
    void SetTime(float fLevel);

    /**
     * Fly the letter of one side to its score.
     *
     * @param nResult One of GfxManager::PendingPointsResult. A lost letter flies from the second
     *                side.
     * @ghidraAddress NTSC-U/C: 0x001bf5c0
     * @ghidraAddress PAL: 0x001c8360
     */
    void Fly(int nResult);

    /**
     * The letters of `AMPLITUDE`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af950
     */
    static const char *sLetters;

    float mPulse;                               /*!< The pulse, which fades by 0.1 each poll. */
    float mPulseFloor;                          /*!< The level the pulse fades to. */
    int mRevealPending;                         /*!< Whether a reveal waits for the flight. */
    int mPendingPlayer;                         /*!< The player of the waiting reveal. */
    int mPendingPoints;                         /*!< The letters of the waiting reveal. */
    Rnd::View *mView;                           /*!< `<hud> letter_exit.view`. */
    Rnd::Text *mTexts[kNumSides];               /*!< `<hud> letter_exit<n>.txt` of each side. */
    Rnd::TransAnim *mAnims[kNumSides];          /*!< `<hud> letter_exit<n>.tnm` of each side. */
    int mLetters[kNumSides];                    /*!< The letter each side shows. */
    float mFlightLength;                        /*!< The last frame of the flight paths. */
    Vector3 mOrigin;                            /*!< The resting position of mView. */
    float mFlyBasis[3][Rnd::kXfmRowFloatCount]; /*!< The basis of the text that flies. */
    float mFlightStart;                         /*!< The tick the flight started, or -1e9. */
    Rnd::TransAnim *mFlight;                    /*!< The path of the flight, or null. */
    int mFlipped;                               /*!< Whether the flight mirrors its path. */
    Rnd::TransAnim *mLetterPaths[kNumLetters];  /*!< `<hud> letter_exit <letter>r.tnm`. */
    int mSecondSidePlayer;                      /*!< The player of the second side, or -1. */
};
