#include "game/voxtrack.h"

#include "game/gamedb.h"
#include "game/player.h"
#include "gfx/gfxmanager.h"
#include "synth/fxmidi.h"

namespace {

constexpr int kNoPattern = -1;
constexpr int kDefaultStyle = 0;
constexpr int kSavedBytesPerSection = 1;

} // namespace

VoxTrack::VoxTrack(CatchTrackData *pGems,
                   PlayMap *pPlayMap,
                   SectionBoundaries *pSections,
                   SlotGrid *pPatternGrid,
                   int nTrack,
                   int nLeadInBars,
                   int nNumBars,
                   int nTicksPerBar)
    : RemixTrack(nTrack), mGems(pGems),
      mDisplay(nTrack, nLeadInBars, nTicksPerBar, pPlayMap, pGems),
      mMusic(pGems, pPlayMap, nTicksPerBar, nNumBars), mMuteRemote(0), mActive(0),
      mSectionMuted(pSections->NumSections(), false), mSection(0), mSections(pSections),
      mPatternGrid(pPatternGrid) {
}

VoxTrack::~VoxTrack() {
}

void VoxTrack::Start() {
    UpdateMute();
    mDisplay.Start();
    mMusic.Start();
}

void VoxTrack::Stop() {
    mDisplay.Stop();
    mMusic.Stop();
}

void VoxTrack::SetPlayer(Player *pPlayer) {
    Track::SetPlayer(pPlayer);
    if (pPlayer != nullptr) {
        TheGfxManager.ShowPhrase(
            pPlayer->GetIndex(), mIndex, false, 0.0f, 0.0f, kDefaultStyle, true);
    }
    if (mMuteRemote != 0) {
        UpdateMute();
    }
}

void VoxTrack::SetMuteRemote(bool bMuteRemote) {
    mMuteRemote = bMuteRemote;
    UpdateMute();
}

bool VoxTrack::GetMuteRemote() const {
    return mMuteRemote != 0;
}

void VoxTrack::SetSection(int nSection) {
    mSection = nSection;
    UpdateMute();
}

void VoxTrack::SetActive(bool bActive) {
    mActive = bActive;
    UpdateMute();
}

bool VoxTrack::IsActive() const {
    return mActive != 0;
}

void VoxTrack::SetSectionMuted(int nSection, bool bMuted) {
    mSectionMuted[nSection] = bMuted;
    if (nSection == mSection) {
        UpdateMute();
    }
}

bool VoxTrack::IsSectionMuted(int nSection) const {
    return mSectionMuted[nSection];
}

void VoxTrack::SkipBars(int nStartBar, int nEndBar) {
    mDisplay.BlankBars(nStartBar, nEndBar);
}

void VoxTrack::RebuildGems() {
    const int nNumSections = mSections->NumSections();
    for (int nSection = 0; nSection < nNumSections; ++nSection) {
        const int nPattern = mPatternGrid->GetPattern(nSection);
        if (nPattern != kNoPattern) {
            mSectionMuted[nSection] = mSectionMuted[nPattern];
        }
    }
}

void VoxTrack::Redraw(int nStartBar) {
    mDisplay.Redraw(nStartBar);
}

void VoxTrack::HandleInput([[maybe_unused]] Player *pPlayer, const PlayNoteEvent &event) {
    if (event.mState != 0) {
        FxMidi::PlaySound1();
    }
}

void VoxTrack::HandleInput([[maybe_unused]] Player *pPlayer,
                           [[maybe_unused]] const BtnEvent<8> &event) {
    FxMidi::PlaySound1();
}

void VoxTrack::Save(BinStream &stream) {
    for (unsigned int i = 0; i < mSectionMuted.size(); ++i) {
        const bool bMuted = mSectionMuted[i];
        stream.Write(&bMuted, kSavedBytesPerSection);
    }
}

void VoxTrack::Load(BinStream &stream) {
    for (unsigned int i = 0; i < mSectionMuted.size(); ++i) {
        bool bMuted = false;
        stream.Read(&bMuted, kSavedBytesPerSection);
        mSectionMuted[i] = bMuted;
    }
}

void VoxTrack::UpdateMute() {
    bool bRemote = false;
    if (mMuteRemote != 0 &&
        (mPlayer == nullptr || !TheGameDb->IsLocalPlayer(mPlayer->GetIndex()))) {
        bRemote = true;
    }
    mMusic.SetMuted(mActive != 0 || bRemote || mSectionMuted[mSection]);
}
