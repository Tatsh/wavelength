#pragma once

#include <vector>

#include "met/freqpanel.h"
#include "os/string.h"
#include "rnd/rndloader.h"
#include "rnd/text.h"
#include "rnd/transformable.h"
#include "rnd/view.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uipanel.h"
#include "ui/uiscreen.h"

/**
 * The panel at the bottom of the front end that explains the focused control and the buttons.
 *
 * The RTTI includes the name and records FreqPanel as the base. The object is 0x150 bytes. The
 * screen file builds one under the type `help_panel`, named `help`. The panel's file provides the
 * text objects `help_help1.txt` to `help_help7.txt` for the help and `help_action1.txt` to
 * `help_action7.txt` for the buttons. A text is split into runs at `<F<n>>` marks, each run in the
 * font the `help_fonts` entry of the metagame configuration gives for `F<n>`, and the runs are
 * placed one after another in the text objects. Every panel of the class shares one loader of the
 * panel's file.
 */
class HelpPanel : public FreqPanel {
public:
    /** One run of a help or button text in one font. */
    struct HelpText {
        String mFont; /*!< The font, a value of the `help_fonts` entry. */
        String mText; /*!< The text. */
    };

    /** The number of text objects for the help and for the buttons. */
    static constexpr int kNumTexts = 7;

    /**
     * Construct a hidden panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x001a5568
     * @ghidraAddress PAL: 0x001ad258
     */
    HelpPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x001a5b08
     * @ghidraAddress PAL: 0x001ad7f8
     */
    ~HelpPanel() override;

    /**
     * Create a panel from its script description.
     *
     * Metagame::RegisterScreenClasses() registers the routine for the entry type `help_panel`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x003618d0
     * @ghidraAddress PAL: 0x003cfde8
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new HelpPanel(pData, pszDir);
    }

    /**
     * Take a reference to the panel's file through the loader every help panel shares, creating
     * the loader on the first call.
     *
     * @ghidraAddress NTSC-U/C: 0x001a55d8
     * @ghidraAddress PAL: 0x001ad2c8
     */
    void Load() override;

    /**
     * Drop a reference to the panel's file, keeping the shared loader on the last reference.
     *
     * @ghidraAddress NTSC-U/C: 0x001a5640
     * @ghidraAddress PAL: 0x001ad330
     */
    void Unload() override;

    /**
     * Find the text objects, the view, and the fonts once the panel's file has loaded, and record
     * where the first help and button texts start.
     *
     * @ghidraAddress NTSC-U/C: 0x001a5670
     * @ghidraAddress PAL: 0x001ad360
     */
    void FinishLoad() override;

    /**
     * Report the font of a font name of the `help_fonts` entry.
     *
     * @param pszName The font name, such as `F1`.
     * @return The font.
     * @ghidraAddress NTSC-U/C: 0x001a5c90
     * @ghidraAddress PAL: 0x001ad980
     */
    const char *FontOf(const char *pszName);

    /**
     * Split a text into runs at its `<F<n>>` marks into mRuns. The first run is in `F1`.
     *
     * @param text The text.
     * @ghidraAddress NTSC-U/C: 0x001a5cc0
     * @ghidraAddress PAL: 0x001ad9b0
     */
    void ParseText(const String &text);

    /**
     * Empty the help text objects.
     *
     * @ghidraAddress NTSC-U/C: 0x001a6388
     * @ghidraAddress PAL: 0x001ae078
     */
    void ClearHelp();

    /**
     * Place the runs in the help text objects left to right, and centre them on where the first
     * help text starts.
     *
     * @ghidraAddress NTSC-U/C: 0x001a6430
     * @ghidraAddress PAL: 0x001ae120
     */
    void LayoutHelp();

    /**
     * Place the runs in the button text objects right to left from where the first button text
     * starts.
     *
     * @ghidraAddress NTSC-U/C: 0x001a6708
     * @ghidraAddress PAL: 0x001ae3f8
     */
    void LayoutActions();

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

    /**
     * The loader every help panel shares, or null before the first Load().
     *
     * @ghidraAddress NTSC-U/C: 0x003af8dc
     */
    static RndLoader *sLoader;

    Rnd::View *mView;                                        /*!< `help.view`. */
    DataArray *mFonts;                                       /*!< The `help_fonts` entry. */
    std::vector<Rnd::Text *> mHelpTexts;                     /*!< `help_help<n>.txt`. */
    std::vector<Rnd::Text *> mActionTexts;                   /*!< `help_action<n>.txt`. */
    alignas(16) float mHelpOrigin[Rnd::kXfmRowFloatCount];   /*!< Where the help starts. */
    alignas(16) float mActionOrigin[Rnd::kXfmRowFloatCount]; /*!< Where the buttons start. */
    const char *mHelpText;                                   /*!< The help that shows. */
    const char *mActionText;                                 /*!< The text of the buttons. */
    std::vector<HelpText> mRuns;                             /*!< The runs of the last text. */
};
