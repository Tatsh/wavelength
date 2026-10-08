#include "synthedit/SyntheditPpg.h"

IMPLEMENT_DYNCREATE(CSyntheditPropPage, COlePropertyPage)

BEGIN_MESSAGE_MAP(CSyntheditPropPage, COlePropertyPage)
END_MESSAGE_MAP()

IMPLEMENT_OLECREATE_EX(CSyntheditPropPage,
                       "SYNTHEDIT.SyntheditPropPage.1",
                       0x66cd1366,
                       0xd0ee,
                       0x4357,
                       0x82,
                       0xee,
                       0xe1,
                       0xd4,
                       0x48,
                       0x76,
                       0xc3,
                       0x84)

// 0x1000a17d
BOOL CSyntheditPropPage::CSyntheditPropPageFactory::UpdateRegistry(BOOL bRegister) {
    if (bRegister) {
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(), m_clsid, IDS_SYNTHEDIT_PPG);
    }
    return AfxOleUnregisterClass(m_clsid, NULL);
}

CSyntheditPropPage::CSyntheditPropPage() : COlePropertyPage(IDD, IDS_SYNTHEDIT_PPG_CAPTION) {
}

void CSyntheditPropPage::DoDataExchange(CDataExchange *pDX) {
    DDP_PostProcessing(pDX);
}
