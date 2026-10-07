#include "game/gameconfig.h"

#include "game/gamecallback.h"
#include "game/gamefx.h"
#include "game/gamelogic.h"
#include "os/debug.h"
#include "os/system.h"
#include "script/scriptfunction.h"

namespace {

constexpr char kDuelAuthoringCommand[] = "duel_authoring";
constexpr char kPowerupCheatCommand[] = "pup_cheat_mode";
constexpr char kScrambleGemsCommand[] = "scramble_gems";
constexpr char kPowerupsAPlentyCommand[] = "powerups_a_plenty";
constexpr char kAutopilotCommand[] = "autopilot";

constexpr char kOnText[] = "ON";
constexpr char kOffText[] = "OFF";

constexpr int kUnsetInt = -1;
constexpr float kUnsetFloat = -1.0f;
constexpr float kDefaultAxeSoftFxVolume = 1.0f;

// The index of the first value of a list, after the symbol that starts it.
constexpr int kFirstValue = 1;

} // namespace

GameCallback *TheGameCallback = nullptr;
int g_bDuelAuthoring = 0;
int g_bPowerupCheat = 0;
int g_bScrambleGems = 0;
int g_bPowerupsAPlenty = 0;

void GameCallback::Set(GameCallback *pCallback) {
    TheGameCallback = pCallback;
}

void GameConfig::ReadFloatMatrix(DataArray *pArray,
                                 const char *pszKey,
                                 std::vector<std::vector<float>> &table) {
    DataArray *pTable = pArray->FindArray(pszKey, true);
    table.clear();
    table.resize(kMaxPlayers);
    for (int nRow = kFirstValue; nRow < pTable->mSize; ++nRow) {
        // Neither the rows nor the columns of the child array are checked against the table size.
        std::vector<float> &row = table[nRow - kFirstValue];
        row.resize(kPowerupDistColumnCount);
        const DataArray *pRow = pTable->Array(nRow);
        for (int nColumn = 0; nColumn < pRow->mSize; ++nColumn) {
            row[nColumn] = pRow->Float(nColumn);
        }
    }
}

void GameConfig::ReadFloatVector(DataArray *pArray,
                                 const char *pszKey,
                                 std::vector<float> &values) {
    const DataArray *pValues = pArray->FindArray(pszKey, true);
    values.clear();
    for (int i = kFirstValue; i < pValues->mSize; ++i) {
        values.push_back(pValues->Float(i));
    }
}

void GameConfig::ReadIntVector(DataArray *pArray, const char *pszKey, std::vector<int> &values) {
    const DataArray *pValues = pArray->FindArray(pszKey, true);
    values.clear();
    for (int i = kFirstValue; i < pValues->mSize; ++i) {
        values.push_back(pValues->Int(i));
    }
}

GameConfig::GameConfig()
    : mSlopMs(kUnsetInt), mStrandBars(), mGemPoints(), mPhraseScale(kUnsetInt), mFreestylePoints(),
      mAutocatcherPoints(), mPlayFromFile(nullptr), mRecordToFile(nullptr), mZeroRandSeed(0),
      mForceFeedbackEnabled(0), mBeatDurationMs(kUnsetInt), mBeatLeadMs(kUnsetInt),
      mScratcherQuantizationTicks(kUnsetInt), mScratcherSampleQuantizationTicks(kUnsetInt),
      mBarCaptureThreshold(kUnsetInt), mCheckpointBars(kUnsetInt), mPowerupProbSolo(kUnsetFloat),
      mPowerupProbMulti(),
      mPowerupDist(GameLogic::kPowerupCount, std::vector<std::vector<float>>()),
      mSlowdownStartTicks(kUnsetInt), mSlowdownStopTicks(kUnsetInt),
      mSlowdownDurationBars(kUnsetInt), mSlowdownSpeed(kUnsetFloat),
      mMultiplierDurationBars(kUnsetInt), mMultiplierValue(kUnsetInt),
      mFreestyleDurationBarsSolo(kUnsetInt), mFreestyleDurationBarsMultiNet(kUnsetInt),
      mCripplerDurationBars(kUnsetInt), mAxeSoftFxVolume(kDefaultAxeSoftFxVolume),
      mStreakMultiplierMaxSolo(kUnsetInt), mStreakMultiplierMaxMulti(kUnsetInt),
      mRotationRepeatInitialDelayMs(kUnsetFloat), mRotationRepeatDelayMs(kUnsetFloat),
      mBarsPerCapture(), mInitialJuice(), mCaptureJuice(), mJuiceMeterMax(kUnsetFloat),
      mFakeInput(0), mPlayAllGems(0), mStreaksEnabled(0), mNoCapture(0), mNoDeactivate(0),
      mGuideTicks(0) {
}

