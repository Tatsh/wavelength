#pragma once

#include "app/hideablepanel.h"
#include "rnd/animatable.h"
#include "rnd/font.h"
#include "rnd/text.h"
#include "rnd/view.h"

/**
 * Score of a duel player: the letters of `AMPLITUDE` the player has lit.
 *
 * The RTTI records the class as deriving from HideablePanel. The object is 0x58 bytes. Letters lit
 * by a change show in the highlight font for 960 milliseconds once the points arrive.
 */
class OvyDuelScore : public HideablePanel {
public:
    /** The number of letters. */
    static constexpr int kNumLetters = 9;

    /** The fonts of the letters, the indices of mFonts. */
    enum LetterFont {
        kLetterFontNormal = 0, /*!< `HUDd letter no.font`. */
        kLetterFontNew = 1,    /*!< `HUDd letter hi <n>.font`, for a letter lit by the change. */
        kNumLetterFonts = 2,   /*!< The number of fonts. */
    };

    /**
     * Construct a hidden score of no letters from `HUDd letters <n>.view`, its slide
     * `HUDd letters <n>.tnm`, and the letters `HUDd letter<i> <n>.txt`.
     *
     * @param nIndex The number of the score, from 0.
     * @ghidraAddress NTSC-U/C: 0x001bde48
     * @ghidraAddress PAL: 0x001c6be8
     */
    explicit OvyDuelScore(int nIndex);

    /**
     * Destroy the score.
     *
     * @ghidraAddress NTSC-U/C: 0x003673b8
     * @ghidraAddress PAL: 0x003d5ae8
     */
    ~OvyDuelScore() override = default;

    /**
     * Light a number of letters.
     *
     * @param nCount The number of lit letters.
     * @ghidraAddress NTSC-U/C: 0x001be108
     * @ghidraAddress PAL: 0x001c6ea8
     */
    void SetLitCount(int nCount);

    /**
     * Advance the slide and the flash of newly lit letters.
     *
     * @ghidraAddress NTSC-U/C: 0x001be140
     * @ghidraAddress PAL: 0x001c6ee0
     */
    void Poll();

    /**
     * Darken every letter and show the score.
     *
     * @ghidraAddress NTSC-U/C: 0x001be230
     * @ghidraAddress PAL: 0x001c6fd0
     */
    void Reset();

    /**
     * Show a number of letters, and choose their fonts.
     *
     * @param nCount The number of letters to show.
     * @param bSetFonts Whether to choose the fonts and start the flash of new letters.
     * @ghidraAddress NTSC-U/C: 0x001be278
     * @ghidraAddress PAL: 0x001c7018
     */
    void ShowLetters(int nCount, bool bSetFonts);

    int mLitCount;                      /*!< The number of lit letters. */
    Rnd::View *mView;                   /*!< `HUDd letters <n>.view`. */
    Rnd::Text *mLetters[kNumLetters];   /*!< The letters. */
    float mFlashTime;                   /*!< The song time the flash starts at, or a sentinel. */
    Rnd::Animatable *mFlashAnim;        /*!< `HUDd letter hi <n>.mnm`. */
    Rnd::Font *mFonts[kNumLetterFonts]; /*!< The fonts, indexed by LetterFont. */
};
