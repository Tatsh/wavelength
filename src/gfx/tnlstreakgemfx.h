#pragma once

#include "gfx/gfxtunnel.h"
#include "math/interpolator.h"
#include "math/transform.h"
#include "rnd/mat.h"
#include "rnd/text.h"
#include "rnd/view.h"

/**
 * The arrow that marks the next gem of a streak on a track, with a text, which blinks out of its
 * old place and into its new one.
 *
 * The RTTI identifies the class, which is not polymorphic. The object is 0x74 bytes. The arrows of
 * all tracks share one text. A new text waits in sPendingText while an arrow is shown.
 */
class TnlStreakGemFX {
public:
    /**
     * Construct a hidden arrow of a track.
     *
     * @param nTrack The track.
     * @param nPlayer The player whose colour the arrow takes.
     * @ghidraAddress NTSC-U/C: 0x001f0968
     * @ghidraAddress PAL: 0x001f9708
     */
    TnlStreakGemFX(int nTrack, int nPlayer);

    /**
     * Destroy the arrow.
     *
     * @ghidraAddress NTSC-U/C: 0x001f0bf8
     * @ghidraAddress PAL: 0x001f9998
     */
    ~TnlStreakGemFX();

    /**
     * Fade the arrow out.
     *
     * @param bNow Whether the arrow vanishes at once and gives up its place in sShownMask.
     * @ghidraAddress NTSC-U/C: 0x001f0c20
     * @ghidraAddress PAL: 0x001f99c0
     */
    void Hide(bool bNow);

    /**
     * Move the arrow to a gem, fading it out of its old place first.
     *
     * @param flTick The tick of the gem.
     * @param nSlot The slot of the gem across the track.
     * @ghidraAddress NTSC-U/C: 0x001f0c68
     * @ghidraAddress PAL: 0x001f9a08
     */
    void Show(float flTick, int nSlot);

    /**
     * Set the text of the arrows, or queue it while an arrow is shown.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x001f0ca0
     * @ghidraAddress PAL: 0x001f9a40
     */
    void SetText(const char *pszText);

    /**
     * Place the shown arrow again where the tunnel changed under it.
     *
     * @param pTunnel The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001f0d00
     * @ghidraAddress PAL: 0x001f9aa0
     */
    void Refresh(GfxTunnel *pTunnel);

    /**
     * Advance the blink of the arrow.
     *
     * @param flDelta The time since the last update.
     * @param pTunnel The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001f0d60
     * @ghidraAddress PAL: 0x001f9b00
     */
    void Poll(float flDelta, GfxTunnel *pTunnel);

    /**
     * Draw the arrow, faded by its blink and by its distance ahead of the song.
     *
     * @ghidraAddress NTSC-U/C: 0x001f0eb8
     * @ghidraAddress PAL: 0x001f9c58
     */
    void Draw();

    /**
     * The blink per millisecond, the reciprocal of `streak_arrow_fade_ms` clamped to 0 to 1.
     *
     * @ghidraAddress NTSC-U/C: 0x003afb20
     */
    static float sBlinkRate;

    /**
     * The text that waits for no arrow to be shown, or empty.
     *
     * @ghidraAddress NTSC-U/C: 0x003afb78
     */
    static char sPendingText[16];

    /**
     * The bit of each shown arrow.
     *
     * @ghidraAddress NTSC-U/C: 0x003afb88
     */
    static unsigned char sShownMask;

    /**
     * The fade of an arrow by its ticks ahead of the song.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b5c8
     */
    static LinearInterpolator sDistanceFade;

    int mTrack;         /*!< The track. */
    unsigned char mBit; /*!< The bit of the arrow in sShownMask. */
    Rnd::View *mView;   /*!< The view of the arrow, `streakarrow.view`. */
    Rnd::Text *mText;   /*!< The text of the arrows, `streakarrow.txt`. */
    Rnd::Mat *mMat;     /*!< The material of the arrow in the colour of the player. */
    Rnd::Mat *mFontMat; /*!< The material of the text in the colour of the player. */
    float mAlpha;       /*!< The blink, from 0 to 1. */
    float mBlinkRate;   /*!< The change of mAlpha per millisecond. */
    float mTick;        /*!< The tick of the gem. */
    int mSlot;          /*!< The slot of the gem. */
    Transform mXfm;     /*!< The placed transform of the arrow. */
    int mMovePending;   /*!< Whether the arrow moves once it has faded out. */

private:
    /**
     * Place the arrow at its gem.
     *
     * @param pTunnel The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001f0ff8
     * @ghidraAddress PAL: 0x001f9d98
     */
    void Place(GfxTunnel *pTunnel);
};