void GameConfig::SetStrandBars(int nStrand, int nBars) {
    mStrandBars[nStrand - 1] = nBars;
}

void GameConfig::Init() {
    Load();
    ScriptFunction::Register(EnableDuelAuthoring, kDuelAuthoringCommand, nullptr);
    ScriptFunction::Register(TogglePowerupCheat, kPowerupCheatCommand, nullptr);
    ScriptFunction::Register(ToggleScrambleGems, kScrambleGemsCommand, nullptr);
    ScriptFunction::Register(TogglePowerupsAPlenty, kPowerupsAPlentyCommand, nullptr);
    ScriptFunction::Register(ToggleAutopilot, kAutopilotCommand, nullptr);
}

float GameConfig::GetPowerupWeight(int nPlayers, int nSection, int nSections, int nPowerup) {
    int nColumn;
    switch (nSections - nSection) {
    case 1:
        nColumn = kPowerupDistColumnLast;
        break;
    case 2:
        nColumn = kPowerupDistColumnSecondLast;
        break;
    case 3:
        nColumn = kPowerupDistColumnThirdLast;
        break;
    default:
        nColumn = kPowerupDistColumnEarlier;
        break;
    }
    return mPowerupDist[nPowerup][nPlayers - 1][nColumn];
}

void GameConfig::Load() {
    DataArray *pGame = SystemConfig()->FindArray("game", true);
    pGame->FindInt("slop_ms", &mSlopMs, true);
    pGame->FindInt("beat_duration_ms", &mBeatDurationMs, true);
    pGame->FindInt("beat_lead_ms", &mBeatLeadMs, true);
    pGame->FindInt("scratcher_quantization_ticks", &mScratcherQuantizationTicks, true);
    pGame->FindInt("scratcher_sample_quantization_ticks", &mScratcherSampleQuantizationTicks, true);
    pGame->FindInt("bar_capture_threshold", &mBarCaptureThreshold, true);
    pGame->FindInt("checkpoint_bars", &mCheckpointBars, true);
    pGame->FindInt("slowdown_duration_bars", &mSlowdownDurationBars, true);
    pGame->FindInt("slowdown_start_ticks", &mSlowdownStartTicks, true);
    pGame->FindInt("slowdown_stop_ticks", &mSlowdownStopTicks, true);
    pGame->FindInt("multiplier_duration_bars", &mMultiplierDurationBars, true);
    pGame->FindInt("multiplier_value", &mMultiplierValue, true);
    pGame->FindInt("freestyle_duration_bars_solo", &mFreestyleDurationBarsSolo, true);
    pGame->FindInt("freestyle_duration_bars_multi_net", &mFreestyleDurationBarsMultiNet, true);
    pGame->FindInt("crippler_duration_bars", &mCripplerDurationBars, true);
    pGame->FindInt("streak_multiplier_max_solo", &mStreakMultiplierMaxSolo, true);
    pGame->FindInt("streak_multiplier_max_multi", &mStreakMultiplierMaxMulti, true);
    pGame->FindFloat("juice_meter_max", &mJuiceMeterMax, true);
    pGame->FindFloat("powerup_prob_solo", &mPowerupProbSolo, true);
    pGame->FindFloat("slowdown_speed", &mSlowdownSpeed, true);
    pGame->FindFloat("rotation_repeat_initial_delay_ms", &mRotationRepeatInitialDelayMs, true);
    pGame->FindFloat("axe_softfx_volume", &mAxeSoftFxVolume, true);
    pGame->FindFloat("rotation_repeat_delay_ms", &mRotationRepeatDelayMs, true);
    pGame->FindBool("force_feedback_enabled", &mForceFeedbackEnabled, true);
    pGame->FindBool("play_all_gems", &mPlayAllGems, true);
    pGame->FindBool("streaks_enabled", &mStreaksEnabled, true);
    pGame->FindBool("no_capture", &mNoCapture, true);
    pGame->FindBool("guide_ticks", &mGuideTicks, true);
    pGame->FindBool("fake_input", &mFakeInput, true);
    pGame->FindBool("zero_rand_seed", &mZeroRandSeed, true);
    ReadFloatVector(pGame, "bars_per_capture", mBarsPerCapture);
    ReadFloatVector(pGame, "initial_juice", mInitialJuice);
    ReadFloatVector(pGame, "capture_juice", mCaptureJuice);
    ReadFloatVector(pGame, "powerup_prob_multi", mPowerupProbMulti);
    ReadIntVector(pGame, "strand_bars", mStrandBars);

    // A missing table is not checked for before its kinds are read.
    DataArray *pDist = pGame->FindArray("powerup_dist", false);
    ReadFloatMatrix(pDist, "autocatcher", mPowerupDist[GameLogic::kPowerupAutocatcher]);
    ReadFloatMatrix(pDist, "bumper", mPowerupDist[GameLogic::kPowerupBumper]);
    ReadFloatMatrix(pDist, "crippler", mPowerupDist[GameLogic::kPowerupCrippler]);
    ReadFloatMatrix(pDist, "multiplier", mPowerupDist[GameLogic::kPowerupMultiplier]);
    ReadFloatMatrix(pDist, "slowdown", mPowerupDist[GameLogic::kPowerupSlowdown]);
    ReadFloatMatrix(pDist, "freestyle", mPowerupDist[GameLogic::kPowerupFreestyle]);

    DataArray *pPoints = pGame->FindArray("points", true);
    ReadIntVector(pPoints, "freestyle", mFreestylePoints);
    ReadIntVector(pPoints, "autocatcher", mAutocatcherPoints);
    pPoints->FindInt("phrase_scale", &mPhraseScale, true);
    const DataArray *pGems = pPoints->FindArray("gem", true);
    mGemPoints.clear();
    for (int nGem = kFirstValue; nGem < pGems->mSize; ++nGem) {
        const DataArray *pGem = pGems->Array(nGem);
        const int nDivisor = pGem->Int(0);
        const int nPoints = pGem->Int(1);
        mGemPoints.push_back(GemPointValue{nDivisor, nPoints});
    }

    pGame->FindSymbol("play_from_file", &mPlayFromFile, false);
    pGame->FindSymbol("record_to_file", &mRecordToFile, false);
}

