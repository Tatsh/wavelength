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
    TheSfxTickDuration = new float(static_cast<float>(nMicrosecondsPerBeat) /
                                   kMicrosecondsPerMillisecond / kTicksPerBeat);
}

void SFXBuilder::OnText([[maybe_unused]] int nTick, const char *pszText, unsigned char nType) {
    if (mTrack == 0 || nType != kTrackNameMeta) {
        return;
    }
    String name(pszText);
    std::transform(name.mBuffer, name.mBuffer + name.mLength, name.mBuffer, toupper);

    FxMidi *pFx = TheFxMidi;
    const struct {
        const char *pszName;
        Ptr<Muse> *pCue;
    } cues[] = {
        {"MISS", &pFx->mMiss},
        {"UNCATCHABLE", &pFx->mUncatchable},
        {"CHECKPOINT_CHEER", &pFx->mCheckpointCheer},
        {"CHECKPOINT_INSANE", &pFx->mCheckpointInsane},
        {"WIN_CHEER", &pFx->mWinCheer},
        {"WARNING", &pFx->mWarning},
        {"GUIDETICK1", &pFx->mGuideTicks[0]},
        {"GUIDETICK2", &pFx->mGuideTicks[1]},
        {"GUIDETICK3", &pFx->mGuideTicks[2]},
        {"AUTOCATCHER_CATCH", &pFx->mPowerupCatches[FxMidi::kPowerupAutocatcher]},
        {"FREESTYLER_CATCH", &pFx->mPowerupCatches[FxMidi::kPowerupFreestyler]},
        {"MULTIPLIER_CATCH", &pFx->mPowerupCatches[FxMidi::kPowerupMultiplier]},
        {"BUMPER_CATCH", &pFx->mPowerupCatches[FxMidi::kPowerupBumper]},
        {"CRIPPLER_CATCH", &pFx->mPowerupCatches[FxMidi::kPowerupCrippler]},
        {"SLOWDOWN_CATCH", &pFx->mPowerupCatches[FxMidi::kPowerupSlowdown]},
        {"AUTOCATCHER_DEPLOY", &pFx->mPowerupDeploys[FxMidi::kPowerupAutocatcher]},
        {"FREESTYLER_DEPLOY", &pFx->mPowerupDeploys[FxMidi::kPowerupFreestyler]},
        {"MULTIPLIER_DEPLOY", &pFx->mPowerupDeploys[FxMidi::kPowerupMultiplier]},
        {"BUMPER_DEPLOY", &pFx->mPowerupDeploys[FxMidi::kPowerupBumper]},
        {"CRIPPLER_DEPLOY", &pFx->mPowerupDeploys[FxMidi::kPowerupCrippler]},
        {"SLOWDOWN_DEPLOY", &pFx->mPowerupDeploys[FxMidi::kPowerupSlowdown]},
        {"WRONG", &pFx->mWrong},
        {"SQUARE", &pFx->mSquare},
        {"LAZYSUSAN", &pFx->mLazySusan},
        {"SOLOPORTAL", &pFx->mSoloPortal},
        {"MULTIPORTAL", &pFx->mMultiPortal},
        {"NETPORTAL", &pFx->mNetPortal},
        {"TRAVELSWOOSH", &pFx->mTravelSwoosh},
        {"LEFT", &pFx->mLeft},
        {"RIGHT", &pFx->mRight},
        {"UP", &pFx->mUp},
        {"DOWN", &pFx->mDown},
        {"SELECT", &pFx->mSelect},
        {"BACK", &pFx->mBack},
        {"CHEAT", &pFx->mCheat},
        {"PROJECTOR", &pFx->mProjector},
        {"UNLOCK", &pFx->mUnlock},
        {"REDLEAD", &pFx->mLeads[FxMidi::kColorRed]},
        {"REDWINS", &pFx->mWins[FxMidi::kColorRed]},
        {"GREENLEAD", &pFx->mLeads[FxMidi::kColorGreen]},
        {"GREENWINS", &pFx->mWins[FxMidi::kColorGreen]},
        {"PURPLELEAD", &pFx->mLeads[FxMidi::kColorPurple]},
        {"PURPLEWINS", &pFx->mWins[FxMidi::kColorPurple]},
        {"YELLOWLEAD", &pFx->mLeads[FxMidi::kColorYellow]},
        {"YELLOWWINS", &pFx->mWins[FxMidi::kColorYellow]},
        {"DUEL_PLAYER1", &pFx->mDuelPlayers[0]},
        {"DUEL_PLAYER2", &pFx->mDuelPlayers[1]},
        {"KEYBOARD_LEFT_UP", &pFx->mKeyboardLeftUp},
        {"KEYBOARD_RIGHT_DOWN", &pFx->mKeyboardRightDown},
        {"KEYBOARD_CIRCLE", &pFx->mKeyboardCircle},
        {"KEYBOARD_BACK", &pFx->mKeyboardBack},
        {"KEYBOARD_KEYENTER", &pFx->mKeyboardKeyEnter},
        {"ARENA_UNLOCK", &pFx->mArenaUnlock},
        {"DUEL_LAYPATT", &pFx->mDuelLayPattern},
        {"DUEL_CATCHPATT", &pFx->mDuelCatchPattern},
        {"DUEL_NICE", &pFx->mDuelNice},
        {"DUEL_YOUGOTIT", &pFx->mDuelYouGotIt},
        {"DUEL_ALMOST", &pFx->mDuelAlmost},
        {"DUEL_ONELETTER", &pFx->mDuelOneLetter},
        {"DUEL_AWW", &pFx->mDuelAww},
        {"DUEL_PERFECT", &pFx->mDuelPerfect},
        {"DUEL_YEAH", &pFx->mDuelYeah},
        {"DECRYPT", &pFx->mDecrypt},
        {"TEXT", &pFx->mText},
        {"DUEL_GAMETIE", &pFx->mDuelGameTie},
        {"DUEL_GREENWINS", &pFx->mDuelGreenWins},
        {"DUEL_PURPLEWINS", &pFx->mDuelPurpleWins},
        {"DUEL_MISS", &pFx->mDuelMiss},
        {"DUEL_MISSTHIS", &pFx->mDuelMissThis},
        {"DUEL_CHEER", &pFx->mDuelCheer},
        {"ERASE1", &pFx->mErase},
        {"ERASE2", &pFx->mEraseSection},
    };

    Ptr<Muse> *pCue = nullptr;
    for (const auto &cue : cues) {
        if (name == cue.pszName) {
            pCue = cue.pCue;
            break;
        }
    }
    if (pCue == nullptr) {
        DebugError("unknown sound effect: %s\n", name.c_str());
    } else {
        mBuilder = new MuseBuilder(0, false, nullptr, pCue);
    }
}
