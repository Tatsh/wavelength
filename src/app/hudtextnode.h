#pragma once

#include "rnd/text.h"
#include "rnd/view.h"

/**
 * Line of text on the head-up display, drawn on its own over the rest.
 *
 * The class is not polymorphic and emits no RTTI. The object is 4 bytes.
 */
class HudTextNode {
public:
    /**
     * Construct a hidden line.
     *
     * @param pszText The text object, such as `HUD genmsg.txt`.
     * @param pHudView The view the text is removed from.
     * @ghidraAddress NTSC-U/C: 0x001bf8c0
     * @ghidraAddress PAL: 0x001c8660
     */
    HudTextNode(const char *pszText, Rnd::View *pHudView);

    /**
     * Show a line, or hide the line for a null or empty text.
     *
     * @param pszText The line, or null.
     * @ghidraAddress NTSC-U/C: 0x001bf970
     * @ghidraAddress PAL: 0x001c8710
     */
    void SetText(const char *pszText);

    /**
     * Hide the line.
     *
     * @ghidraAddress NTSC-U/C: 0x001bf9f8
     * @ghidraAddress PAL: 0x001c8798
     */
    void Hide();

    /**
     * Draw the line opaque while it shows.
     *
     * @ghidraAddress NTSC-U/C: 0x001bfa28
     * @ghidraAddress PAL: 0x001c87c8
     */
    void Draw();

    Rnd::Text *mText; /*!< The text object. */
};
