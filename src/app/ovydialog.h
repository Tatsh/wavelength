#pragma once

#include "app/hideablepanel.h"
#include "rnd/animatable.h"
#include "rnd/text.h"
#include "rnd/view.h"

/**
 * Panel of text the tutorial opens over the head-up display.
 *
 * The RTTI records the class as deriving from HideablePanel. The object is 0x28 bytes.
 */
class OvyDialog : public HideablePanel {
public:
    /**
     * Construct a hidden panel and draw it into the letterboxed view of the head-up display.
     *
     * @param pHudView The view of the head-up display.
     * @ghidraAddress NTSC-U/C: 0x001c0728
     * @ghidraAddress PAL: 0x001c94c8
     */
    explicit OvyDialog(Rnd::View *pHudView);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00367558
     * @ghidraAddress PAL: 0x003d5c88
     */
    ~OvyDialog() override = default;

    /**
     * Pose the place of the panel, unless the frame is negative.
     *
     * @param fFrame The frame of `HUD dialog pos.tnm`.
     * @ghidraAddress NTSC-U/C: 0x003675b0
     * @ghidraAddress PAL: 0x003d5ce0
     */
    void SetPosition(float fFrame) {
        if (0.0f <= fFrame) {
            mPosAnim->SetFrame(fFrame);
        }
    }

    /**
     * Fill the panel and slide it in.
     *
     * @param pszText The text.
     * @param fSize The frame of `HUD dialog panel size.msnm`.
     * @param fPosition The frame of `HUD dialog pos.tnm`, or a negative value to retain the place.
     * @ghidraAddress NTSC-U/C: 0x001c0a00
     * @ghidraAddress PAL: 0x001c97a0
     */
    void Open(const char *pszText, float fSize, float fPosition);

    /**
     * Slide the panel out.
     *
     * @ghidraAddress NTSC-U/C: 0x001c0a98
     * @ghidraAddress PAL: 0x001c9838
     */
    void Close();

    Rnd::Text *mText;           /*!< `HUD dialog.txt`. */
    Rnd::Animatable *mSizeAnim; /*!< `HUD dialog panel size.msnm`. */
    Rnd::Animatable *mPosAnim;  /*!< `HUD dialog pos.tnm`. */
};
