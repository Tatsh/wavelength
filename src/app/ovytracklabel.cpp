#include "app/ovytracklabel.h"

#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "os/locale.h"
#include "rnd/rndmanager.h"

namespace {

// The kinds of track that show a label.
constexpr int kLabelledKind = 2;
constexpr int kFirstLabelledKindOfRange = 4;
constexpr int kEndLabelledKindOfRange = 6;

} // namespace

OvyTrackLabel::OvyTrackLabel()
    : HideablePanel("HUD track label.tnm", "HUD track label.view", false) {
    mWanted = 0;
    mHasText = 0;
    mText = dynamic_cast<RndText *>(TheManager.Find("HUD track label.txt"));
    mMat = dynamic_cast<RndMat *>(TheManager.Find("HUD track label.mat"));
    for (mPlayer = 0; mPlayer < TheGameDb->GetNumPlayers(); ++mPlayer) {
        if (TheGameDb->IsLocalPlayer(mPlayer)) {
            break;
        }
    }
    (void)TheGameDb->GetNumPlayers(); // Yes, the binary discards this call's result.
    Hide();
}

void OvyTrackLabel::Hide() {
    mText->SetShowing(false);
    mHasText = 0;
    UpdateShown();
}

void OvyTrackLabel::SetTrack(int nPlayer, int nInstrument, int nTrackKind) {
    if (nPlayer != mPlayer) {
        return;
    }
    const char *pszLabel = nullptr;
    if (nTrackKind == kLabelledKind ||
        (nTrackKind >= kFirstLabelledKindOfRange && nTrackKind < kEndLabelledKindOfRange)) {
        switch (nInstrument) {
        case GfxManager::kInstrumentDrum:
            pszLabel = TheLocale.Localize("DRUM_LABEL", true);
            break;
        case GfxManager::kInstrumentBass:
            pszLabel = TheLocale.Localize("BASS_LABEL", true);
            break;
        case GfxManager::kInstrumentSynth:
            pszLabel = TheLocale.Localize("SYNTH_LABEL", true);
            break;
        case GfxManager::kInstrumentGuitar:
            pszLabel = TheLocale.Localize("GUITAR_LABEL", true);
            break;
        case GfxManager::kInstrumentVocal:
            pszLabel = TheLocale.Localize("VOCAL_LABEL", true);
            break;
        case GfxManager::kInstrumentFx:
            pszLabel = TheLocale.Localize("FX_LABEL", true);
            break;
        default:
            break;
        }
        mMat->SetBaseColor(*TheGfxManager.GetInstrumentBgColor(nInstrument));
    }
    if (pszLabel != nullptr) {
        mText->SetText(pszLabel);
        mText->SetShowing(true);
        mHasText = 1;
    } else {
        mText->SetShowing(false);
        mHasText = 0;
    }
    UpdateShown();
}

void OvyTrackLabel::UpdateShown() {
    HideablePanel::Show(mWanted != 0 && mHasText != 0);
}
