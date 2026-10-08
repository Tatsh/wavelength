#include "synth/fxmidi.h"

#include "game/gamedb.h"
#include "os/system.h"
#include "script/dataarray.h"
#include "synth/sfxbuilder.h"

namespace {

constexpr char kDbSection[] = "db";
constexpr char kFxMidiFileTag[] = "fx_midi_file";

// The tick the interface scheduler starts at.
constexpr int kStartTick = 0;

// NTSC-U/C: 0x0027f8b8, PAL: 0x002891a0
const char *FindRequiredSymbol(DataArray *pData, const char *pszName) {
    const char *pszValue = nullptr;
    pData->FindSymbol(pszName, &pszValue, true);
    return pszValue;
}

void Play(Ptr<Muse> &cue) {
    cue->Play(&TheGameDb->mSfxScheduler);
}

} // namespace

FxMidi *TheFxMidi;

void GameDb::InitSfx() {
    DataArray *pDb = SystemConfig()->FindArray(kDbSection, true);
    TheFxMidi = new FxMidi;
    SFXBuilder builder(FindRequiredSymbol(pDb, kFxMidiFileTag));
    builder.Read();
    mSfxScheduler.Reset(TheSfxTickDuration, kStartTick);
    mSfxScheduler.Resume();
}

void GameDb::TerminateSfx() {
    mSfxScheduler.Pause();
    mSfxScheduler.Clear();
    delete TheSfxTickDuration;
    delete TheFxMidi;
    TheFxMidi = nullptr;
}

void FxMidi::PlayMenuLeft() {
    Play(TheFxMidi->mLeft);
}

void FxMidi::PlayMenuRight() {
    Play(TheFxMidi->mRight);
}

void FxMidi::PlayMenuUp() {
    Play(TheFxMidi->mUp);
}

void FxMidi::PlayMenuDown() {
    Play(TheFxMidi->mDown);
}

void FxMidi::PlayMenuSelect() {
    Play(TheFxMidi->mSelect);
}

void FxMidi::PlayBack() {
    Play(TheFxMidi->mBack);
}

void FxMidi::PlayCheat() {
    Play(TheFxMidi->mCheat);
}

void FxMidi::PlayProjector() {
    Play(TheFxMidi->mProjector);
}

void FxMidi::PlaySound0() {
    Play(TheFxMidi->mMiss);
}

void FxMidi::PlaySound1() {
    Play(TheFxMidi->mUncatchable);
}

void FxMidi::PlaySound2() {
    Play(TheFxMidi->mCheckpointCheer);
}

void FxMidi::PlaySound3() {
    Play(TheFxMidi->mCheckpointInsane);
}

void FxMidi::PlayWinSound() {
    Play(TheFxMidi->mWinCheer);
}

void FxMidi::StopLoop() {
    TheFxMidi->mWinCheer->Stop();
}

void FxMidi::PlayDecrypt() {
    Play(TheFxMidi->mDecrypt);
}

void FxMidi::PlayCursorLoop() {
    Play(TheFxMidi->mText);
}

void FxMidi::StopCursorLoop() {
    TheFxMidi->mText->Stop();
}

void FxMidi::PlayJuiceLowSound() {
    Play(TheFxMidi->mWarning);
}

void FxMidi::PlayUnlock() {
    Play(TheFxMidi->mUnlock);
}

void FxMidi::PlayPowerupCatchSound(int nPowerup) {
    Play(TheFxMidi->mPowerupCatches[nPowerup]);
}

void FxMidi::PlayPowerupSound(int nPowerup) {
    Play(TheFxMidi->mPowerupDeploys[nPowerup]);
}

void FxMidi::PlayGuideSound(int nLane) {
    Play(TheFxMidi->mGuideTicks[nLane]);
}

bool FxMidi::IsLeaderSoundPlaying() {
    for (int i = 0; i < kColorCount; ++i) {
        if (TheFxMidi->mLeads[i]->IsPlaying()) {
            return true;
        }
    }
    return false;
}

void FxMidi::PlayLeaderSound(int nSlot) {
    Play(TheFxMidi->mLeads[nSlot]);
}

void FxMidi::PlayWinnerSound(int nSlot) {
    Play(TheFxMidi->mWins[nSlot]);
}

void FxMidi::PlayDuelLayPattern() {
    Play(TheFxMidi->mDuelLayPattern);
}

void FxMidi::PlayDuelCatchPattern() {
    Play(TheFxMidi->mDuelCatchPattern);
}

void FxMidi::PlayDuelNice() {
    Play(TheFxMidi->mDuelNice);
}

void FxMidi::PlayDuelYouGotIt() {
    Play(TheFxMidi->mDuelYouGotIt);
}

void FxMidi::PlayDuelAlmost() {
    Play(TheFxMidi->mDuelAlmost);
}

void FxMidi::PlayDuelOneLetter() {
    Play(TheFxMidi->mDuelOneLetter);
}

void FxMidi::PlayDuelAww() {
    Play(TheFxMidi->mDuelAww);
}

void FxMidi::PlayDuelPerfect() {
    Play(TheFxMidi->mDuelPerfect);
}

void FxMidi::PlayDuelGameTie() {
    Play(TheFxMidi->mDuelGameTie);
}

void FxMidi::PlayDuelGreenWins() {
    Play(TheFxMidi->mDuelGreenWins);
}

void FxMidi::PlayDuelPurpleWins() {
    Play(TheFxMidi->mDuelPurpleWins);
}

void FxMidi::PlayDuelMiss() {
    Play(TheFxMidi->mDuelMiss);
}

void FxMidi::PlayDuelMissThis() {
    Play(TheFxMidi->mDuelMissThis);
}

void FxMidi::PlayDuelCheer() {
    Play(TheFxMidi->mDuelCheer);
}

void FxMidi::PlayEraseSound() {
    Play(TheFxMidi->mErase);
}

void FxMidi::PlayEraseSectionSound() {
    Play(TheFxMidi->mEraseSection);
}

void FxMidi::PlayWrong() {
    Play(TheFxMidi->mWrong);
}

void FxMidi::PlaySquare() {
    Play(TheFxMidi->mSquare);
}

void FxMidi::PlayLazySusan() {
    Play(TheFxMidi->mLazySusan);
}

void FxMidi::PlayKeyboardLeftUp() {
    Play(TheFxMidi->mKeyboardLeftUp);
}

void FxMidi::PlayKeyboardBack() {
    Play(TheFxMidi->mKeyboardBack);
}

void FxMidi::PlayKeyboardKeyEnter() {
    Play(TheFxMidi->mKeyboardKeyEnter);
}

void FxMidi::PlayArenaUnlock() {
    Play(TheFxMidi->mArenaUnlock);
}

void FxMidi::PlaySoloPortal() {
    Play(TheFxMidi->mSoloPortal);
}

void FxMidi::PlayMultiPortal() {
    Play(TheFxMidi->mMultiPortal);
}

void FxMidi::PlayNetPortal() {
    Play(TheFxMidi->mNetPortal);
}

void FxMidi::StopPortals() {
    TheFxMidi->mSoloPortal->Stop();
    TheFxMidi->mMultiPortal->Stop();
    TheFxMidi->mNetPortal->Stop();
}

void FxMidi::PlayTransition() {
    Play(TheFxMidi->mTravelSwoosh);
}

void FxMidi::StopTransition() {
    TheFxMidi->mTravelSwoosh->Stop();
}
