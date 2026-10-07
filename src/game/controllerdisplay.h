#pragma once

#include "game/inputmap.h"

/**
 * Display of the controller buttons on the screen around a song.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the button names it
 * shows. Each of the three lanes shows the icon of the controller button it is played with.
 */
class ControllerDisplay {
public:
    /**
     * Construct a hidden display.
     *
     * @ghidraAddress NTSC-U/C: 0x0014aee0
     * @ghidraAddress PAL: 0x0014c880
     */
    ControllerDisplay();

    /**
     * Report the display.
     *
     * @return The display, constructed on the first call.
     * @ghidraAddress NTSC-U/C: 0x0014aef8
     * @ghidraAddress PAL: 0x0014c898
     */
    static ControllerDisplay *shared();

    /**
     * Refresh the display and its button labels.
     *
     * @ghidraAddress NTSC-U/C: 0x0014af40
     * @ghidraAddress PAL: 0x0014c8e0
     */
    void Init();

    /**
     * Add one request to show the display.
     *
     * @ghidraAddress NTSC-U/C: 0x0014af78
     * @ghidraAddress PAL: 0x0014c918
     */
    void Show();

    /**
     * Withdraw one request to show the display.
     *
     * @ghidraAddress NTSC-U/C: 0x0014afa0
     * @ghidraAddress PAL: 0x0014c940
     */
    void Hide();

    /**
     * Highlight the buttons of every lane or clear the highlight.
     *
     * @param bHighlight Whether the buttons are highlighted.
     * @ghidraAddress NTSC-U/C: 0x0014afe8
     * @ghidraAddress PAL: 0x0014c988
     */
    void SetHighlight(bool bHighlight);

    /**
     * Show the display in one of its two modes.
     *
     * @param bOption The mode. The buttons are hidden while it is set.
     * @ghidraAddress NTSC-U/C: 0x0014afc8
     * @ghidraAddress PAL: 0x0014c968
     */
    void SetOption(bool bOption);

    /**
     * Clear the mode, the highlight, and the count.
     *
     * @ghidraAddress NTSC-U/C: 0x0014b048
     * @ghidraAddress PAL: 0x0014c9e8
     */
    void Clear();

    /**
     * Refresh the button labels from the first player's controller bindings.
     *
     * The tutorial keeps the labels it has.
     *
     * @ghidraAddress NTSC-U/C: 0x0014b128
     * @ghidraAddress PAL: 0x0014cac8
     */
    void RefreshLabels();

    int mShowCount; /*!< The requests to show the display. */
    int mOption;    /*!< Nonzero in the mode that hides the buttons. */
    int mHighlight; /*!< Nonzero while the buttons are highlighted. */

private:
    /**
     * Show the button of every lane while the display is requested, or hide it.
     *
     * @ghidraAddress NTSC-U/C: 0x0014b058
     * @ghidraAddress PAL: 0x0014c9f8
     */
    void Refresh();

    /**
     * Report the glyph of the controller button bound to an action.
     *
     * @param pMap The controller bindings.
     * @param nAction The action.
     * @return The glyph, or null when no button the display knows is bound to the action.
     * @ghidraAddress NTSC-U/C: 0x0014ae38
     * @ghidraAddress PAL: 0x0014c7d8
     */
    static const char *GetButtonGlyph(InputMap *pMap, int nAction);
};

/**
 * The display of the controller buttons.
 *
 * @ghidraAddress NTSC-U/C: 0x00436298
 */
extern ControllerDisplay *TheControllerDisplay;
