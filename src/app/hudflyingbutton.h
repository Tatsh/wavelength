#pragma once

#include "math/interpolator.h"
#include "math/vector2.h"
#include "rnd/blur.h"
#include "rnd/text.h"

/**
 * Button icon of a lane that flies to a place on the screen and pulses there.
 *
 * The class is not polymorphic and emits no RTTI. The object is 0x20 bytes. The tutorial shows
 * which button to press with it.
 */
class HudFlyingButton {
public:
    /**
     * Construct a hidden icon of a lane from `HUD flying button <n>.txt` and its blur.
     *
     * The icon of lane 0 reads the flight curve `button_icon_interp` of the `gfx` section of the
     * configuration, which every icon shares.
     *
     * @param nLane The lane.
     * @ghidraAddress NTSC-U/C: 0x001bfa88
     * @ghidraAddress PAL: 0x001c8828
     */
    explicit HudFlyingButton(int nLane);

    /**
     * Destroy the icon and the shared flight curve.
     *
     * @ghidraAddress NTSC-U/C: 0x001bfc18
     * @ghidraAddress PAL: 0x001c89b8
     */
    ~HudFlyingButton();

    /**
     * Hide the icon and stop its flight.
     *
     * @ghidraAddress NTSC-U/C: 0x001bfc88
     * @ghidraAddress PAL: 0x001c8a28
     */
    void Reset();

    /**
     * Fly the icon from one place to another, or put it at the place when both are the same.
     *
     * @param pFrom The place the flight starts at, as x and z.
     * @param pTo The place the flight ends at.
     * @ghidraAddress NTSC-U/C: 0x001bfcf0
     * @ghidraAddress PAL: 0x001c8a90
     */
    void Fly(const Vector2 *pFrom, const Vector2 *pTo);

    /**
     * Hide the icon.
     *
     * @ghidraAddress NTSC-U/C: 0x001bfdf8
     * @ghidraAddress PAL: 0x001c8b98
     */
    void Hide();

    /**
     * Start or stop the pulse of the icon.
     *
     * @param bPulse Whether the icon pulses.
     * @ghidraAddress NTSC-U/C: 0x001bfe40
     * @ghidraAddress PAL: 0x001c8be0
     */
    void SetPulse(bool bPulse);

    /**
     * Advance the flight, and start the pulse once the icon arrives.
     *
     * @ghidraAddress NTSC-U/C: 0x001bfea0
     * @ghidraAddress PAL: 0x001c8c40
     */
    void Poll();

    /**
     * Pulse the icon and draw its blur while it shows.
     *
     * @ghidraAddress NTSC-U/C: 0x001bffb8
     * @ghidraAddress PAL: 0x001c8d58
     */
    void Draw();

    /**
     * The end of the shared flight curve, the time a flight takes.
     *
     * @ghidraAddress NTSC-U/C: 0x003af954
     */
    static float sFlightTime;

    /**
     * The shared flight curve, `button_icon_interp`, or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003af958
     */
    static Interpolator *sFlight;

    Rnd::Text *mText; /*!< `HUD flying button <n>.txt`, the glyph of the button. */
    Rnd::Blur *mBlur; /*!< `HUD flying button <n>.blur`. */
    int mPulse;       /*!< Whether the icon pulses. */
    float mStartTime; /*!< The song time the flight started at, or a sentinel. */
    Vector2 mFrom;    /*!< The place the flight starts at. */
    Vector2 mTo;      /*!< The place the flight ends at. */
};
