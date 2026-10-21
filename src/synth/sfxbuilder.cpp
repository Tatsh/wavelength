#include "synth/sfxbuilder.h"

#include <algorithm>
#include <cctype>

#include "mid/midireader.h"
#include "os/debug.h"
#include "os/string.h"
#include "synth/fxmidi.h"

namespace {

// The meta event type of a track name.
constexpr unsigned char kTrackNameMeta = 3;

// The ticks of one beat, and the microseconds of one millisecond.
constexpr float kTicksPerBeat = 480.0f;
constexpr float kMicrosecondsPerMillisecond = 1000.0f;

} // namespace

std::unique_ptr<float> TheSfxTickDuration;

SFXBuilder::~SFXBuilder() = default;

void SFXBuilder::Read() {
    MidiReader reader(mFile, this);
    reader.ReadAll();
}

void SFXBuilder::OnEndTrack() {
    if (mBuilder != nullptr) {
        mBuilder->OnEndTrack();
        mBuilder.reset();
    }
    ++mTrack;
}

void SFXBuilder::OnMidi(int nTick,
                        unsigned char nStatus,
                        unsigned char nData1,
                        unsigned char nData2) {
    if (mBuilder != nullptr) {
        mBuilder->OnMidi(nTick, nStatus, nData1, nData2);
    }
}

void SFXBuilder::OnTempo([[maybe_unused]] int nTick, int nMicrosecondsPerBeat) {
    TheSfxTickDuration = std::make_unique<float>();
    *TheSfxTickDuration =
        static_cast<float>(nMicrosecondsPerBeat) / kMicrosecondsPerMillisecond / kTicksPerBeat;
}

