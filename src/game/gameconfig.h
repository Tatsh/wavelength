#pragma once

#include <vector>

#include "script/dataarray.h"

/**
 * Points of a gem whose tick is a multiple of a divisor, from the `gem` list of the `points`
 * section.
 */
struct GemPointValue {
    int mDivisor; /*!< The divisor of the gem's tick. */
    int mPoints;  /*!< The points of the gem. */
};

/**
 * Tuning values of the game, read from the "game" section of the configuration.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the section. The one
 * instance is the function-local static of shared(), and TheGameConfig addresses it. Each value
 * except mAxeSoftFxVolume starts unset (-1, null, or false) until Init() reads its key. The vectors
 * with one entry for each skill level are indexed by GameDb::mSkillLevel.
 */
class GameConfig {
public:
    /**
     * Start with every value unset, mAxeSoftFxVolume at 1, and an empty power-up table for each
     * kind of power-up.
     *
     * @ghidraAddress NTSC-U/C: 0x0010f5d0
     * @ghidraAddress PAL: 0x00110d68
     */
    GameConfig();

    /**
     * Release the tables.
     *
     * @ghidraAddress NTSC-U/C: 0x00336db8
     * @ghidraAddress PAL: 0x003a4368
     */
    ~GameConfig() = default;

    /**
     * Read the "game" section of the configuration and register the cheat script commands.
     *
     * @ghidraAddress NTSC-U/C: 0x0010f990
     * @ghidraAddress PAL: 0x00111128
     */
    void Init();

    /**
     * Replace the bars of one strand in mStrandBars.
     *
     * @param nStrand The strand, counting from 1.
     * @param nBars The bars.
     * @ghidraAddress NTSC-U/C: 0x0010f978
     * @ghidraAddress PAL: 0x00111110
     */
    void SetStrandBars(int nStrand, int nBars);

    /**
     * Report the weight of a kind of power-up in a section, from the `powerup_dist` tables.
     *
     * Each of the last three sections has a column of its own, and the earlier sections share the
     * first column.
     *
     * @param nPlayers The number of players.
     * @param nSection The section.
     * @param nSections The number of sections.
     * @param nPowerup The kind of power-up, a GameLogic::Powerup.
     * @return The weight.
     * @ghidraAddress NTSC-U/C: 0x0010fa30
     * @ghidraAddress PAL: 0x001111c8
     */
    float GetPowerupWeight(int nPlayers, int nSection, int nSections, int nPowerup);

    /**
     * Report the single instance, constructing it on first use.
     *
     * The instance is destroyed at exit.
     *
     * @return The instance.
     * @ghidraAddress NTSC-U/C: 0x00110200
     * @ghidraAddress PAL: 0x00111998
     */
    static GameConfig *shared();

    /** @return `zero_rand_seed`. */
    int GetZeroRandSeed() const {
        return mZeroRandSeed;
    }

    /** @return `streaks_enabled`. */
    int GetStreaksEnabled() const {
        return mStreaksEnabled;
    }

    /** @param nStreaksEnabled `streaks_enabled`. */
    void SetStreaksEnabled(int nStreaksEnabled) {
        mStreaksEnabled = nStreaksEnabled;
    }

    /** @return `streak_multiplier_max_solo`. */
    int GetStreakMultiplierMaxSolo() const {
        return mStreakMultiplierMaxSolo;
    }

    /** @return `streak_multiplier_max_multi`. */
    int GetStreakMultiplierMaxMulti() const {
        return mStreakMultiplierMaxMulti;
    }

    /** @return `strand_bars`. */
    const std::vector<int> &GetStrandBars() const {
        return mStrandBars;
    }

    /** @return `strand_bars`. */
    std::vector<int> &GetStrandBars() {
        return mStrandBars;
    }

    /** @return `slowdown_stop_ticks`. */
    int GetSlowdownStopTicks() const {
        return mSlowdownStopTicks;
    }

