#include "game/pitchtrack.h"

#include "game/gamecallback.h"
#include "game/gamedb.h"
#include "game/player.h"
#include "gfx/gfxmanager.h"
#include "msg/editgempacket.h"
#include "msg/erasebarpacket.h"
#include "msg/erasesectionpacket.h"
#include "netflow/nettransport.h"
#include "os/memfun1command.h"
#include "os/scheduler.h"
#include "os/system.h"
#include "synth/fxmidi.h"

namespace {

constexpr int kNoOwner = -1;
constexpr int kNoPattern = -1;

// A press snaps to the nearest sixteenth of a bar, measured from 8 ticks before the press.
constexpr int kStepsPerBar = 16;
constexpr int kPressLeadTicks = 8;

// A phrase spans two bars, starting on an even bar of its section.
constexpr int kPhraseBars = 2;

// A second erase within this time erases the whole section.
constexpr float kDoubleEraseMs = 500.0f;

bool IsOnline() {
    return TheGameDb->mCommunity == GameDb::kCommunityOnline;
}

} // namespace

PitchTrack::PitchTrack(int nTrack,
                       int nLeadInBars,
                       int nNumBars,
                       int nTicksPerBar,
                       PitchTrackRiffData *pRiffData,
                       const std::vector<PitchTrackGems *> *pPatterns,
                       PlayMap *pPlayMap,
                       SectionBoundaries *pSections,
                       SlotGrid *pPatternGrid,
                       bool bDisplayed)
    : RemixTrack(nTrack), mTicksPerBar(nTicksPerBar), mPlayMap(pPlayMap), mSections(pSections),
      mGems(pSections, pPatternGrid, nNumBars, nTicksPerBar),
      mPlayGems(&mGems, pPlayMap, pSections, pPatternGrid, nNumBars, nTicksPerBar), mPatterns(),
      mRiffs(pRiffData, pPlayMap), mMusic(&mPlayGems, &mRiffs, nTicksPerBar), mDisplay(nullptr),
      mRepeat(true), mEditing(false), mMuteRemote(false), mActive(false), mLastEraseMs(0.0f),
      mStartBar(0), mPreviewPattern(kNoPattern),
      mPhraseCommand(NewMemFun1Command(this, &PitchTrack::UpdatePhrase, false)) {
    if (bDisplayed) {
        mDisplay = new PitchTrackDisplay(
            nTrack, nLeadInBars, nTicksPerBar, pPlayMap, mSections, &mPlayGems);
    }
    for (unsigned int i = 0; i < pPatterns->size(); ++i) {
        mPatterns.push_back(new PitchTrackPlayGems(
            (*pPatterns)[i], pPlayMap, pSections, pPatternGrid, nNumBars, nTicksPerBar));
    }
}

PitchTrack::~PitchTrack() {
    Stop();
    for (auto *pPattern : mPatterns) {
        delete pPattern;
    }
    delete mDisplay;
}

void PitchTrack::SetPlayer(Player *pPlayer) {
    if (pPlayer == nullptr && mPlayer != nullptr) {
        ShowPhrase(false, true);
        EndPreview();
    }
    mLastEraseMs = 0.0f;
    Track::SetPlayer(pPlayer);
    if (mMuteRemote) {
        UpdateMute();
    }
    if (pPlayer != nullptr) {
        SetRepeat(pPlayer->GetRepeat());
    }
}

void PitchTrack::Start() {
    if (mDisplay != nullptr) {
        mDisplay->Start();
    }
    mMusic.Start();
    UpdatePhrase(true);
}

void PitchTrack::Stop() {
    if (mDisplay != nullptr) {
        mDisplay->Stop();
    }
    mMusic.Stop();
}

void PitchTrack::EndPreview() {
    if (mPreviewPattern != kNoPattern) {
        mPreviewPattern = kNoPattern;
        if (mDisplay != nullptr) {
            mDisplay->SetGems(&mPlayGems, 0, 0, kNoOwner);
        }
        mMusic.SetGems(&mPlayGems);
    }
}

