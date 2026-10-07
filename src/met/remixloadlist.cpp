#include "met/remixloadlist.h"

#include "rnd/font.h"
#include "rnd/manager.h"

RemixLoadList::RemixLoadList(DataArray *pData, const char *pszPanel)
    : FreqList(pData, pszPanel), mGreyReadOnly(0), mGreyUnplayable(0) {
    pData->FindSymbol("pic_panel", &mPicPanel, false);
}

void RemixLoadList::SetRemixes(const std::vector<RemixInfo> &remixes,
                               int nGreyReadOnly,
                               int nGreyUnplayable) {
    mRemixes = remixes;
    mGreyReadOnly = nGreyReadOnly;
    mGreyUnplayable = nGreyUnplayable;
    Refresh(static_cast<int>(remixes.size()), kKeepSelection);
}

void RemixLoadList::UpdateRow(int nRow, int nItem) {
    const RemixInfo &remix = mRemixes[nItem];
    const bool bGrey = (mGreyReadOnly && remix.mReadOnly) || (mGreyUnplayable && !remix.mPlayable);
    if (bGrey) {
        SetCellFont(nRow, 0, dynamic_cast<Rnd::Font *>(Rnd::TheManager.Find("lucida_1_grey.font")));
    } else {
        SetCellFont(
            nRow, 0, dynamic_cast<Rnd::Font *>(Rnd::TheManager.Find("lucida_1_white.font")));
    }
    SetCellText(nRow, 0, mRemixes[nItem].mName);
}

void RemixLoadList::UpdateCursor() {
    UIList::UpdateCursor();
    mRemixes[mSelected].ShowDetails(mPicPanel);
}
