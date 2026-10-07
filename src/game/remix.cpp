#include "game/remix.h"

#include <vector>

#include "gfx/gfxmanager.h"
#include "os/string.h"
#include "os/system.h"
#include "script/dataarray.h"
#include "synth/synth.h"

namespace {

constexpr char kGameSection[] = "game";
constexpr char kFxBankFileKey[] = "remix_fx_bank_file";
constexpr char kFxBankSlotKey[] = "game_fx_bank_slot";

constexpr int kNoFxBankSlot = -1;
constexpr int kReserved48Value = 2;

constexpr int kTrackFlagCount = 4;

} // namespace

Remix::Remix() : mReserved48(kReserved48Value), mLogic(nullptr), mFxBankSlot(kNoFxBankSlot) {
}

Remix::~Remix() {
    UnloadFxBank();
}

void Remix::CreateLogic() {
    mLogic = new RemixLogic(mSong);
    SetLogic(mLogic);
}

bool Remix::IsFinished() {
    return mLogic->IsFinished() && !mLogic->HasQuit();
}

void Remix::Handle(const RotateEvent &event) {
    Record(event);
}

void Remix::Handle(const PlayNoteEvent &event) {
    Record(event);
}

void Remix::Handle(const BtnEvent<8> &event) {
    Record(event);
}

void Remix::Handle(const StickEvent<2> &event) {
    Record(event);
}

void Remix::Handle([[maybe_unused]] const StickEvent<6> &event) {
}

void Remix::Handle(const BtnEvent<3> &event) {
    Record(event);
}

void Remix::Handle(const ChangeSectionEvent &event) {
    Record(event);
}

void Remix::Handle(const BtnEvent<9> &event) {
    Record(event);
}

void Remix::Handle(const BtnEvent<10> &event) {
    Record(event);
}

void Remix::BeginSongLoad() {
    mSong->GetBankTrack()->Load();
}

bool Remix::IsSongLoadDone() {
    return mSong->GetBankTrack()->PollLoad();
}

void Remix::BeginAssetLoad() {
    Synth *pSynth = TheSynth;
    DataArray *pGame = SystemConfig()->FindArray(kGameSection, false);
    const char *pszBankFile = nullptr;
    (void)pGame->FindSymbol(kFxBankFileKey, &pszBankFile, true); // Yes, the result is discarded.
    (void)pGame->FindInt(kFxBankSlotKey, &mFxBankSlot, true);
    pSynth->UnloadBank(static_cast<unsigned short>(mFxBankSlot));
    pSynth->LoadBank(static_cast<unsigned short>(mFxBankSlot), String(pszBankFile), true);
}

bool Remix::IsAssetLoadDone() {
    return TheSynth->IsBankLoaded(static_cast<unsigned short>(mFxBankSlot));
}

void Remix::UnloadFxBank() {
    TheSynth->UnloadBank(static_cast<unsigned short>(mFxBankSlot));
    mFxBankSlot = kNoFxBankSlot;
}

void Remix::BuildTracks(int nStartTick) {
    std::vector<int> instruments;
    std::vector<int> types;
    int nOption = 0;
    for (int nTrack = 0; nTrack < mSong->GetNumTracks(); ++nTrack) {
        const int nType = mSong->GetTrackType(nTrack);
        if (nType == Song::kTrackTypePitch || nType == Song::kTrackTypeVox) {
            instruments.push_back(mSong->GetTrackInstrument(nTrack));
            types.push_back(nType);
        } else if (nType == Song::kTrackTypeScratch || nType == Song::kTrackTypeAxe) {
            nOption = nType;
        }
    }

    const float *pTickDuration = mSong->GetMsPerTick();
    const std::vector<bool> flags(kTrackFlagCount, false);
    TheGfxManager.BuildTracks(
        static_cast<float>(nStartTick), 0.0f, pTickDuration, instruments, types, nOption, flags);
}
