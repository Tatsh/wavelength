#pragma once

#ifdef VIDEO_STANDARD_PAL

#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel whose file is that of the console's language.
 *
 * The RTTI records the class as deriving from UIPanel. The object is 0x90 bytes, as UIPanel is. In
 * a language other than English the panel loads `<directory>/<name>_<language>.rnd`. The metagame
 * registers the class for the panel type `localized_panel`.
 */
class LocalizedPanel : public UIPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress PAL: 0x00174ba0
     */
    LocalizedPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress PAL: 0x003c4c90
     */
    ~LocalizedPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress PAL: 0x003c20c8
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new LocalizedPanel(pData, pszDir);
    }
};

#endif
