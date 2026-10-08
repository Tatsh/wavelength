#pragma once

#include <afxctl.h>

#include "synthedit/Resource.h"

/**
 * The control's property page, which has no properties.
 *
 * The class name is from the runtime class information.
 */
class CSyntheditPropPage : public COlePropertyPage {
    DECLARE_DYNCREATE(CSyntheditPropPage)
    DECLARE_OLECREATE_EX(CSyntheditPropPage)

public:
    /** @ghidraAddress 0x1000a1b4 */
    CSyntheditPropPage();

    /** The dialogue template. */
    enum { IDD = IDD_PROPPAGE_SYNTHEDIT };

protected:
    /**
     * Exchange the page's values with its controls.
     *
     * @param pDX The exchange.
     * @ghidraAddress 0x1000a1dd
     */
    virtual void DoDataExchange(CDataExchange *pDX);

    DECLARE_MESSAGE_MAP()
};
