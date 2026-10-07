#pragma once

/**
 * Display of the controller buttons on the screen around a song.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the button names it
 * shows. Only the members its callers here use are declared.
 */
class ControllerDisplay {
public:
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
     * @param bOption The mode.
     * @ghidraAddress NTSC-U/C: 0x0014afc8
     * @ghidraAddress PAL: 0x0014c968
     */
    void SetOption(bool bOption);

    /**
     * Clear the mode, the selection, and the count.
     *
     * @ghidraAddress NTSC-U/C: 0x0014b048
     * @ghidraAddress PAL: 0x0014c9e8
     */
    void Clear();

    /**
     * Refresh the button labels.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0014b128
     * @ghidraAddress PAL: 0x0014cac8
     */
    void RefreshLabels();
};

/**
 * The display of the controller buttons.
 *
 * @ghidraAddress NTSC-U/C: 0x00436298
 */
extern ControllerDisplay *TheControllerDisplay;