GameConfig *GameConfig::shared() {
    static GameConfig instance;
    return &instance;
}

void GameConfig::EnableDuelAuthoring([[maybe_unused]] DataArray *pCommand,
                                     [[maybe_unused]] void *pUserData) {
    g_bDuelAuthoring = 1;
    DebugPrint("CHEAT: duel authoring mode is on!\n");
}

void GameConfig::TogglePowerupCheat([[maybe_unused]] DataArray *pCommand,
                                    [[maybe_unused]] void *pUserData) {
    GameFx::PlayCheat();
    g_bPowerupCheat = !g_bPowerupCheat;
    DebugPrint("CHEAT: powerup cheat mode %s\n", g_bPowerupCheat ? kOnText : kOffText);
}

void GameConfig::ToggleScrambleGems([[maybe_unused]] DataArray *pCommand,
                                    [[maybe_unused]] void *pUserData) {
    GameFx::PlayCheat();
    g_bScrambleGems = !g_bScrambleGems;
    DebugPrint("CHEAT: scrambled gem mode %s\n", g_bScrambleGems ? kOnText : kOffText);
}

void GameConfig::TogglePowerupsAPlenty([[maybe_unused]] DataArray *pCommand,
                                       [[maybe_unused]] void *pUserData) {
    GameFx::PlayCheat();
    g_bPowerupsAPlenty = !g_bPowerupsAPlenty;
    DebugPrint("CHEAT: powerups-a-plenty mode %s\n", g_bPowerupsAPlenty ? kOnText : kOffText);
}

void GameConfig::ToggleAutopilot([[maybe_unused]] DataArray *pCommand,
                                 [[maybe_unused]] void *pUserData) {
    GameFx::PlayCheat();
    TheGameConfig->mFakeInput = !TheGameConfig->mFakeInput;
    DebugPrint("CHEAT: autopilot mode %s\n", TheGameConfig->mFakeInput ? kOnText : kOffText);
}

GameConfig *TheGameConfig = GameConfig::shared();
