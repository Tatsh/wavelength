#include "app/ovyremixpanel.h"

#include <algorithm>

#include "game/gamedb.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "os/debug.h"
#include "os/string.h"
#include "rnd/transanim.h"

namespace {

// The row of a transform that holds the translation.
constexpr int kXfmRowTranslation = 3;

// The layout letters of the remix displays.
constexpr char kHudSolo = 's';
constexpr char kHudLocal = 'm';
constexpr char kHudOnline = 'n';

} // namespace

Vector3 OvyRemixPanel::sPositions[kNumPositions] = {
    {-258.0f, 0.0f, 164.0f},
    {-106.0f, 0.0f, 164.0f},
    {46.0f, 0.0f, 164.0f},
    {198.0f, 0.0f, 164.0f},
};

OvyRemixPanel::OvyRemixPanel(int nPlayer, int nPosition, Rnd::Transformable *pParent)
    : HideablePanel(nullptr, nullptr, false) {
    mShownSubPanel = nullptr;
    mNextSubPanel = nullptr;
    char chHud = kHudSolo;
    int nLayout = 1;
    switch (TheGameDb->mCommunity) {
    case GameDb::kCommunitySolo:
        nLayout = 0;
        break;
    case GameDb::kCommunityLocal:
        chHud = kHudLocal;
        nLayout = nPlayer != 0;
        break;
    case GameDb::kCommunityOnline:
        chHud = kHudOnline;
        nLayout = TheGameDb->GetPlayerNetOrder(nPlayer) != 0;
        break;
    default:
        DebugWarn("unknown community type");
        break;
    }
    String prefix(FormatString("HUD%cr_fx%d", chHud, nLayout));
    Load(chHud, prefix.c_str(), nPlayer, pParent);

    Rnd::Transformable *pPanel = mPanel;
    const Vector3 &position = sPositions[nPosition];
    pPanel->mLocalXfm[kXfmRowTranslation][0] = position.x;
    pPanel->mLocalXfm[kXfmRowTranslation][1] = position.y;
    pPanel->mLocalXfm[kXfmRowTranslation][2] = position.z;
    pPanel->mLocalXfm[kXfmRowTranslation][3] = position.w;
    pPanel->mDirty = 1;

    auto *pSlideAnim =
        dynamic_cast<Rnd::TransAnim *>(Clone(FormatString("%s.tnm", prefix.c_str()), nPlayer));
    pSlideAnim->SetTrans(mPanel);
    String unused(FormatString("HUD%cr", chHud)); // Yes, the binary builds this and never reads it.
    SetObjects(pSlideAnim->mName.mStr, nullptr, false);

    mSubPanels.push_back(new OvyRemixSubPanel(GfxManager::kRemixSubPanelChorus, chHud, nPlayer));
    mSubPanels.push_back(new OvyRemixSubPanel(GfxManager::kRemixSubPanelStutter, chHud, nPlayer));
    mSubPanels.push_back(new OvyRemixSubPanel(GfxManager::kRemixSubPanelEcho, chHud, nPlayer));
    if (chHud == kHudOnline) {
        mSubPanels.push_back(
            new OvyRemixSubPanel(GfxManager::kRemixSubPanelBoot, kHudOnline, nPlayer));
    } else {
        mSubPanels.push_back(new OvyRemixSubPanel(GfxManager::kRemixSubPanelTempo, chHud, nPlayer));
    }
    for (OvyRemixSubPanel *pSubPanel : mSubPanels) {
        mPanel->AddTrans(pSubPanel->mPanel);
    }
}

OvyRemixPanel::~OvyRemixPanel() {
    for (auto it = mSubPanels.begin(); it != mSubPanels.end();) {
        delete *it;
        it = mSubPanels.erase(it);
    }
}

void OvyRemixPanel::Poll(float fDelta) {
    HideablePanel::Poll();
    OvyRemixGenPanel::Poll(fDelta);
    for (OvyRemixSubPanel *pSubPanel : mSubPanels) {
        pSubPanel->Poll(fDelta);
    }
    if (mNextSubPanel != mShownSubPanel &&
        mShownSubPanel->mSlide.mValue == OvyRemixSubPanel::kHiddenFrame) {
        mShownSubPanel = mNextSubPanel;
        if (mNextSubPanel != nullptr) {
            mNextSubPanel->Show(true);
        }
    }
}

void OvyRemixPanel::Draw() {
    if (mSlide.mValue == 0.0f) {
        return;
    }
    for (OvyRemixSubPanel *pSubPanel : mSubPanels) {
        pSubPanel->Draw();
    }
    mBackground->Draw();
}

void OvyRemixPanel::ShowSubPanel(int nType) {
    auto it = std::find_if(mSubPanels.begin(), mSubPanels.end(), [nType](OvyRemixSubPanel *pPanel) {
        return pPanel->IsType(nType);
    });
    OvyRemixSubPanel *pSubPanel = it != mSubPanels.end() ? *it : nullptr;
    if (pSubPanel == mShownSubPanel) {
        return;
    }
    if (mShownSubPanel != nullptr) {
        mShownSubPanel->Show(false);
        mNextSubPanel = pSubPanel;
    } else if (pSubPanel != nullptr) {
        mShownSubPanel = pSubPanel;
        mNextSubPanel = pSubPanel;
        pSubPanel->Show(true);
    }
}

OvyRemixGenPanel *OvyRemixPanel::FindSubPanel(int nType) {
    if (nType == GfxManager::kRemixSubPanelMain) {
        return this;
    }
    auto it = std::find_if(mSubPanels.begin(), mSubPanels.end(), [nType](OvyRemixSubPanel *pPanel) {
        return pPanel->IsType(nType);
    });
    return it != mSubPanels.end() ? *it : nullptr;
}

void OvyRemixPanel::LoadPositions(DataArray *pConfig, DataArray *pDefaults) {
    DataArray *pPositions;
    FindConfigArray(pConfig, pDefaults, "remix_panel_positions", &pPositions, true);
    for (int i = 0; i < kNumPositions; ++i) {
        DataArray *pPosition = pPositions->Array(i + 1);
        const float fX = pPosition->Float(0);
        const float fY = pPosition->Float(1);
        const float fZ = pPosition->Float(2);
        sPositions[i].x = fX;
        sPositions[i].y = fY;
        sPositions[i].z = fZ;
    }
}