void PitchTrack::SelectPattern(int nPattern, int nStartBar, int nEndBar) {
    mPlayGems.CopyBars(mPatterns[nPattern], nStartBar, nEndBar);
    mMusic.SetGems(&mPlayGems);
}

void PitchTrack::SetRepeat(bool bRepeat) {
    mRepeat = bRepeat;
    UpdatePhrase(true);
}

void PitchTrack::SetEditing(bool bEditing) {
    mEditing = bEditing;
    UpdatePhrase(true);
}

void PitchTrack::RebuildGems() {
    mPlayGems.ApplyPatterns();
}

void PitchTrack::SetMuteRemote(bool bMuteRemote) {
    mMuteRemote = bMuteRemote;
    UpdateMute();
}

bool PitchTrack::GetMuteRemote() const {
    return mMuteRemote;
}

void PitchTrack::SetActive(bool bActive) {
    mActive = bActive;
    UpdateMute();
}

bool PitchTrack::IsActive() const {
    return mActive;
}

void PitchTrack::SkipBars(int nStartBar, int nEndBar) {
    if (mDisplay != nullptr) {
        mDisplay->BlankBars(nStartBar, nEndBar);
    }
    mStartBar = nEndBar;
    UpdatePhrase(true);
}

void PitchTrack::Redraw(int nStartBar) {
    if (mDisplay != nullptr) {
        mDisplay->Redraw(nStartBar);
    }
}

bool PitchTrack::ToggleGem(signed char nSlot, int nTick, bool bRepeat, signed char nOwner) {
    const bool bPlaced = mPlayGems.ToggleGem(nSlot, nTick, nOwner, bRepeat);
    if (mDisplay != nullptr) {
        if (bPlaced) {
            mDisplay->ShowGem(nSlot, nTick, bRepeat, nOwner);
        } else {
            mDisplay->HideGem(nTick, bRepeat);
        }
    }
    return bPlaced;
}

void PitchTrack::EraseBar(int nBar) {
    mPlayGems.ClearBar(nBar, mRepeat);
    if (mDisplay != nullptr) {
        mDisplay->ClearBar(nBar, mRepeat);
    }
}

void PitchTrack::EraseSection(int nBar) {
    mPlayGems.ClearSection(nBar);
    if (mDisplay != nullptr) {
        mDisplay->ClearGems(nBar);
    }
}

void PitchTrack::HandleInput(Player *pPlayer, const PlayNoteEvent &event) {
    if (mPreviewPattern != kNoPattern || event.mState == 0 || pPlayer != mPlayer) {
        return;
    }

    const int nNow = TheSongScheduler.mTick;
    const int nPlayer = pPlayer->GetIndex();
    const int nOwner = mPlayGems.GetOwner(nNow / mTicksPerBar);
    if (nOwner != kNoOwner && nOwner != nPlayer) {
        FxMidi::PlaySound1();
        return;
    }

    const int nStep = mTicksPerBar / kStepsPerBar;
    const int nTick = ((nNow - kPressLeadTicks + nStep / 2) / nStep) * nStep;
    const signed char nSlot = static_cast<signed char>(event.mButton);
    const bool bPlaced = ToggleGem(nSlot, nTick, mRepeat, static_cast<signed char>(nPlayer));
    if (bPlaced) {
        Muse *pRiff = mRiffs.GetRiff(nSlot, nTick);
        mMusic.Play(pRiff, nNow < nTick ? nNow - nTick : 0);
    } else {
        FxMidi::PlayEraseSound();
    }

    if (IsOnline()) {
        EditGemPacket packet(mIndex, nSlot, nTick, mRepeat);
        TheNetTransport->Send(packet);
    }

    if (bPlaced) {
        TheGfxManager.HitGem(nPlayer, mIndex, nSlot, static_cast<float>(nTick));
    } else {
        TheGfxManager.ShowGemResult(mIndex, nSlot, false, nPlayer, 0, static_cast<float>(nTick));
    }
}