    /** @return `slowdown_start_ticks`. */
    int GetSlowdownStartTicks() const {
        return mSlowdownStartTicks;
    }

    /** @return `slowdown_speed`. */
    float GetSlowdownSpeed() const {
        return mSlowdownSpeed;
    }

    /** @return `slowdown_duration_bars`. */
    int GetSlowdownDurationBars() const {
        return mSlowdownDurationBars;
    }

    /** @return `slop_ms`. */
    int GetSlopMs() const {
        return mSlopMs;
    }

    /** @return `scratcher_sample_quantization_ticks`. */
    int GetScratcherSampleQuantizationTicks() const {
        return mScratcherSampleQuantizationTicks;
    }

    /** @return `scratcher_quantization_ticks`. */
    int GetScratcherQuantizationTicks() const {
        return mScratcherQuantizationTicks;
    }

    /** @return `rotation_repeat_initial_delay_ms`. */
    float GetRotationRepeatInitialDelayMs() const {
        return mRotationRepeatInitialDelayMs;
    }

    /** @return `rotation_repeat_delay_ms`. */
    float GetRotationRepeatDelayMs() const {
        return mRotationRepeatDelayMs;
    }

    /** @return `powerup_prob_solo`. */
    float GetPowerupProbSolo() const {
        return mPowerupProbSolo;
    }

    /** @return `powerup_prob_multi`. */
    const std::vector<float> &GetPowerupProbMulti() const {
        return mPowerupProbMulti;
    }

    /** @return `powerup_prob_multi`. */
    std::vector<float> &GetPowerupProbMulti() {
        return mPowerupProbMulti;
    }

    /** @return `play_all_gems`. */
    int GetPlayAllGems() const {
        return mPlayAllGems;
    }

    /** @param nPlayAllGems `play_all_gems`. */
    void SetPlayAllGems(int nPlayAllGems) {
        mPlayAllGems = nPlayAllGems;
    }

    /** @return `phrase_scale` of `points`. */
    int GetPhraseScale() const {
        return mPhraseScale;
    }

    /** @return Set by the `set_no_deactivate` command. */
    int GetNoDeactivate() const {
        return mNoDeactivate;
    }

    /** @param nNoDeactivate Set by the `set_no_deactivate` command. */
    void SetNoDeactivate(int nNoDeactivate) {
        mNoDeactivate = nNoDeactivate;
    }

    /** @return `no_capture`. */
    int GetNoCapture() const {
        return mNoCapture;
    }

    /** @param nNoCapture `no_capture`. */
    void SetNoCapture(int nNoCapture) {
        mNoCapture = nNoCapture;
    }

    /** @return `multiplier_value`. */
    int GetMultiplierValue() const {
        return mMultiplierValue;
    }

    /** @return `multiplier_duration_bars`. */
    int GetMultiplierDurationBars() const {
        return mMultiplierDurationBars;
    }

    /** @return `juice_meter_max`. */
    float GetJuiceMeterMax() const {
        return mJuiceMeterMax;
    }

    /** @return `initial_juice`, by skill level. */
    const std::vector<float> &GetInitialJuice() const {
        return mInitialJuice;
    }

    /** @return `initial_juice`, by skill level. */
    std::vector<float> &GetInitialJuice() {
        return mInitialJuice;
    }

    /** @return `guide_ticks`. */
    int GetGuideTicks() const {
        return mGuideTicks;
    }

    /** @param nGuideTicks `guide_ticks`. */
    void SetGuideTicks(int nGuideTicks) {
        mGuideTicks = nGuideTicks;
    }

    /** @return `gem` of `points`, in order. */
    const std::vector<GemPointValue> &GetGemPoints() const {
        return mGemPoints;
    }

    /** @return `freestyle` of `points`. */
    const std::vector<int> &GetFreestylePoints() const {
        return mFreestylePoints;
    }

    /** @return `freestyle` of `points`. */
    std::vector<int> &GetFreestylePoints() {
        return mFreestylePoints;
    }

