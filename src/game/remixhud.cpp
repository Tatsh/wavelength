#include "game/remixhud.h"

#include <algorithm>

#include "game/gamedb.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/memfun2command.h"
#include "os/memfun4command.h"
#include "os/scheduler.h"
#include "os/string.h"
#include "synth/fxmidi.h"

namespace {

// The buttons that move the cursor and choose. The values lie outside JoypadButton.
constexpr int kButtonConfirm = 10;
constexpr int kButtonUp = 20;
constexpr int kButtonDown = 22;

constexpr int kNoIndex = -1;
constexpr int kFirstEntryCount = 1;
constexpr int kTempoPanelIndex = 1;
constexpr int kLocalHost = 0;
constexpr float kFirstRepeatMs = 500.0f;
constexpr float kRepeatMs = 100.0f;
constexpr int kPostStaggerTicks = 10;

const char *const kBlankLabel = "";
const char *const kCancelLabel = "(CANCEL)";

} // namespace

RemixHUD::RemixHUD(int nPlayer, const char *pszEffectLabel)
    : mPlayer(nPlayer), mActive(0), mFocused(1), mLoopOn(0), mMuteOn(0), mSoloOn(0), mChorusOn(0),
      mStutterOn(0), mEffectOn(0), mMode(0), mPeers(), mPeerIndex(0), mPanel(0), mEntry(kNoIndex),
      mPanelTypes(), mPanelOfType(kNumPanelTypes, kNoIndex), mSubPanels(), mEntryCounts(),
      mRepeatUpCmd(NewMemFun2Command(this, &RemixHUD::Navigate, true, true)),
      mRepeatDownCmd(NewMemFun2Command(this, &RemixHUD::Navigate, false, true)), mCallback(nullptr),
      mCallbackData(nullptr) {
    AddLoopPanel();
    if (TheGameDb->mCommunity != GameDb::kCommunityOnline && nPlayer == 0) {
        AddTempoPanel();
    }
    if (TheGameDb->mCommunity != GameDb::kCommunityLocal) {
        AddSoloPanel();
    }
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline &&
        TheGameDb->GetPlayerNetOrder(nPlayer) == kLocalHost) {
        AddBootPanel();
    }
    AddChorusPanel();
    AddStutterPanel();
    AddEffectPanel(pszEffectLabel);
    AddMutePanel();
}

RemixHUD::~RemixHUD() {
    if (mActive && mFocused) {
        JoypadRemoveSink(this);
    }
}

void RemixHUD::SetActive(int nActive) {
    if (nActive == mActive) {
        return;
    }
    mActive = nActive;
    if (nActive) {
        if (mFocused) {
            JoypadAddSink(this);
        }
        SelectPanel(0);
        return;
    }
    if (mFocused) {
        JoypadRemoveSink(this);
    }
    TheSongScheduler.Cancel(mRepeatUpCmd.Get());
    TheSongScheduler.Cancel(mRepeatDownCmd.Get());
    if (mEntry != kNoIndex) {
        OpenPanel(mEntry, false);
    }
    TheGfxManager.SelectRemixPanelEntry(mPlayer, kNoIndex, GfxManager::kRemixSubPanelMain);
}

void RemixHUD::SetChorus(int nOn) {
    if (nOn != mChorusOn) {
        TheGfxManager.SetRemixPanelLit(
            mPlayer, mPanelOfType[kPanelChorus], nOn, GfxManager::kRemixSubPanelMain);
    }
    mChorusOn = nOn;
}

void RemixHUD::SetStutter(int nOn) {
    if (nOn != mStutterOn) {
        TheGfxManager.SetRemixPanelLit(
            mPlayer, mPanelOfType[kPanelStutter], nOn, GfxManager::kRemixSubPanelMain);
    }
    mStutterOn = nOn;
}

void RemixHUD::SetEffect(int nOn) {
    if (nOn != mEffectOn) {
        TheGfxManager.SetRemixPanelLit(
            mPlayer, mPanelOfType[kPanelEffect], nOn, GfxManager::kRemixSubPanelMain);
    }
    mEffectOn = nOn;
}

