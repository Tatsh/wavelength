#pragma once

#include "app/ovyremixgenpanel.h"
#include "app/rampanimator.h"

/**
 * Effect panel of the remix head-up display, which slides out of the main panel.
 *
 * The RTTI records the class as deriving from OvyRemixGenPanel. The object is 0x60 bytes.
 */
class OvyRemixSubPanel : public OvyRemixGenPanel {
public:
    /** The frame of the slide at which the panel is hidden behind the main panel. */
    static constexpr float kHiddenFrame = 240.0f;

    /** The frame of the slide at which the panel is fully out. */
    static constexpr float kShownFrame = 360.0f;

    /**
     * Construct a hidden panel of one type.
     *
     * The scene objects are `HUD<hud>r_fxs_<name>`, where the name is `pat`, `chs`, `stt`, `eco`,
     * `bot`, or `bpm` for the types 1 to 6, and the slide is `HUD<hud>r_fxs.tnm`.
     *
     * @param nType The type, one of GfxManager::RemixSubPanel other than the main panel.
     * @param chHud The letter of the head-up display layout.
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001b77d0
     * @ghidraAddress PAL: 0x001c0570
     */
    OvyRemixSubPanel(int nType, char chHud, int nPlayer);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x001b7a00
     * @ghidraAddress PAL: 0x001c07a0
     */
    ~OvyRemixSubPanel() override;

    /**
     * Advance the slide, show the panel while it is out, and blink the cursor.
     *
     * @param fDelta The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001b7a88
     * @ghidraAddress PAL: 0x001c0828
     */
    void Poll(float fDelta) override;

    /**
     * Draw the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x001b7b10
     * @ghidraAddress PAL: 0x001c08b0
     */
    void Draw() override;

    /**
     * Slide the panel out or back.
     *
     * @param bShow Whether to slide the panel out.
     * @ghidraAddress NTSC-U/C: 0x001b7a50
     * @ghidraAddress PAL: 0x001c07f0
     */
    void Show(bool bShow);

    /**
     * Report whether the panel is of a type.
     *
     * @param nType The type.
     * @return Whether mType is nType.
     * @ghidraAddress NTSC-U/C: 0x001b7b30
     * @ghidraAddress PAL: 0x001c08d0
     */
    bool IsType(int nType) const;

    int mType;           /*!< The type, one of GfxManager::RemixSubPanel. */
    RampAnimator mSlide; /*!< The slide, from kHiddenFrame to kShownFrame. */
};