    /** @return `freestyle_duration_bars_solo`. */
    int GetFreestyleDurationBarsSolo() const {
        return mFreestyleDurationBarsSolo;
    }

    /** @return `freestyle_duration_bars_multi_net`. */
    int GetFreestyleDurationBarsMultiNet() const {
        return mFreestyleDurationBarsMultiNet;
    }

    /** @return `force_feedback_enabled`. */
    int GetForceFeedbackEnabled() const {
        return mForceFeedbackEnabled;
    }

    /** @return `fake_input`, toggled by the `autopilot` command. */
    int GetFakeInput() const {
        return mFakeInput;
    }

    /** @return `crippler_duration_bars`. */
    int GetCripplerDurationBars() const {
        return mCripplerDurationBars;
    }

    /** @return `checkpoint_bars`. */
    int GetCheckpointBars() const {
        return mCheckpointBars;
    }

    /** @return `capture_juice`, by skill level. */
    const std::vector<float> &GetCaptureJuice() const {
        return mCaptureJuice;
    }

    /** @return `capture_juice`, by skill level. */
    std::vector<float> &GetCaptureJuice() {
        return mCaptureJuice;
    }

    /** @return `beat_lead_ms`. */
    int GetBeatLeadMs() const {
        return mBeatLeadMs;
    }

    /** @return `beat_duration_ms`. */
    int GetBeatDurationMs() const {
        return mBeatDurationMs;
    }

    /** @return `bars_per_capture`, by skill level. */
    const std::vector<float> &GetBarsPerCapture() const {
        return mBarsPerCapture;
    }

    /** @return `bars_per_capture`, by skill level. */
    std::vector<float> &GetBarsPerCapture() {
        return mBarsPerCapture;
    }

    /** @return `bar_capture_threshold`. */
    int GetBarCaptureThreshold() const {
        return mBarCaptureThreshold;
    }

    /** @return `axe_softfx_volume`. */
    float GetAxeSoftFxVolume() const {
        return mAxeSoftFxVolume;
    }

    /** @return `autocatcher` of `points`. */
    const std::vector<int> &GetAutocatcherPoints() const {
        return mAutocatcherPoints;
    }

    /** @return `autocatcher` of `points`. */
    std::vector<int> &GetAutocatcherPoints() {
        return mAutocatcherPoints;
    }

    /** @return `play_from_file`, or null. */
    const char *GetPlayFromFile() const {
        return mPlayFromFile;
    }

    /** @return `record_to_file`, or null. */
    const char *GetRecordToFile() const {
        return mRecordToFile;
    }

private:
    /** Rows of each `powerup_dist` table, one for each number of players. */
    static constexpr int kMaxPlayers = 4;

    /** Columns of each `powerup_dist` table. */
    enum PowerupDistColumn {
        kPowerupDistColumnEarlier = 0,    /*!< Every section before the last three. */
        kPowerupDistColumnThirdLast = 1,  /*!< The third section from the end. */
        kPowerupDistColumnSecondLast = 2, /*!< The second section from the end. */
        kPowerupDistColumnLast = 3,       /*!< The last section. */
        kPowerupDistColumnCount = 4,      /*!< Number of columns. */
    };

    /**
     * Read the "game" section of the configuration into the members.
     *
     * @ghidraAddress NTSC-U/C: 0x0010fa90
     * @ghidraAddress PAL: 0x00111228
     */
    void Load();

    /**
     * Read a `powerup_dist` table, one row for each number of players.
     *
     * The table is cleared and given kMaxPlayers rows of kPowerupDistColumnCount columns. Row N of
     * the child array fills row N - 1 of the table.
     *
     * @param pArray The array the table is a child of.
     * @param pszKey The symbol that starts the child array.
     * @param table Receives the table.
     * @ghidraAddress NTSC-U/C: 0x0010f030
     * @ghidraAddress PAL: 0x001107c8
     */
    static void
    ReadFloatMatrix(DataArray *pArray, const char *pszKey, std::vector<std::vector<float>> &table);

