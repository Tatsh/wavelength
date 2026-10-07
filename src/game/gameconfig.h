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
 */
extern int g_bDuelAuthoring;

/**
 * Whether the cheat that lets the `powerup` script command give any power-up is on.
 *
 * @ghidraAddress NTSC-U/C: 0x003af710
 */
extern int g_bPowerupCheat;

/**
 * Whether the scrambled gem mode the `scramble_gems` command toggles is on.
 *
 * @ghidraAddress NTSC-U/C: 0x003af714
 */
extern int g_bScrambleGems;

/**
 * Whether the cheat that places a random power-up in every bar is on.
 *
 * @ghidraAddress NTSC-U/C: 0x003af718
 */
extern int g_bPowerupsAPlenty;