void SFXBuilder::OnText([[maybe_unused]] int nTick, const char *pszText, unsigned char nType) {
    if (mTrack == 0 || nType != kTrackNameMeta) {
        return;
    }
    String name(pszText);
    std::transform(name.mBuffer, name.mBuffer + name.mLength, name.mBuffer, toupper);

    const struct {
        const char *pszName;
        Ptr<Muse> *(*pfnCue)(FxMidi *pFx);
    } cues[] = {
        {"MISS", [](FxMidi *pFx) { return &pFx->GetMiss(); }},
        {"UNCATCHABLE", [](FxMidi *pFx) { return &pFx->GetUncatchable(); }},
        {"CHECKPOINT_CHEER", [](FxMidi *pFx) { return &pFx->GetCheckpointCheer(); }},
        {"CHECKPOINT_INSANE", [](FxMidi *pFx) { return &pFx->GetCheckpointInsane(); }},
        {"WIN_CHEER", [](FxMidi *pFx) { return &pFx->GetWinCheer(); }},
        {"WARNING", [](FxMidi *pFx) { return &pFx->GetWarning(); }},
        {"GUIDETICK1", [](FxMidi *pFx) { return &pFx->GetGuideTicks(0); }},
        {"GUIDETICK2", [](FxMidi *pFx) { return &pFx->GetGuideTicks(1); }},
        {"GUIDETICK3", [](FxMidi *pFx) { return &pFx->GetGuideTicks(2); }},
        {"AUTOCATCHER_CATCH",
         [](FxMidi *pFx) { return &pFx->GetPowerupCatches(FxMidi::kPowerupAutocatcher); }},
        {"FREESTYLER_CATCH",
         [](FxMidi *pFx) { return &pFx->GetPowerupCatches(FxMidi::kPowerupFreestyler); }},
        {"MULTIPLIER_CATCH",
         [](FxMidi *pFx) { return &pFx->GetPowerupCatches(FxMidi::kPowerupMultiplier); }},
        {"BUMPER_CATCH",
         [](FxMidi *pFx) { return &pFx->GetPowerupCatches(FxMidi::kPowerupBumper); }},
        {"CRIPPLER_CATCH",
         [](FxMidi *pFx) { return &pFx->GetPowerupCatches(FxMidi::kPowerupCrippler); }},
        {"SLOWDOWN_CATCH",
         [](FxMidi *pFx) { return &pFx->GetPowerupCatches(FxMidi::kPowerupSlowdown); }},
        {"AUTOCATCHER_DEPLOY",
         [](FxMidi *pFx) { return &pFx->GetPowerupDeploys(FxMidi::kPowerupAutocatcher); }},
        {"FREESTYLER_DEPLOY",
         [](FxMidi *pFx) { return &pFx->GetPowerupDeploys(FxMidi::kPowerupFreestyler); }},
        {"MULTIPLIER_DEPLOY",
         [](FxMidi *pFx) { return &pFx->GetPowerupDeploys(FxMidi::kPowerupMultiplier); }},
        {"BUMPER_DEPLOY",
         [](FxMidi *pFx) { return &pFx->GetPowerupDeploys(FxMidi::kPowerupBumper); }},
        {"CRIPPLER_DEPLOY",
         [](FxMidi *pFx) { return &pFx->GetPowerupDeploys(FxMidi::kPowerupCrippler); }},
        {"SLOWDOWN_DEPLOY",
         [](FxMidi *pFx) { return &pFx->GetPowerupDeploys(FxMidi::kPowerupSlowdown); }},
        {"WRONG", [](FxMidi *pFx) { return &pFx->GetWrong(); }},
        {"SQUARE", [](FxMidi *pFx) { return &pFx->GetSquare(); }},
        {"LAZYSUSAN", [](FxMidi *pFx) { return &pFx->GetLazySusan(); }},
        {"SOLOPORTAL", [](FxMidi *pFx) { return &pFx->GetSoloPortal(); }},
        {"MULTIPORTAL", [](FxMidi *pFx) { return &pFx->GetMultiPortal(); }},
        {"NETPORTAL", [](FxMidi *pFx) { return &pFx->GetNetPortal(); }},
        {"TRAVELSWOOSH", [](FxMidi *pFx) { return &pFx->GetTravelSwoosh(); }},
        {"LEFT", [](FxMidi *pFx) { return &pFx->GetLeft(); }},
        {"RIGHT", [](FxMidi *pFx) { return &pFx->GetRight(); }},
        {"UP", [](FxMidi *pFx) { return &pFx->GetUp(); }},
        {"DOWN", [](FxMidi *pFx) { return &pFx->GetDown(); }},
        {"SELECT", [](FxMidi *pFx) { return &pFx->GetSelect(); }},
        {"BACK", [](FxMidi *pFx) { return &pFx->GetBack(); }},
        {"CHEAT", [](FxMidi *pFx) { return &pFx->GetCheat(); }},
        {"PROJECTOR", [](FxMidi *pFx) { return &pFx->GetProjector(); }},
        {"UNLOCK", [](FxMidi *pFx) { return &pFx->GetUnlock(); }},
        {"REDLEAD", [](FxMidi *pFx) { return &pFx->GetLeads(FxMidi::kColorRed); }},
        {"REDWINS", [](FxMidi *pFx) { return &pFx->GetWins(FxMidi::kColorRed); }},
        {"GREENLEAD", [](FxMidi *pFx) { return &pFx->GetLeads(FxMidi::kColorGreen); }},
        {"GREENWINS", [](FxMidi *pFx) { return &pFx->GetWins(FxMidi::kColorGreen); }},
        {"PURPLELEAD", [](FxMidi *pFx) { return &pFx->GetLeads(FxMidi::kColorPurple); }},
        {"PURPLEWINS", [](FxMidi *pFx) { return &pFx->GetWins(FxMidi::kColorPurple); }},
        {"YELLOWLEAD", [](FxMidi *pFx) { return &pFx->GetLeads(FxMidi::kColorYellow); }},
        {"YELLOWWINS", [](FxMidi *pFx) { return &pFx->GetWins(FxMidi::kColorYellow); }},
        {"DUEL_PLAYER1", [](FxMidi *pFx) { return &pFx->GetDuelPlayers(0); }},
        {"DUEL_PLAYER2", [](FxMidi *pFx) { return &pFx->GetDuelPlayers(1); }},
        {"KEYBOARD_LEFT_UP", [](FxMidi *pFx) { return &pFx->GetKeyboardLeftUp(); }},
        {"KEYBOARD_RIGHT_DOWN", [](FxMidi *pFx) { return &pFx->GetKeyboardRightDown(); }},
        {"KEYBOARD_CIRCLE", [](FxMidi *pFx) { return &pFx->GetKeyboardCircle(); }},
        {"KEYBOARD_BACK", [](FxMidi *pFx) { return &pFx->GetKeyboardBack(); }},
        {"KEYBOARD_KEYENTER", [](FxMidi *pFx) { return &pFx->GetKeyboardKeyEnter(); }},
        {"ARENA_UNLOCK", [](FxMidi *pFx) { return &pFx->GetArenaUnlock(); }},
        {"DUEL_LAYPATT", [](FxMidi *pFx) { return &pFx->GetDuelLayPattern(); }},
        {"DUEL_CATCHPATT", [](FxMidi *pFx) { return &pFx->GetDuelCatchPattern(); }},
        {"DUEL_NICE", [](FxMidi *pFx) { return &pFx->GetDuelNice(); }},
        {"DUEL_YOUGOTIT", [](FxMidi *pFx) { return &pFx->GetDuelYouGotIt(); }},
        {"DUEL_ALMOST", [](FxMidi *pFx) { return &pFx->GetDuelAlmost(); }},
        {"DUEL_ONELETTER", [](FxMidi *pFx) { return &pFx->GetDuelOneLetter(); }},
        {"DUEL_AWW", [](FxMidi *pFx) { return &pFx->GetDuelAww(); }},
        {"DUEL_PERFECT", [](FxMidi *pFx) { return &pFx->GetDuelPerfect(); }},
        {"DUEL_YEAH", [](FxMidi *pFx) { return &pFx->GetDuelYeah(); }},
        {"DECRYPT", [](FxMidi *pFx) { return &pFx->GetDecrypt(); }},
        {"TEXT", [](FxMidi *pFx) { return &pFx->GetText(); }},
        {"DUEL_GAMETIE", [](FxMidi *pFx) { return &pFx->GetDuelGameTie(); }},
        {"DUEL_GREENWINS", [](FxMidi *pFx) { return &pFx->GetDuelGreenWins(); }},
        {"DUEL_PURPLEWINS", [](FxMidi *pFx) { return &pFx->GetDuelPurpleWins(); }},
        {"DUEL_MISS", [](FxMidi *pFx) { return &pFx->GetDuelMiss(); }},
        {"DUEL_MISSTHIS", [](FxMidi *pFx) { return &pFx->GetDuelMissThis(); }},
        {"DUEL_CHEER", [](FxMidi *pFx) { return &pFx->GetDuelCheer(); }},
        {"ERASE1", [](FxMidi *pFx) { return &pFx->GetErase(); }},
        {"ERASE2", [](FxMidi *pFx) { return &pFx->GetEraseSection(); }},
    };

    // TheFxMidi is read only when a name matches, as in the binary's comparison chain.
    Ptr<Muse> *pCue = nullptr;
    bool bFound = false;
    for (const auto &cue : cues) {
        if (name == cue.pszName) {
            pCue = cue.pfnCue(TheFxMidi.get());
            bFound = true;
            break;
        }
    }
    if (!bFound) {
        DebugError("unknown sound effect: %s\n", name.c_str());
    }
    if (pCue != nullptr) {
        mBuilder = std::make_unique<MuseBuilder>(0, false, nullptr, pCue);
    }
}
