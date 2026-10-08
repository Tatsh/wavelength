#include "met/freqloadlist.h"

#include "met/avatarpanel.h"
#include "met/metagameutil.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/mesh.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPanel[] = "f_load_p";

// The account state shows as `yes` for this value of Campaign::mNameLocked.
constexpr int kNameLocked = 1;

} // namespace

FreqLoadList::FreqLoadList(DataArray *pData, const char *pszPanel) : FreqList(pData, pszPanel) {
}

void FreqLoadList::SetProfiles(const std::vector<Campaign> &profiles) {
    dynamic_cast<AvatarPanel *>(TheUI.FindPanel(kPanel, false))->SetAvatar(nullptr);
    mProfiles = profiles;
    Refresh(static_cast<int>(mProfiles.size()), kKeepSelection);
}

void FreqLoadList::UpdateRow(int nRow, int nItem) {
    SetCellText(nRow, 0, mProfiles[nItem].mName.c_str());
}

void FreqLoadList::UpdateCursor() {
    UIList::UpdateCursor();
    String format;
    String text;

    UIComponent *pComponent = TheUI.FindComponent(kPanel, "born", false);
    format = TheLocale.Localize("f_load_p_14", true);
    const Campaign &profile = mProfiles[mSelected];
    String born;
    profile.mBorn.FormatDate(born);
    text = FormatString(format.c_str(), born.c_str());
    pComponent->SetText(text.c_str());

    pComponent = TheUI.FindComponent(kPanel, "account", false);
    format = TheLocale.Localize("f_load_p_15", true);
    if (mProfiles[mSelected].mNameLocked == kNameLocked) {
        text = FormatString(format.c_str(), TheLocale.Localize("yes", true));
    } else {
        text = FormatString(format.c_str(), TheLocale.Localize("no", true));
    }
    pComponent->SetText(text.c_str());

    dynamic_cast<AvatarPanel *>(TheUI.FindPanel(kPanel, false))
        ->SetAvatar(&mProfiles[mSelected].mAvatar);

    Rnd::Mesh *pRank = dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find("f_load_p_rank.mesh"));
    pRank->SetMat(FindRankMaterial(mProfiles[mSelected].GetHighestBeatenSkillLevel()));
}