void RemixHUD::SetTempo(float fTempo, int nDimmed) {
    const int nIndex = mPanelOfType[kPanelTempo];
    if (nIndex == kNoIndex) {
        return;
    }
    const String text(
        FormatString(TheLocale.Localize("REMIX_TEMPO", true), static_cast<int>(fTempo)));
    TheGfxManager.SetRemixPanelText(mPlayer, 0, text.c_str(), GfxManager::kRemixSubPanelTempo);
    TheGfxManager.SetRemixPanelText(mPlayer, nIndex, text.c_str(), GfxManager::kRemixSubPanelMain);
    TheGfxManager.SetRemixPanelLit(mPlayer, nIndex, nDimmed ^ 1, GfxManager::kRemixSubPanelMain);
}

void RemixHUD::SetLoop(int nOn) {
    if (nOn != mLoopOn) {
        TheGfxManager.SetRemixPanelLit(
            mPlayer, mPanelOfType[kPanelLoop], nOn, GfxManager::kRemixSubPanelMain);
    }
    mLoopOn = nOn;
}

void RemixHUD::SetMute(int nOn) {
    if (mMode == kModeMute && nOn != mMuteOn) {
        TheGfxManager.SetRemixPanelLit(
            mPlayer, mPanelOfType[kPanelMute], nOn, GfxManager::kRemixSubPanelMain);
    }
    mMuteOn = nOn;
}

void RemixHUD::SetSolo(int nOn) {
    const int nIndex = mPanelOfType[kPanelSolo];
    if (nIndex != kNoIndex && nOn != mSoloOn) {
        TheGfxManager.SetRemixPanelLit(mPlayer, nIndex, nOn, GfxManager::kRemixSubPanelMain);
        mSoloOn = nOn;
    }
}

void RemixHUD::SetMode(int nMode) {
    if (nMode == mMode) {
        return;
    }
    const int nIndex = mPanelOfType[kPanelMute];
    const char *pszText;
    if (nMode == kModeMute) {
        pszText = TheLocale.Localize("REMIX_MUTE", true);
        TheGfxManager.SetRemixPanelLit(
            mPlayer, mPanelOfType[kPanelMute], mMuteOn, GfxManager::kRemixSubPanelMain);
    } else {
        if (mPanelTypes[mPanel] == kPanelMute) {
            SelectPanel(0);
        }
        pszText = kBlankLabel;
    }
    TheGfxManager.SetRemixPanelText(mPlayer, nIndex, pszText, GfxManager::kRemixSubPanelMain);
    mMode = nMode;
}

void RemixHUD::RemovePeer(int nPeer) {
    const auto it = std::find(mPeers.begin(), mPeers.end(), nPeer);
    if (it == mPeers.end()) {
        return;
    }
    mPeers.erase(it);
    if (static_cast<unsigned int>(mPeerIndex) < mPeers.size()) {
        SetPeerIndex(mPeerIndex);
    } else {
        SetPeerIndex(static_cast<int>(mPeers.size()) - 1);
    }
}

void RemixHUD::SetCallback(Callback pfnCallback, void *pData) {
    mCallbackData = pData;
    mCallback = pfnCallback;
}

void RemixHUD::SelectPanel(int nIndex) {
    mPanel = nIndex;
    TheGfxManager.SelectRemixPanelEntry(mPlayer, nIndex, GfxManager::kRemixSubPanelMain);
}

void RemixHUD::SelectEntry(int nEntry) {
    mEntry = nEntry;
    TheGfxManager.SelectRemixPanelEntry(mPlayer, nEntry, mSubPanels[mPanel]);
}

void RemixHUD::OpenPanel(int nIndex, bool bOpen) {
    if (!bOpen) {
        mEntry = kNoIndex;
        TheGfxManager.ShowRemixPanel(mPlayer, GfxManager::kRemixSubPanelMain);
        return;
    }
    const GfxManager::RemixSubPanel eSubPanel = mSubPanels[nIndex];
    if (mCallback != nullptr && eSubPanel == GfxManager::kRemixSubPanelTempo) {
        mCallback(mPlayer, kEventTempoOpened, mCallbackData);
    }
    TheGfxManager.FlashRemixPanel(mPlayer, eSubPanel);
    TheGfxManager.ShowRemixPanel(mPlayer, eSubPanel);
    TheGfxManager.SelectRemixPanelEntry(mPlayer, 0, eSubPanel);
    SelectEntry(0);
}

bool RemixHUD::ConfirmEntry() {
    const GfxManager::RemixSubPanel eSubPanel = mSubPanels[mPanel];
    int nEvent = kEventNone;
    if (eSubPanel == GfxManager::kRemixSubPanelBoot && mPeerIndex != kNoIndex) {
        nEvent = mPeers[mPeerIndex] + kEventBootFirst;
    }
    TheGfxManager.FlashRemixPanel(mPlayer, eSubPanel);
    if (mCallback != nullptr && nEvent != kEventNone) {
        mCallback(mPlayer, nEvent, mCallbackData);
    }
    return true;
}

