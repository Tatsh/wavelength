#pragma once

#include "math/color.h"
#include "rnd/blur.h"
#include "rnd/mat.h"
#include "rnd/text.h"
#include "rnd/transanim.h"
#include "rnd/view.h"

/**
 * Message of two lines that flies across the head-up display and trails a blur.
 *
 * The class is not polymorphic and emits no RTTI. The object is 0x40 bytes. The title follows the
 * head-up display message of the previous game, which shares the scene objects.
 */
class HudTextMessage {
public:
    /** The time the message takes to fly in, and again to fly out, in milliseconds. */
    static constexpr float kFlyTime = 350.0f;

    /**
     * Construct a hidden message from `HUD textmsg.txt`, `HUD sm textmsg.txt`, their blurs, and
     * `<hud> textmsg.tnm`.
     *
     * @param pHudView The view the blurs are removed from.
     * @ghidraAddress NTSC-U/C: 0x001bb000
     * @ghidraAddress PAL: 0x001c3da0
     */
    explicit HudTextMessage(Rnd::View *pHudView);

    /**
     * Destroy the message.
     *
     * @ghidraAddress NTSC-U/C: 0x001bb308
     * @ghidraAddress PAL: 0x001c40a8
     */
    ~HudTextMessage();

    /**
     * Hide the message at once.
     *
     * @ghidraAddress NTSC-U/C: 0x001bb330
     * @ghidraAddress PAL: 0x001c40d0
     */
    void Hide();

    /**
     * Fly a message in, unless one is showing.
     *
     * @param pszText The large line.
     * @param pszSmallText The small line.
     * @param fDuration The time the message rests in view, in milliseconds.
     * @param fScale The horizontal and depth scale of the lines.
     * @param nPlayer The player whose colour the lines take, or a negative value for the default.
     * @param fX The horizontal place of the message.
     * @param fZ The vertical place of the message.
     * @ghidraAddress NTSC-U/C: 0x001bb398
     * @ghidraAddress PAL: 0x001c4138
     */
    void Show(const char *pszText,
              const char *pszSmallText,
              float fDuration,
              float fScale,
              int nPlayer,
              float fX,
              float fZ);

    /**
     * Advance the flight, and hide the message once it has flown out.
     *
     * @ghidraAddress NTSC-U/C: 0x001bb5b0
     * @ghidraAddress PAL: 0x001c4350
     */
    void Poll();

    /**
     * Pose the flight and draw the blurs.
     *
     * @ghidraAddress NTSC-U/C: 0x001bb6a8
     * @ghidraAddress PAL: 0x001c4448
     */
    void Draw();

    Rnd::Blur *mBlur;        /*!< `HUD textmsg.blur`. */
    Rnd::Blur *mSmallBlur;   /*!< `HUD sm textmsg.blur`. */
    Rnd::Text *mText;        /*!< `HUD textmsg.txt`, the large line. */
    Rnd::Text *mSmallText;   /*!< `HUD sm textmsg.txt`, the small line. */
    Rnd::View *mOffsetView;  /*!< `HUD textmsg offset.view`, which places the message. */
    Rnd::TransAnim *mFlight; /*!< `<hud> textmsg.tnm`, the flight. */
    Rnd::Mat *mMat;          /*!< The material of the large line's font. */
    Color mDefaultColor;     /*!< The colour of mMat when the message started. */
    float mFrame;            /*!< The frame of the flight. */
    float mStartTime;        /*!< The song time Show() ran at, or kNotShown. */
    float mDuration;         /*!< The time the message rests in view. */
    int mShowing;            /*!< Whether a message shows. Show() does not set it. */

    /** The value of mStartTime when no message shows. */
    static constexpr float kNotShown = -1e9f;
};
