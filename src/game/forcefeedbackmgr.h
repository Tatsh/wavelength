#pragma once

#include <vector>

#include "os/ptr.h"

/**
 * Driver of the controllers' vibration motors during a song.
 *
 * The class is not polymorphic and emits no RTTI. The RTTI of the nested BeatCmd, Controller,
 * MotorEffect, and MotorEffect::OffCmd records the name. The one instance is the function-local
 * static of shared(), and TheForceFeedbackMgr addresses it.
 *
 * Two kinds of vibration run through it. The beat pulse vibrates every enabled controller that
 * plays no effect once a beat, and an effect runs one of three motor patterns on one controller.
 */
class ForceFeedbackMgr {
public:
    class BeatCmd;
    class Controller;
    class MotorEffect;

    /** The effects of a controller, indices into Controller::mEffects. */
    enum Effect {
        kEffectCripple = 0,   /*!< A crippler hit. */
        kEffectBump = 1,      /*!< A bumper hit. */
        kEffectAutocatch = 2, /*!< An autocatcher or a capture. */
        kEffectCount = 3,     /*!< The number of effects. */
    };

    /** The most players Start() prepares controllers for. */
    static constexpr int kMaxPlayers = 2;

    /** The number of controller ports. */
    static constexpr int kPadCount = 2;

    /**
     * Construct the manager with no controller and the beat pulse's command.
     *
     * @ghidraAddress NTSC-U/C: 0x0010c8e0
     * @ghidraAddress PAL: 0x0010e018
     */
    ForceFeedbackMgr();

    /**
     * Delete every controller.
     *
     * @ghidraAddress NTSC-U/C: 0x0010c958
     * @ghidraAddress PAL: 0x0010e090
     */
    ~ForceFeedbackMgr();

    /**
     * Report the single instance, constructing it on first use.
     *
     * @return The instance.
     * @ghidraAddress NTSC-U/C: 0x0010d290
     * @ghidraAddress PAL: 0x0010e9c8
     */
    static ForceFeedbackMgr *shared();

    /**
     * Prepare one controller for each player and enable vibration.
     *
     * Nothing happens when more than kMaxPlayers players take part, while a demo plays, or when
     * the game configuration disables vibration. Every effect is cancelled and both ports are
     * stopped first. The name is inferred.
     *
     * @param pTickDuration The duration of one tick on the song clock, which the manager retains.
     * @param nTicksPerBar The song ticks in one bar.
     * @ghidraAddress NTSC-U/C: 0x0010ca08
     * @ghidraAddress PAL: 0x0010e140
     */
    void Start(const float *pTickDuration, int nTicksPerBar);

    /**
     * Withdraw the beat pulse, cancel every effect, and disable vibration.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0010cd58
     * @ghidraAddress PAL: 0x0010e490
     */
    void StopAll();

    /**
     * Schedule the beat pulse at the first multiple of a period from a tick that still lies ahead.
     *
     * The pulse leads the beat by the configured lead, and every controller is enabled. Nothing
     * happens while vibration is disabled. The name is inferred.
     *
     * @param nPeriod The ticks between two pulses.
     * @param nTick The tick the pulse starts from.
     * @ghidraAddress NTSC-U/C: 0x0010cde8
     * @ghidraAddress PAL: 0x0010e520
     */
    void StartMetronome(int nPeriod, int nTick);

    /**
     * Withdraw the beat pulse from the song scheduler.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0010cf58
     * @ghidraAddress PAL: 0x0010e690
     */
    void StopMetronome();

    /**
     * Set whether the beat pulse drives one controller.
     *
     * Nothing happens while vibration is disabled. The name is inferred.
     *
     * @param nPad The controller.
     * @param bEnabled Whether the controller follows the beat pulse.
     * @ghidraAddress NTSC-U/C: 0x0010cf90
     * @ghidraAddress PAL: 0x0010e6c8
     */
    void SetBeatEnabled(int nPad, bool bEnabled);

    /**
     * Record a controller's motor levels and pass them to its port.
     *
     * The port receives the levels only while the manager is not suspended and the controller's
     * vibration option is on. The name is inferred.
     *
     * @param nController The controller.
     * @param nSmallMotor The small motor's state, 0 or 1.
     * @param nBigMotor The big motor's level.
     * @ghidraAddress NTSC-U/C: 0x0010cfb8
     * @ghidraAddress PAL: 0x0010e6f0
     */
    void SetVibration(int nController, int nSmallMotor, int nBigMotor);

    /**
     * Play the bumper effect on a controller while vibration is enabled.
     *
     * @param nController The controller.
     * @ghidraAddress NTSC-U/C: 0x0010d010
     * @ghidraAddress PAL: 0x0010e748
     */
    void PlayBumpEffect(int nController);

    /**
     * Play the crippler effect on a controller while vibration is enabled.
     *
     * @param nController The controller.
     * @ghidraAddress NTSC-U/C: 0x0010d038
     * @ghidraAddress PAL: 0x0010e770
     */
    void PlayCrippleEffect(int nController);

    /**
     * Play the autocatcher effect on a controller while vibration is enabled.
     *
     * @param nController The controller.
     * @ghidraAddress NTSC-U/C: 0x0010d060
     * @ghidraAddress PAL: 0x0010e798
     */
    void PlayAutocatchEffect(int nController);

    /**
     * Cancel every effect of a controller and start one.
     *
     * The name is inferred.
     *
     * @param nController The controller.
     * @param nEffect One of Effect.
     * @ghidraAddress NTSC-U/C: 0x0010d088
     * @ghidraAddress PAL: 0x0010e7c0
     */
    void PlayEffect(int nController, int nEffect);

    /**
     * Stop the motors of both ports while the game is paused.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0010d130
     * @ghidraAddress PAL: 0x0010e868
     */
    void Pause();

    /**
     * Read each controller's vibration option again and restore the motors Pause() stopped.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0010d198
     * @ghidraAddress PAL: 0x0010e8d0
     */
    void Resume();

    Ptr<BeatCmd> mBeatCmd;                  /*!< The command of the beat pulse. */
    const float *mTickDuration;             /*!< The duration of one tick, from Start(). */
    std::vector<Controller *> mControllers; /*!< One controller for each player. */
    int mEnabled;                           /*!< Whether vibration runs, set by Start(). */
    int mPaused;                            /*!< Whether Pause() stopped the motors. */
};

/**
 * The vibration manager, ForceFeedbackMgr::shared() as the unit's static initialiser stored it.
 *
 * @ghidraAddress NTSC-U/C: 0x00435df4
 */
extern ForceFeedbackMgr *TheForceFeedbackMgr;