    /**
     * Read the floating-point numbers of a child array.
     *
     * @param pArray The array the list is a child of.
     * @param pszKey The symbol that starts the child array.
     * @param values Receives the numbers that follow the symbol, replacing its contents.
     * @ghidraAddress NTSC-U/C: 0x0010f248
     * @ghidraAddress PAL: 0x001109e0
     */
    static void ReadFloatVector(DataArray *pArray, const char *pszKey, std::vector<float> &values);

    /**
     * Read the integers of a child array.
     *
     * @param pArray The array the list is a child of.
     * @param pszKey The symbol that starts the child array.
     * @param values Receives the integers that follow the symbol, replacing its contents.
     * @ghidraAddress NTSC-U/C: 0x0010f408
     * @ghidraAddress PAL: 0x00110ba0
     */
    static void ReadIntVector(DataArray *pArray, const char *pszKey, std::vector<int> &values);

    /**
     * Turn the duel authoring mode on, the `duel_authoring` script command.
     *
     * @param pCommand The command.
     * @param pUserData The value given to ScriptFunction::Register(), null.
     * @ghidraAddress NTSC-U/C: 0x00110258
     * @ghidraAddress PAL: 0x001119f0
     */
    static void EnableDuelAuthoring(DataArray *pCommand, void *pUserData);

    /**
     * Toggle g_bPowerupCheat, the `pup_cheat_mode` script command.
     *
     * @param pCommand The command.
     * @param pUserData The value given to ScriptFunction::Register(), null.
     * @ghidraAddress NTSC-U/C: 0x00110288
     * @ghidraAddress PAL: 0x00111a20
     */
    static void TogglePowerupCheat(DataArray *pCommand, void *pUserData);

    /**
     * Toggle g_bScrambleGems, the `scramble_gems` script command.
     *
     * @param pCommand The command.
     * @param pUserData The value given to ScriptFunction::Register(), null.
     * @ghidraAddress NTSC-U/C: 0x001102e0
     * @ghidraAddress PAL: 0x00111a78
     */
    static void ToggleScrambleGems(DataArray *pCommand, void *pUserData);

    /**
     * Toggle g_bPowerupsAPlenty, the `powerups_a_plenty` script command.
     *
     * @param pCommand The command.
     * @param pUserData The value given to ScriptFunction::Register(), null.
     * @ghidraAddress NTSC-U/C: 0x00110338
     * @ghidraAddress PAL: 0x00111ad0
     */
    static void TogglePowerupsAPlenty(DataArray *pCommand, void *pUserData);

    /**
     * Toggle mFakeInput of TheGameConfig, the `autopilot` script command.
     *
     * @param pCommand The command.
     * @param pUserData The value given to ScriptFunction::Register(), null.
     * @ghidraAddress NTSC-U/C: 0x00110390
     * @ghidraAddress PAL: 0x00111b28
     */
    static void ToggleAutopilot(DataArray *pCommand, void *pUserData);

