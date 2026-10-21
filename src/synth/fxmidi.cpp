#include "synth/fxmidi.h"

#include "game/gamedb.h"
#include "os/string.h"
#include "os/system.h"
#include "script/dataarray.h"
#include "synth/sfxbuilder.h"

namespace {

constexpr char kDbSection[] = "db";
#ifdef VIDEO_STANDARD_PAL
// The tag of the console language's interface sounds.
constexpr char kLanguageFxMidiFileTag[] = "%s_fx_midi_file";
#else
constexpr char kFxMidiFileTag[] = "fx_midi_file";
#endif

// The tick the interface scheduler starts at.
constexpr int kStartTick = 0;

// NTSC-U/C: 0x0027f8b8, PAL: 0x002891a0
const char *FindRequiredSymbol(DataArray *pData, const char *pszName) {
    const char *pszValue = nullptr;
    pData->FindSymbol(pszName, &pszValue, true);
    return pszValue;
}

void Play(Ptr<Muse> &cue) {
    cue->Play(&TheGameDb->GetSfxScheduler());
}

} // namespace

std::unique_ptr<FxMidi> TheFxMidi;

void GameDb::InitSfx() {
    DataArray *pDb = SystemConfig()->FindArray(kDbSection, true);
    TheFxMidi = std::make_unique<FxMidi>();
#ifdef VIDEO_STANDARD_PAL
    SFXBuilder builder(
        FindRequiredSymbol(pDb, FormatString(kLanguageFxMidiFileTag, GetSystemLanguage())));
#else
    SFXBuilder builder(FindRequiredSymbol(pDb, kFxMidiFileTag));
#endif
    builder.Read();
    mSfxScheduler.Reset(TheSfxTickDuration.get(), kStartTick);
    mSfxScheduler.Resume();
}

void GameDb::TerminateSfx() {
    mSfxScheduler.Pause();
    mSfxScheduler.Clear();
    TheSfxTickDuration.reset();
    TheFxMidi.reset();
}

void FxMidi::PlayMenuLeft() {
    Play(TheFxMidi->GetLeft());
}

void FxMidi::PlayMenuRight() {
    Play(TheFxMidi->GetRight());
}

void FxMidi::PlayMenuUp() {
    Play(TheFxMidi->GetUp());
}

void FxMidi::PlayMenuDown() {
    Play(TheFxMidi->GetDown());
}

void FxMidi::PlayMenuSelect() {
    Play(TheFxMidi->GetSelect());
}

void FxMidi::PlayBack() {
    Play(TheFxMidi->GetBack());
}

void FxMidi::PlayCheat() {
    Play(TheFxMidi->GetCheat());
}

void FxMidi::PlayProjector() {
    Play(TheFxMidi->GetProjector());
}

void FxMidi::PlaySound0() {
    Play(TheFxMidi->GetMiss());
}

void FxMidi::PlaySound1() {
    Play(TheFxMidi->GetUncatchable());
}

void FxMidi::PlaySound2() {
    Play(TheFxMidi->GetCheckpointCheer());
}

void FxMidi::PlaySound3() {
    Play(TheFxMidi->GetCheckpointInsane());
}

void FxMidi::PlayWinSound() {
    Play(TheFxMidi->GetWinCheer());
}

void FxMidi::StopLoop() {
    TheFxMidi->GetWinCheer()->Stop();
}

void FxMidi::PlayDecrypt() {
    Play(TheFxMidi->GetDecrypt());
}

void FxMidi::PlayCursorLoop() {
    Play(TheFxMidi->GetText());
}

void FxMidi::StopCursorLoop() {
    TheFxMidi->GetText()->Stop();
}

void FxMidi::PlayJuiceLowSound() {
    Play(TheFxMidi->GetWarning());
}

void FxMidi::PlayUnlock() {
    Play(TheFxMidi->GetUnlock());
}

void FxMidi::PlayPowerupCatchSound(int nPowerup) {
    Play(TheFxMidi->GetPowerupCatches(nPowerup));
}

