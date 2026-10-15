#pragma once

#ifdef VIDEO_STANDARD_PAL

#include "met/freqpanel.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Front-end panel whose file is that of the console's language.
 *
 * The RTTI records the class as deriving from FreqPanel. The object is 0xe0 bytes, as FreqPanel
 * is. In a language other than English the panel loads `<directory>/<name>_<language>.rnd`. The
 * metagame registers the class for the panel type `localized_freq_panel`.
 */
class LocalizedFreqPanel : public FreqPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress PAL: 0x00174c40
     */
    LocalizedFreqPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress PAL: 0x003c4d38
     */
    ~LocalizedFreqPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress PAL: 0x003c2118
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new LocalizedFreqPanel(pData, pszDir);
    }
};

#endif
