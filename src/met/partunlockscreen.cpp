#include "met/partunlockscreen.h"

#include <string.h>

#include "game/gamedb.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/animatable.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "ui/uimanager.h"

namespace {

constexpr char kMetagameKey[] = "metagame";
constexpr char kViewName[] = "s_unlock.view";
constexpr char kHaloPart[] = "fm_hg_halo";

// The rows of a page. Row 0 shows the prefabricated Freq, and the others show one part each.
constexpr int kRowCount = 8;
constexpr int kFirstPartRow = 1;
constexpr int kPrefabRow = 0;

// Row n shows its kind in the text numbered 2n + 2 and its name in the text numbered 2n + 3.
constexpr int kRowTextStride = 2;
constexpr int kKindTextOffset = 2;
constexpr int kNameTextOffset = 3;

// The kinds AddItem() does not queue.
constexpr unsigned char kSkippedKinds[] = {4, 6, 15};

// The Freq applies its parts during the last second before a page shows. A part that has not
// loaded by then delays the page by half a second.
constexpr float kApplyLeadMs = 1000.0f;
constexpr float kApplyDelayMs = 500.0f;

// The value of mNextMs while no page is due.
constexpr float kNever = 1.0e30f;

} // namespace

PartUnlockScreen::PartUnlockScreen(DataArray *pData) : FreqScreen(pData), mHalo(0) {
}

void PartUnlockScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    mIndex = 0;
    mNextMs = kNever;
    SystemConfig()->FindArray(kMetagameKey, true)->FindFloat("unlock_prefab_time", &mDelayMs, true);
    mPanel = dynamic_cast<UnlockAvatarPanel *>(TheUI.FindPanel("s_unloc_p", false));
    ShowNextItems();
    mAnimPlayer.SetAnim(dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find(kViewName)));
    FreqScreen::Enter(pPrevScreen, fTime);
}

void PartUnlockScreen::Exit(UIScreen *pNextScreen, float fTime) {
    mItems.clear();
    mHalo = 0;
    mAnimPlayer.Stop();
    FreqScreen::Exit(pNextScreen, fTime);
}

void PartUnlockScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    mAnimPlayer.Poll(fTime);
    if (mNextMs - kApplyLeadMs < SystemMs() && !mPanel->UpdateAvatar()) {
        mNextMs += kApplyDelayMs;
    }
    if (mNextMs < SystemMs() && !mAnimPlayer.IsPlaying()) {
        if (static_cast<unsigned>(mIndex) < mItems.size()) {
            mNextMs = SystemMs() + mDelayMs;
            ShowNextItems();
            mAnimPlayer.SetAnim(dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find(kViewName)));
            mAnimPlayer.Start(TheUI.mTime);
        } else {
            TheUI.GotoScreen("unlock_parts_done");
        }
    }
}

void PartUnlockScreen::SetLine(int nLine, const char *pszText) {
    Rnd::Text *pText =
        dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(FormatString("s_unlock_%02d.txt", nLine)));
    pText->SetText(pszText);
    pText->UpdateCursors();
}

void PartUnlockScreen::SetLinePair(int nRow, int nKind, const char *pszName) {
    const int nText = nRow * kRowTextStride;
    if (pszName != nullptr) {
        SetLine(nText + kKindTextOffset, PartLabel(nKind));
        SetLine(nText + kNameTextOffset, pszName);
    } else {
        SetLine(nText + kKindTextOffset, "");
        SetLine(nText + kNameTextOffset, "");
    }
}

void PartUnlockScreen::ShowNextItems() {
    int nRow = kFirstPartRow;
    const char *pszPrefab = nullptr;
    for (unsigned i = mIndex; i < mItems.size(); ++i) {
        const UnlockableItem &item = mItems[i];
        if (item.mKind == UnlockableItem::kKindPrefab) {
            pszPrefab = item.mName;
            break;
        }
        if (nRow >= kRowCount) {
            break;
        }
        SetLinePair(nRow, item.mKind, TheLocale.Localize(item.mName, true));
        ++mIndex;
        ++nRow;
    }
    for (; nRow < kRowCount; ++nRow) {
        SetLinePair(nRow, UnlockableItem::kKindHead, nullptr); // The kind is unused without a name.
    }
    if (pszPrefab != nullptr) {
        SetLinePair(kPrefabRow, UnlockableItem::kKindPrefab, TheLocale.Localize(pszPrefab, true));
        mPrefab.Load(SystemConfig()
                         ->FindArray(kMetagameKey, true)
                         ->FindArray("locked_prefabs", true)
                         ->FindArray(pszPrefab, true));
        mPanel->SetParts(mPrefab);
        ++mIndex;
        return;
    }
    SetLinePair(kPrefabRow, UnlockableItem::kKindPrefab, nullptr);
    if (mHalo) {
        AvatarPartSet *pAvatar = TheGameDb->GetAvatar(0);
        pAvatar->SetPart(AvatarPartSet::kPartHeadGear, kHaloPart);
        mPanel->SetParts(*pAvatar);
    }
}

const char *PartUnlockScreen::PartLabel(int nKind) {
    const char *pszLabel = nullptr;
    switch (nKind) {
    case UnlockableItem::kKindHead:
        pszLabel = "head_label";
        break;
    case UnlockableItem::kKindTorso:
        pszLabel = "torso_label";
        break;
    case UnlockableItem::kKindEmblem:
        pszLabel = "emblem_label";
        break;
    case UnlockableItem::kKindHeadGear:
        pszLabel = "head_gear_label";
        break;
    case UnlockableItem::kKindFaceGear:
        pszLabel = "face_gear_label";
        break;
    case UnlockableItem::kKindArms:
        pszLabel = "arms_label";
        break;
    case UnlockableItem::kKindLowerBody:
        pszLabel = "lowerbody_label";
        break;
    case UnlockableItem::kKindPrefab:
        pszLabel = "prefab_label";
        break;
    default:
        DebugWarn(" Couldn't find part type: %d", nKind);
        break;
    }
    return TheLocale.Localize(pszLabel, true);
}

bool PartUnlockScreen::AddItem(const UnlockableItem &item) {
    for (const unsigned char nSkipped : kSkippedKinds) {
        if (item.mKind == nSkipped) {
            return false;
        }
    }
    mItems.push_back(item);
    if (strcmp(item.mName, kHaloPart) == 0) {
        mHalo = 1;
    }
    return true;
}

bool PartUnlockScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    mNextMs = SystemMs() + mDelayMs;
    mAnimPlayer.Start(TheUI.mTime);
    return FreqScreen::HandleTransitionComplete(pMsg);
}

bool PartUnlockScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool PartUnlockScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
