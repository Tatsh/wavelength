#pragma once

#include "ui/uicomponent.h"
#include "ui/uipanel.h"
#include "ui/uiscreen.h"

/**
 * The panel at the bottom of the front end that explains the focused control and the buttons.
 *
 * The RTTI includes the name. The screen file builds one under the type `help_panel`, named
 * `help`. Only the members the metagame uses are declared.
 */
class HelpPanel : public UIPanel {
public:
    /**
     * Show the help of the component that has the focus on the current screen, or clear the help
     * when no panel has the focus.
     *
     * @ghidraAddress NTSC-U/C: 0x001a6998
     * @ghidraAddress PAL: 0x001ae688
     */
    void Refresh();

    /**
     * Show the help of a component, from the `<screen>_HELP`, `<panel>_HELP`, or
     * `<panel>_<component>_HELP` token of the locale.
     *
     * @param pPanel The panel of the component.
     * @param pComponent The component, or null.
     * @ghidraAddress NTSC-U/C: 0x001a69e0
     * @ghidraAddress PAL: 0x001ae6d0
     */
    void ShowHelp(UIPanel *pPanel, UIComponent *pComponent);

    /**
     * Show the buttons of a screen, from the `<screen>_ACTION` or `<panel>_ACTION` token of the
     * locale, or from `default_ACTION`.
     *
     * @param pScreen The screen, or null for the default.
     * @ghidraAddress NTSC-U/C: 0x001a6b30
     * @ghidraAddress PAL: 0x001ae820
     */
    void ShowActions(UIScreen *pScreen);

    /**
     * Show a help text.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x001a6ca0
     * @ghidraAddress PAL: 0x001ae990
     */
    void SetHelpText(const char *pszText);

    /**
     * Show a text of the buttons.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x001a6cf0
     * @ghidraAddress PAL: 0x001ae9e0
     */
    void SetActionText(const char *pszText);
};
