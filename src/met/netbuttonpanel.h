#pragma once

#include "met/focuschangepanel.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel of buttons of the online screens that shows its focused materials as soon as it loads.
 *
 * The RTTI records the class as deriving from FocusChangePanel. Its vtable is at `0x003cd0b8`.
 */
class NetButtonPanel : public FocusChangePanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00171898
     * @ghidraAddress PAL: 0x00174ce0
     */
    NetButtonPanel(DataArray *pData, const char *pszDir) : FocusChangePanel(pData, pszDir) {
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00357a88
     * @ghidraAddress PAL: 0x003c4e38
     */
    ~NetButtonPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new NetButtonPanel(pData, pszDir);
    }

    /**
     * Finish the load and take the focus.
     *
     * @ghidraAddress NTSC-U/C: 0x001718d0
     * @ghidraAddress PAL: 0x00174d18
     */
    void FinishLoad() override;
};