void RemixHUD::SetPeerIndex(int nIndex) {
    mPeerIndex = nIndex;
    const char *pszText = kCancelLabel;
    if (nIndex != kNoIndex) {
        pszText = TheGameDb->GetPlayerName(mPeers[nIndex]);
    }
    TheGfxManager.SetRemixPanelText(mPlayer, 0, pszText, GfxManager::kRemixSubPanelBoot);
}

void RemixHUD::AddPanel(int nType, const char *pszLabel, int nPostLabel) {
    const int nIndex = static_cast<int>(mPanelTypes.size());
    mPanelTypes.push_back(nType);
    mPanelOfType[nType] = nIndex;
    mSubPanels.push_back(GfxManager::kRemixSubPanelMain);
    mEntryCounts.push_back(kFirstEntryCount);
    if (nPostLabel) {
        PostPanelText(nIndex, pszLabel, GfxManager::kRemixSubPanelMain);
    } else {
        TheGfxManager.SetRemixPanelText(mPlayer, nIndex, pszLabel, GfxManager::kRemixSubPanelMain);
    }
}

void RemixHUD::AddChorusPanel() {
    AddPanel(kPanelChorus, TheLocale.Localize("REMIX_CHORUS", true), 1);
}

void RemixHUD::AddSoloPanel() {
    AddPanel(kPanelSolo, TheLocale.Localize("REMIX_SOLO", true), 1);
}

void RemixHUD::AddLoopPanel() {
    AddPanel(kPanelLoop, TheLocale.Localize("REMIX_LOOP", true), 1);
}

void RemixHUD::AddStutterPanel() {
    AddPanel(kPanelStutter, TheLocale.Localize("REMIX_STUTTER", true), 1);
}

void RemixHUD::AddEffectPanel(const char *pszLabel) {
    AddPanel(kPanelEffect, pszLabel, 1);
}

void RemixHUD::AddMutePanel() {
    AddPanel(kPanelMute, kBlankLabel, 0);
}

void RemixHUD::AddTempoPanel() {
    const int nIndex = static_cast<int>(mPanelTypes.size());
    mPanelTypes.push_back(kPanelTempo);
    mPanelOfType[kPanelTempo] = nIndex;
    mSubPanels.push_back(GfxManager::kRemixSubPanelTempo);
    mEntryCounts.push_back(kFirstEntryCount);
    SetTempo(0.0f, 0);
}

void RemixHUD::AddBootPanel() {
    const int nIndex = static_cast<int>(mPanelTypes.size());
    mPanelTypes.push_back(kPanelBoot);
    mPanelOfType[kPanelBoot] = nIndex;
    mSubPanels.push_back(GfxManager::kRemixSubPanelBoot);
    mEntryCounts.push_back(kFirstEntryCount);
    PostPanelText(nIndex, TheLocale.Localize("REMIX_BOOT", true), GfxManager::kRemixSubPanelMain);
    for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
        if (i != mPlayer) {
            mPeers.push_back(i);
        }
    }
    SetPeerIndex(mPeers.empty() ? kNoIndex : 0);
}

void RemixHUD::PostPanelText(int nIndex, const char *pszText, GfxManager::RemixSubPanel ePanel) {
    Command *pCommand = NewMemFun4Command(
        &TheGfxManager, &GfxManager::SetRemixPanelText, mPlayer, nIndex, pszText, ePanel);
    TheSongScheduler.PostIn(
        pCommand, (ePanel * kPostStaggerTicks + nIndex) * kPostStaggerTicks, false);
}

void RemixHUD::OpenTempo() {
    if (mSubPanels[mPanel] == GfxManager::kRemixSubPanelTempo && mEntry != kNoIndex) {
        return;
    }
    SelectPanel(kTempoPanelIndex);
    (void)Confirm(); // The binary discards the result.
}

void RemixHUD::CloseTempo() {
    if (mSubPanels[mPanel] == GfxManager::kRemixSubPanelTempo) {
        OpenPanel(mPanel, false);
    }
}

void RemixHUD::SetFocused(int nFocused) {
    if (nFocused == mFocused) {
        return;
    }
    if (mActive) {
        if (nFocused) {
            JoypadAddSink(this);
        } else {
            JoypadRemoveSink(this);
            TheSongScheduler.Cancel(mRepeatUpCmd.Get());
            TheSongScheduler.Cancel(mRepeatDownCmd.Get());
        }
    }
    mFocused = nFocused;
}

