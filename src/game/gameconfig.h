#pragma once

#include <vector>

/**
 * Tuning values of the game, read from the "game" section of the configuration.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the section. The one
 * instance is the function-local static of its singleton accessor, and TheGameConfig addresses
 * it. Only the members its callers here read are declared, and each records its offset.
 */
class GameConfig {
public:
    /**
     * Read the "game" section of the configuration and register the script commands.
     *
     * @ghidraAddress NTSC-U/C: 0x0010f990
     * @ghidraAddress PAL: 0x00111128
     */
    void Init();

    /**
     * Report the weight of a kind of power-up in a section, from the `powerup_dist` tables.
     *
     * Each of the last three sections has a column of its own, and the earlier sections share the
     * first column.
     *
     * @param nPlayers The number of players.
     * @param nSection The section.
     * @param nSections The number of sections.
     * @param nPowerup The kind of power-up.
     * @return The weight.
     * @ghidraAddress NTSC-U/C: 0x0010fa30
     * @ghidraAddress PAL: 0x001111c8
     */
    float GetPowerupWeight(int nPlayers, int nSection, int nSections, int nPowerup);

    int mSlopMs;                           /*!< `slop_ms`. +0x00 */
    int mCheckpointBars;                   /*!< `checkpoint_bars`. +0x6c */
    float mPowerupProbSolo;                /*!< `powerup_prob_solo`. +0x70 */
    std::vector<float> mPowerupProbMulti;  /*!< `powerup_prob_multi`. +0x74 */
    int mSlowdownStartTicks;               /*!< `slowdown_start_ticks`. +0x94 */
    int mSlowdownStopTicks;                /*!< `slowdown_stop_ticks`. +0x98 */
    int mSlowdownDurationBars;             /*!< `slowdown_duration_bars`. +0x9c */
    float mSlowdownSpeed;                  /*!< `slowdown_speed`. +0xa0 */
    int mMultiplierDurationBars;           /*!< `multiplier_duration_bars`. +0xa4 */
    int mMultiplierValue;                  /*!< `multiplier_value`. +0xa8 */
    int mFreestyleDurationBarsSolo;        /*!< `freestyle_duration_bars_solo`. +0xac */
    int mFreestyleDurationBarsMultiNet;    /*!< `freestyle_duration_bars_multi_net`. +0xb0 */
    int mStreakMultiplierMaxSolo;          /*!< `streak_multiplier_max_solo`. +0xbc */
    int mStreakMultiplierMaxMulti;         /*!< `streak_multiplier_max_multi`. +0xc0 */
    float mRotationRepeatInitialDelayMs;   /*!< `rotation_repeat_initial_delay_ms`. +0xc4 */
    float mRotationRepeatDelayMs;          /*!< `rotation_repeat_delay_ms`. +0xc8 */
    bool mStreaksEnabled;                  /*!< `streaks_enabled`. +0x108 */
};

/**
 * The game configuration.
 *
 * @ghidraAddress NTSC-U/C: 0x00435f14
 */
extern GameConfig *TheGameConfig;

/**
 * Whether the cheat that lets the `powerup` script command give any power-up is on.
 *
 * @ghidraAddress NTSC-U/C: 0x003af710
 */
extern int g_bPowerupCheat;

/**
 * Whether the cheat that places a random power-up in every bar is on.
 *
 * @ghidraAddress NTSC-U/C: 0x003af718
 */
extern int g_bPowerupsAPlenty;
