#include "game/duel.h"

#include <vector>

#include "game/banktrack.h"
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

constexpr int kDuelPlayerCount = 2;

// The value Duel passes for each player in both track lists. Its meaning is not yet recovered.
constexpr int kDuelTrackListValue = 5;

constexpr int kDuelTrackFlagCount = 4;

constexpr int kDuelTrackOption = 0;

} // namespace

Duel::Duel() {
    mFxBankSlot = kNoFxBankSlot;
    mLogic = nullptr;
}

Duel::~Duel() {
}

void Duel::CreateLogic() {
    SetLogic(new DuelLogic(mSong));
}

void Duel::SetLogic(DuelLogic *pLogic) {
    World::SetLogic(pLogic);
    mLogic = pLogic;
}

void Duel::BeginSongLoad() {
    mSong->GetBankTrack()->Load();
}

bool Duel::IsSongLoadDone() {
    return mSong->GetBankTrack()->PollLoad();
}

void Duel::BeginAssetLoad() {
    Synth *pSynth = TheSynth;
    DataArray *pGame = SystemConfig()->FindArray(kGameSection, false);
    const char *pszBankFile = nullptr;
    (void)pGame->FindSymbol(kFxBankFileKey, &pszBankFile, true); // Yes, the result is discarded.
    (void)pGame->FindInt(kFxBankSlotKey, &mFxBankSlot, true);
    pSynth->UnloadBank(static_cast<unsigned short>(mFxBankSlot));
    pSynth->LoadBank(static_cast<unsigned short>(mFxBankSlot), String(pszBankFile), true);
}

bool Duel::IsAssetLoadDone() {
    return TheSynth->IsBankLoaded(static_cast<unsigned short>(mFxBankSlot));
}

void Duel::BeginEnding() {
    BankTrack *pBankTrack = mSong->GetBankTrack();
    pBankTrack->Stop();
    pBankTrack->Load();
}

void Duel::PollEnding() {
}

bool Duel::IsEndingDone() {
    return mSong->GetBankTrack()->PollLoad();
}

void Duel::OnStart() {
}

bool Duel::IsFinished() {
    return mLogic->IsFinished() && mLogic->HasQuit() == 0;
}

void Duel::BuildTracks(int nStartTick) {
    std::vector<int> firstList;
    std::vector<int> secondList;
    for (int i = 0; i < kDuelPlayerCount; ++i) {
        firstList.push_back(kDuelTrackListValue);
        secondList.push_back(kDuelTrackListValue);
    }

    const float fStartTick = static_cast<float>(nStartTick);
    const int nEndTick = mSong->mNumBars * mSong->mBuilder->mTicksPerBar;
    const float *pTickDuration = mSong->GetMsPerTick();
    const std::vector<bool> flags(kDuelTrackFlagCount, false);
    TheGfxManager.BuildTracks(fStartTick,
                              static_cast<float>(nEndTick),
                              pTickDuration,
                              firstList,
                              secondList,
                              kDuelTrackOption,
                              flags);
}

void Duel::Handle(const RotateEvent &event) {
    Record(event);
}

void Duel::Handle(const PlayNoteEvent &event) {
    Record(event);
}

void Duel::Handle(const StickEvent<2> &event) {
    Record(event);
}

void Duel::Handle(const StickEvent<6> &event) {
    Record(event);
}

void Duel::Handle(const BtnEvent<3> &event) {
    Record(event);
}

void Duel::Handle(const BtnEvent<4> &event) {
    Record(event);
}

void Duel::Handle(const BtnEvent<5> &event) {
    Record(event);
}