    int mSlopMs;                           /*!< `slop_ms`. */
    std::vector<int> mStrandBars;          /*!< `strand_bars`. */
    std::vector<GemPointValue> mGemPoints; /*!< `gem` of `points`, in order. */
    int mPhraseScale;                      /*!< `phrase_scale` of `points`. */
    std::vector<int> mFreestylePoints;     /*!< `freestyle` of `points`. */
    std::vector<int> mAutocatcherPoints;   /*!< `autocatcher` of `points`. */
    const char *mPlayFromFile;             /*!< `play_from_file`, or null. */
    const char *mRecordToFile;             /*!< `record_to_file`, or null. */
    int mZeroRandSeed;                     /*!< `zero_rand_seed`. */
    int mForceFeedbackEnabled;             /*!< `force_feedback_enabled`. */
    int mBeatDurationMs;                   /*!< `beat_duration_ms`. */
    int mBeatLeadMs;                       /*!< `beat_lead_ms`. */
    int mScratcherQuantizationTicks;       /*!< `scratcher_quantization_ticks`. */
    int mScratcherSampleQuantizationTicks; /*!< `scratcher_sample_quantization_ticks`. */
    int mBarCaptureThreshold;              /*!< `bar_capture_threshold`. */
    int mCheckpointBars;                   /*!< `checkpoint_bars`. */
    float mPowerupProbSolo;                /*!< `powerup_prob_solo`. */
    std::vector<float> mPowerupProbMulti;  /*!< `powerup_prob_multi`. */
    /** `powerup_dist`, one table for each GameLogic::Powerup. */
    std::vector<std::vector<std::vector<float>>> mPowerupDist;
    int mSlowdownStartTicks;             /*!< `slowdown_start_ticks`. */
    int mSlowdownStopTicks;              /*!< `slowdown_stop_ticks`. */
    int mSlowdownDurationBars;           /*!< `slowdown_duration_bars`. */
    float mSlowdownSpeed;                /*!< `slowdown_speed`. */
    int mMultiplierDurationBars;         /*!< `multiplier_duration_bars`. */
    int mMultiplierValue;                /*!< `multiplier_value`. */
    int mFreestyleDurationBarsSolo;      /*!< `freestyle_duration_bars_solo`. */
    int mFreestyleDurationBarsMultiNet;  /*!< `freestyle_duration_bars_multi_net`. */
    int mCripplerDurationBars;           /*!< `crippler_duration_bars`. */
    float mAxeSoftFxVolume;              /*!< `axe_softfx_volume`. */
    int mStreakMultiplierMaxSolo;        /*!< `streak_multiplier_max_solo`. */
    int mStreakMultiplierMaxMulti;       /*!< `streak_multiplier_max_multi`. */
    float mRotationRepeatInitialDelayMs; /*!< `rotation_repeat_initial_delay_ms`. */
    float mRotationRepeatDelayMs;        /*!< `rotation_repeat_delay_ms`. */
    std::vector<float> mBarsPerCapture;  /*!< `bars_per_capture`, by skill level. */
    std::vector<float> mInitialJuice;    /*!< `initial_juice`, by skill level. */
    std::vector<float> mCaptureJuice;    /*!< `capture_juice`, by skill level. */
    float mJuiceMeterMax;                /*!< `juice_meter_max`. */
    int mFakeInput;                      /*!< `fake_input`, toggled by the `autopilot` command. */
    int mPlayAllGems;                    /*!< `play_all_gems`. */
    int mStreaksEnabled;                 /*!< `streaks_enabled`. */
    int mNoCapture;                      /*!< `no_capture`. */
    int mNoDeactivate;                   /*!< Set by the `set_no_deactivate` command. */
    int mGuideTicks;                     /*!< `guide_ticks`. */
};

/**
 * The game configuration, GameConfig::shared() as the unit's static initialiser stored it.
 *
 * @ghidraAddress NTSC-U/C: 0x00435f14
 */
extern GameConfig *TheGameConfig;

/**
 * Whether the `duel_authoring` command turned the duel authoring mode on.
 *
 * @ghidraAddress NTSC-U/C: 0x003af70c
 * @ghidraAddress PAL: 0x0042e5a4
 */
extern int g_bDuelAuthoring;

/**
 * Whether the cheat that lets the `powerup` script command give any power-up is on.
 *
 * @ghidraAddress NTSC-U/C: 0x003af710
 * @ghidraAddress PAL: 0x0042e5a8
 */
extern int g_bPowerupCheat;

/**
 * Whether the scrambled gem mode the `scramble_gems` command toggles is on.
 *
 * @ghidraAddress NTSC-U/C: 0x003af714
 * @ghidraAddress PAL: 0x0042e5ac
 */
extern int g_bScrambleGems;

/**
 * Whether the cheat that places a random power-up in every bar is on.
 *
 * @ghidraAddress NTSC-U/C: 0x003af718
 * @ghidraAddress PAL: 0x0042e5b0
 */
extern int g_bPowerupsAPlenty;
