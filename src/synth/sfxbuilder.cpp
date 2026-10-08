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

float *TheSfxTickDuration;

SFXBuilder::~SFXBuilder() {
    delete mBuilder;
}

void SFXBuilder::Read() {
    MidiReader reader(mFile, this);
    reader.ReadAll();
}

void SFXBuilder::OnEndTrack() {
    if (mBuilder != nullptr) {
        mBuilder->OnEndTrack();
        delete mBuilder;
    }
    mBuilder = nullptr;
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
    TheSfxTickDuration = new float;
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
        {"MISS", [](FxMidi *pFx) { return &pFx->mMiss; }},
        {"UNCATCHABLE", [](FxMidi *pFx) { return &pFx->mUncatchable; }},
        {"CHECKPOINT_CHEER", [](FxMidi *pFx) { return &pFx->mCheckpointCheer; }},
        {"CHECKPOINT_INSANE", [](FxMidi *pFx) { return &pFx->mCheckpointInsane; }},
        {"WIN_CHEER", [](FxMidi *pFx) { return &pFx->mWinCheer; }},
        {"WARNING", [](FxMidi *pFx) { return &pFx->mWarning; }},
        {"GUIDETICK1", [](FxMidi *pFx) { return &pFx->mGuideTicks[0]; }},
        {"GUIDETICK2", [](FxMidi *pFx) { return &pFx->mGuideTicks[1]; }},
        {"GUIDETICK3", [](FxMidi *pFx) { return &pFx->mGuideTicks[2]; }},
        {"AUTOCATCHER_CATCH",
         [](FxMidi *pFx) { return &pFx->mPowerupCatches[FxMidi::kPowerupAutocatcher]; }},
        {"FREESTYLER_CATCH",
         [](FxMidi *pFx) { return &pFx->mPowerupCatches[FxMidi::kPowerupFreestyler]; }},
        {"MULTIPLIER_CATCH",
         [](FxMidi *pFx) { return &pFx->mPowerupCatches[FxMidi::kPowerupMultiplier]; }},
        {"BUMPER_CATCH", [](FxMidi *pFx) { return &pFx->mPowerupCatches[FxMidi::kPowerupBumper]; }},
        {"CRIPPLER_CATCH",
         [](FxMidi *pFx) { return &pFx->mPowerupCatches[FxMidi::kPowerupCrippler]; }},
        {"SLOWDOWN_CATCH",
         [](FxMidi *pFx) { return &pFx->mPowerupCatches[FxMidi::kPowerupSlowdown]; }},
        {"AUTOCATCHER_DEPLOY",
         [](FxMidi *pFx) { return &pFx->mPowerupDeploys[FxMidi::kPowerupAutocatcher]; }},
        {"FREESTYLER_DEPLOY",
         [](FxMidi *pFx) { return &pFx->mPowerupDeploys[FxMidi::kPowerupFreestyler]; }},
        {"MULTIPLIER_DEPLOY",
         [](FxMidi *pFx) { return &pFx->mPowerupDeploys[FxMidi::kPowerupMultiplier]; }},
        {"BUMPER_DEPLOY",
         [](FxMidi *pFx) { return &pFx->mPowerupDeploys[FxMidi::kPowerupBumper]; }},
        {"CRIPPLER_DEPLOY",
         [](FxMidi *pFx) { return &pFx->mPowerupDeploys[FxMidi::kPowerupCrippler]; }},
        {"SLOWDOWN_DEPLOY",
         [](FxMidi *pFx) { return &pFx->mPowerupDeploys[FxMidi::kPowerupSlowdown]; }},
        {"WRONG", [](FxMidi *pFx) { return &pFx->mWrong; }},
        {"SQUARE", [](FxMidi *pFx) { return &pFx->mSquare; }},
        {"LAZYSUSAN", [](FxMidi *pFx) { return &pFx->mLazySusan; }},
        {"SOLOPORTAL", [](FxMidi *pFx) { return &pFx->mSoloPortal; }},
        {"MULTIPORTAL", [](FxMidi *pFx) { return &pFx->mMultiPortal; }},
        {"NETPORTAL", [](FxMidi *pFx) { return &pFx->mNetPortal; }},
        {"TRAVELSWOOSH", [](FxMidi *pFx) { return &pFx->mTravelSwoosh; }},
        {"LEFT", [](FxMidi *pFx) { return &pFx->mLeft; }},
        {"RIGHT", [](FxMidi *pFx) { return &pFx->mRight; }},
        {"UP", [](FxMidi *pFx) { return &pFx->mUp; }},
        {"DOWN", [](FxMidi *pFx) { return &pFx->mDown; }},
        {"SELECT", [](FxMidi *pFx) { return &pFx->mSelect; }},
        {"BACK", [](FxMidi *pFx) { return &pFx->mBack; }},
        {"CHEAT", [](FxMidi *pFx) { return &pFx->mCheat; }},
        {"PROJECTOR", [](FxMidi *pFx) { return &pFx->mProjector; }},
        {"UNLOCK", [](FxMidi *pFx) { return &pFx->mUnlock; }},
        {"REDLEAD", [](FxMidi *pFx) { return &pFx->mLeads[FxMidi::kColorRed]; }},
        {"REDWINS", [](FxMidi *pFx) { return &pFx->mWins[FxMidi::kColorRed]; }},
        {"GREENLEAD", [](FxMidi *pFx) { return &pFx->mLeads[FxMidi::kColorGreen]; }},
        {"GREENWINS", [](FxMidi *pFx) { return &pFx->mWins[FxMidi::kColorGreen]; }},
        {"PURPLELEAD", [](FxMidi *pFx) { return &pFx->mLeads[FxMidi::kColorPurple]; }},
        {"PURPLEWINS", [](FxMidi *pFx) { return &pFx->mWins[FxMidi::kColorPurple]; }},
        {"YELLOWLEAD", [](FxMidi *pFx) { return &pFx->mLeads[FxMidi::kColorYellow]; }},
        {"YELLOWWINS", [](FxMidi *pFx) { return &pFx->mWins[FxMidi::kColorYellow]; }},
        {"DUEL_PLAYER1", [](FxMidi *pFx) { return &pFx->mDuelPlayers[0]; }},
        {"DUEL_PLAYER2", [](FxMidi *pFx) { return &pFx->mDuelPlayers[1]; }},
        {"KEYBOARD_LEFT_UP", [](FxMidi *pFx) { return &pFx->mKeyboardLeftUp; }},
        {"KEYBOARD_RIGHT_DOWN", [](FxMidi *pFx) { return &pFx->mKeyboardRightDown; }},
        {"KEYBOARD_CIRCLE", [](FxMidi *pFx) { return &pFx->mKeyboardCircle; }},
        {"KEYBOARD_BACK", [](FxMidi *pFx) { return &pFx->mKeyboardBack; }},
        {"KEYBOARD_KEYENTER", [](FxMidi *pFx) { return &pFx->mKeyboardKeyEnter; }},
        {"ARENA_UNLOCK", [](FxMidi *pFx) { return &pFx->mArenaUnlock; }},
        {"DUEL_LAYPATT", [](FxMidi *pFx) { return &pFx->mDuelLayPattern; }},
        {"DUEL_CATCHPATT", [](FxMidi *pFx) { return &pFx->mDuelCatchPattern; }},
        {"DUEL_NICE", [](FxMidi *pFx) { return &pFx->mDuelNice; }},
        {"DUEL_YOUGOTIT", [](FxMidi *pFx) { return &pFx->mDuelYouGotIt; }},
        {"DUEL_ALMOST", [](FxMidi *pFx) { return &pFx->mDuelAlmost; }},
        {"DUEL_ONELETTER", [](FxMidi *pFx) { return &pFx->mDuelOneLetter; }},
        {"DUEL_AWW", [](FxMidi *pFx) { return &pFx->mDuelAww; }},
        {"DUEL_PERFECT", [](FxMidi *pFx) { return &pFx->mDuelPerfect; }},
        {"DUEL_YEAH", [](FxMidi *pFx) { return &pFx->mDuelYeah; }},
        {"DECRYPT", [](FxMidi *pFx) { return &pFx->mDecrypt; }},
        {"TEXT", [](FxMidi *pFx) { return &pFx->mText; }},
        {"DUEL_GAMETIE", [](FxMidi *pFx) { return &pFx->mDuelGameTie; }},
        {"DUEL_GREENWINS", [](FxMidi *pFx) { return &pFx->mDuelGreenWins; }},
        {"DUEL_PURPLEWINS", [](FxMidi *pFx) { return &pFx->mDuelPurpleWins; }},
        {"DUEL_MISS", [](FxMidi *pFx) { return &pFx->mDuelMiss; }},
        {"DUEL_MISSTHIS", [](FxMidi *pFx) { return &pFx->mDuelMissThis; }},
        {"DUEL_CHEER", [](FxMidi *pFx) { return &pFx->mDuelCheer; }},
        {"ERASE1", [](FxMidi *pFx) { return &pFx->mErase; }},
        {"ERASE2", [](FxMidi *pFx) { return &pFx->mEraseSection; }},
    };

    // TheFxMidi is read only when a name matches, as in the binary's comparison chain.
    Ptr<Muse> *pCue = nullptr;
    bool bFound = false;
    for (const auto &cue : cues) {
        if (name == cue.pszName) {
            pCue = cue.pfnCue(TheFxMidi);
            bFound = true;
            break;
        }
    }
    if (!bFound) {
        DebugError("unknown sound effect: %s\n", name.c_str());
    }
    if (pCue != nullptr) {
        mBuilder = new MuseBuilder(0, false, nullptr, pCue);
    }
}