void RemixHUD::Navigate(bool bUp, bool bRepeat) {
    const float fDelay = bRepeat ? kRepeatMs : kFirstRepeatMs;
    if (bUp) {
        FxMidi::PlayMenuUp();
        TheSongScheduler.PostAfter(mRepeatUpCmd.Get(), fDelay, false);
    } else {
        FxMidi::PlayMenuDown();
        TheSongScheduler.PostAfter(mRepeatDownCmd.Get(), fDelay, false);
    }

    if (mEntry == kNoIndex) {
        // The mute panel, always last, can be chosen only in kModeMute.
        const int nCount = static_cast<int>(mPanelTypes.size());
        const int nLast = (mMode == kModeMute ? nCount : nCount - 1) - 1;
        int nPanel = bUp ? mPanel - 1 : mPanel + 1;
        if (nPanel <= kNoIndex) {
            nPanel = nLast;
        }
        if (nLast < nPanel) {
            nPanel = 0;
        }
        SelectPanel(nPanel);
        return;
    }

    const int nType = mPanelTypes[mPanel];
    if (nType == kPanelTempo) {
        if (mCallback != nullptr) {
            mCallback(mPlayer, bUp ? kEventTempoUp : kEventTempoDown, mCallbackData);
        }
    } else if (nType == kPanelBoot) {
        mPeerIndex += bUp ? -1 : 1;
        if (mPeerIndex < kNoIndex) {
            mPeerIndex = static_cast<int>(mPeers.size()) - 1;
        }
        if (static_cast<unsigned int>(mPeerIndex) >= mPeers.size()) {
            mPeerIndex = kNoIndex;
        }
        SetPeerIndex(mPeerIndex);
    } else {
        const int nLast = mEntryCounts[mPanel] - 1;
        int nEntry = bUp ? mEntry - 1 : mEntry + 1;
        if (nEntry <= kNoIndex) {
            nEntry = nLast;
        }
        if (nLast < nEntry) {
            nEntry = 0;
        }
        SelectEntry(nEntry);
    }
}

bool RemixHUD::Confirm() {
    if (mEntry != kNoIndex) {
        if (!ConfirmEntry()) {
            return false;
        }
        OpenPanel(mPanel, false);
        return true;
    }
    if (mSubPanels[mPanel] != GfxManager::kRemixSubPanelMain) {
        OpenPanel(mPanel, true);
        return true;
    }

    int nEvent = kEventNone;
    switch (mPanelTypes[mPanel]) {
    case kPanelLoop:
        nEvent = mLoopOn ? kEventLoopOff : kEventLoopOn;
        break;
    case kPanelMute:
        if (mMode == kModeMute) {
            nEvent = mMuteOn ? kEventMuteOff : kEventMuteOn;
        }
        break;
    case kPanelChorus:
        nEvent = mChorusOn ? kEventChorusOff : kEventChorusOn;
        break;
    case kPanelStutter:
        nEvent = mStutterOn ? kEventStutterOff : kEventStutterOn;
        break;
    case kPanelEffect:
        nEvent = mEffectOn ? kEventEffectOff : kEventEffectOn;
        break;
    case kPanelSolo:
        nEvent = mSoloOn ? kEventSoloOff : kEventSoloOn;
        break;
    default:
        break;
    }
    if (mCallback != nullptr) {
        mCallback(mPlayer, nEvent, mCallbackData); // kEventNone is reported too.
    }
    return true;
}

bool RemixHUD::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPad != mPlayer) {
        return false;
    }
    switch (pMsg->mButton) {
    case kButtonUp:
    case kButtonDown:
        if (pMsg->mPressed) {
            Navigate(pMsg->mButton == kButtonUp, false);
        } else {
            TheSongScheduler.Cancel(mRepeatUpCmd.Get());
            TheSongScheduler.Cancel(mRepeatDownCmd.Get());
        }
        break;
    case kButtonConfirm:
        if (pMsg->mPressed) {
            if (Confirm()) {
                FxMidi::PlayMenuSelect();
            } else {
                FxMidi::PlaySound1();
            }
        }
        break;
    default:
        break;
    }
    return false;
}

bool RemixHUD::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() != g_nJoypadInputMsgType) {
        return false;
    }
    return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
}