void FxMidi::PlayPowerupSound(int nPowerup) {
    Play(TheFxMidi->GetPowerupDeploys(nPowerup));
}

void FxMidi::PlayGuideSound(int nLane) {
    Play(TheFxMidi->GetGuideTicks(nLane));
}

bool FxMidi::IsLeaderSoundPlaying() {
    for (int i = 0; i < kColorCount; ++i) {
        if (TheFxMidi->GetLeads(i)->IsPlaying()) {
            return true;
        }
    }
    return false;
}

void FxMidi::PlayLeaderSound(int nSlot) {
    Play(TheFxMidi->GetLeads(nSlot));
}

void FxMidi::PlayWinnerSound(int nSlot) {
    Play(TheFxMidi->GetWins(nSlot));
}

void FxMidi::PlayDuelLayPattern() {
    Play(TheFxMidi->GetDuelLayPattern());
}

void FxMidi::PlayDuelCatchPattern() {
    Play(TheFxMidi->GetDuelCatchPattern());
}

void FxMidi::PlayDuelNice() {
    Play(TheFxMidi->GetDuelNice());
}

void FxMidi::PlayDuelYouGotIt() {
    Play(TheFxMidi->GetDuelYouGotIt());
}

void FxMidi::PlayDuelAlmost() {
    Play(TheFxMidi->GetDuelAlmost());
}

void FxMidi::PlayDuelOneLetter() {
    Play(TheFxMidi->GetDuelOneLetter());
}

void FxMidi::PlayDuelAww() {
    Play(TheFxMidi->GetDuelAww());
}

void FxMidi::PlayDuelPerfect() {
    Play(TheFxMidi->GetDuelPerfect());
}

void FxMidi::PlayDuelGameTie() {
    Play(TheFxMidi->GetDuelGameTie());
}

void FxMidi::PlayDuelGreenWins() {
    Play(TheFxMidi->GetDuelGreenWins());
}

void FxMidi::PlayDuelPurpleWins() {
    Play(TheFxMidi->GetDuelPurpleWins());
}

void FxMidi::PlayDuelMiss() {
    Play(TheFxMidi->GetDuelMiss());
}

void FxMidi::PlayDuelMissThis() {
    Play(TheFxMidi->GetDuelMissThis());
}

void FxMidi::PlayDuelCheer() {
    Play(TheFxMidi->GetDuelCheer());
}

void FxMidi::PlayEraseSound() {
    Play(TheFxMidi->GetErase());
}

void FxMidi::PlayEraseSectionSound() {
    Play(TheFxMidi->GetEraseSection());
}

void FxMidi::PlayWrong() {
    Play(TheFxMidi->GetWrong());
}

void FxMidi::PlaySquare() {
    Play(TheFxMidi->GetSquare());
}

void FxMidi::PlayLazySusan() {
    Play(TheFxMidi->GetLazySusan());
}

void FxMidi::PlayKeyboardLeftUp() {
    Play(TheFxMidi->GetKeyboardLeftUp());
}

void FxMidi::PlayKeyboardBack() {
    Play(TheFxMidi->GetKeyboardBack());
}

void FxMidi::PlayKeyboardKeyEnter() {
    Play(TheFxMidi->GetKeyboardKeyEnter());
}

void FxMidi::PlayArenaUnlock() {
    Play(TheFxMidi->GetArenaUnlock());
}

void FxMidi::PlaySoloPortal() {
    Play(TheFxMidi->GetSoloPortal());
}

void FxMidi::PlayMultiPortal() {
    Play(TheFxMidi->GetMultiPortal());
}

void FxMidi::PlayNetPortal() {
    Play(TheFxMidi->GetNetPortal());
}

void FxMidi::StopPortals() {
    TheFxMidi->GetSoloPortal()->Stop();
    TheFxMidi->GetMultiPortal()->Stop();
    TheFxMidi->GetNetPortal()->Stop();
}

void FxMidi::PlayTransition() {
    Play(TheFxMidi->GetTravelSwoosh());
}

void FxMidi::StopTransition() {
    TheFxMidi->GetTravelSwoosh()->Stop();
}