void PitchTrack::HandleInput(Player *pPlayer, [[maybe_unused]] const BtnEvent<8> &event) {
    if (mPreviewPattern != kNoPattern || pPlayer != mPlayer) {
        return;
    }

    const int nStep = mTicksPerBar / kStepsPerBar;
    const int nTick = ((TheSongScheduler.mTick - kPressLeadTicks + nStep / 2) / nStep) * nStep;
    const int nBar = nTick / mTicksPerBar;
    const int nOwner = mPlayGems.GetOwner(nBar);
    if (nOwner != kNoOwner && nOwner != pPlayer->GetIndex()) {
        FxMidi::PlaySound1();
        return;
    }

    const float fNowMs = SystemMs();
    if (fNowMs - mLastEraseMs < kDoubleEraseMs) {
        if (TheGameCallback != nullptr) {
            TheGameCallback->OnEraseSection();
        }
        FxMidi::PlayEraseSectionSound();
        EraseSection(nBar);
        if (IsOnline()) {
            EraseSectionPacket packet(mIndex, nBar);
            TheNetTransport->Send(packet);
        }
        // Yes, the binary reports the erased section a second time.
        if (TheGameCallback != nullptr) {
            TheGameCallback->OnEraseSection();
        }
    } else {
        FxMidi::PlayEraseSound();
        EraseBar(nBar);
        if (IsOnline()) {
            EraseBarPacket packet(mIndex, nBar);
            TheNetTransport->Send(packet);
        }
        if (TheGameCallback != nullptr) {
            TheGameCallback->OnErase();
        }
    }
    mLastEraseMs = fNowMs;
}

void PitchTrack::Save(BinStream &stream) {
    mPlayGems.Save(stream);
}

void PitchTrack::Load(BinStream &stream) {
    mPlayGems.Load(stream);
    if (mDisplay != nullptr) {
        mDisplay->Redraw(TheSongScheduler.mTick / mTicksPerBar);
    }
}

void PitchTrack::UpdatePhrase(bool bStyle) {
    if (mPlayer != nullptr) {
        ShowPhrase(mRepeat ? !mEditing : false, bStyle);
    }
}

void PitchTrack::ShowPhrase(bool bPlayable, bool bStyle) {
    const int nNowBar = TheSongScheduler.mTick / mTicksPerBar;
    const int nBar = mStartBar < nNowBar ? nNowBar : mStartBar;
    const int nSongBar = mPlayMap->MapBar(nBar);
    int nNextTick = 0;
    if (nSongBar < 0) {
        TheGfxManager.ShowPhrase(mPlayer->GetIndex(), mIndex, false, 0.0f, 0.0f, bStyle, !mEditing);
        nNextTick = (nBar + 1) * mTicksPerBar;
    } else {
        int nSectionStart = 0;
        int nSectionEnd = 0;
        mSections->GetSectionRange(mSections->SectionAt(nSongBar), &nSectionStart, &nSectionEnd);
        const int nPhraseTick = (nBar - (nSongBar - nSectionStart) % kPhraseBars) * mTicksPerBar;
        nNextTick = nPhraseTick + kPhraseBars * mTicksPerBar;
        const int nOwner = mPlayGems.GetOwner(nBar);
        const bool bOwned = nOwner == kNoOwner || nOwner == mPlayer->GetIndex();
        TheGfxManager.ShowPhrase(mPlayer->GetIndex(),
                                 mIndex,
                                 bPlayable,
                                 static_cast<float>(nPhraseTick),
                                 static_cast<float>(nNextTick - 1),
                                 bStyle,
                                 bOwned ? !mEditing : false);
    }
    TheSongScheduler.Cancel(mPhraseCommand.Get());
    TheSongScheduler.PostAt(mPhraseCommand.Get(), nNextTick, false);
}

void PitchTrack::UpdateMute() {
    bool bRemote = false;
    if (mMuteRemote && (mPlayer == nullptr || !TheGameDb->IsLocalPlayer(mPlayer->GetIndex()))) {
        bRemote = true;
    }
    mMusic.SetMuted(mActive || bRemote);
}
