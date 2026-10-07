#pragma once

/**
 * Vibration of the controllers, on the beat and for the effects of the game.
 *
 * The RTTI includes the nested ForceFeedbackMgr::BeatCmd, ForceFeedbackMgr::Controller, and
 * ForceFeedbackMgr::MotorEffect. Only the members its callers here use are declared.
 */
class ForceFeedbackMgr {
public:
    /**
     * Prepare the controllers for a song.
     *
     * @param pfMsPerTick The duration of one tick in milliseconds.
     * @param nTicksPerBar The length of a bar in ticks.
     * @ghidraAddress NTSC-U/C: 0x0010ca08
     * @ghidraAddress PAL: 0x0010e140
     */
    void Start(const float *pfMsPerTick, int nTicksPerBar);

    /**
     * Stop every motor and the beat.
     *
     * @ghidraAddress NTSC-U/C: 0x0010cd58
     * @ghidraAddress PAL: 0x0010e490
     */
    void StopAll();

    /**
     * Pulse the motors on the beat.
     *
     * @param nPeriodTicks The beat period in ticks.
     * @param nFirstTick The tick of the first beat.
     * @ghidraAddress NTSC-U/C: 0x0010cde8
     * @ghidraAddress PAL: 0x0010e520
     */
    void StartMetronome(int nPeriodTicks, int nFirstTick);

    /**
     * Let the motors of a controller pulse on the beat or stop them pulsing.
     *
     * @param nPad The controller.
     * @param bEnabled Whether the motors pulse.
     * @ghidraAddress NTSC-U/C: 0x0010cf90
     * @ghidraAddress PAL: 0x0010e6c8
     */
    void SetBeatEnabled(int nPad, bool bEnabled);

    /**
     * Play the effect of a bumper on a controller.
     *
     * @param nPad The controller.
     * @ghidraAddress NTSC-U/C: 0x0010d010
     * @ghidraAddress PAL: 0x0010e748
     */
    void PlayBumpEffect(int nPad);

    /**
     * Play the effect of a crippler on a controller.
     *
     * @param nPad The controller.
     * @ghidraAddress NTSC-U/C: 0x0010d038
     * @ghidraAddress PAL: 0x0010e770
     */
    void PlayCrippleEffect(int nPad);

    /**
     * Play the effect of an autocatcher on a controller.
     *
     * @param nPad The controller.
     * @ghidraAddress NTSC-U/C: 0x0010d060
     * @ghidraAddress PAL: 0x0010e798
     */
    void PlayAutocatchEffect(int nPad);

    /**
     * Stop the motors while the song is paused.
     *
     * @ghidraAddress NTSC-U/C: 0x0010d130
     * @ghidraAddress PAL: 0x0010e868
     */
    void Pause();

    /**
     * Restart the motors Pause() stopped.
     *
     * @ghidraAddress NTSC-U/C: 0x0010d198
     * @ghidraAddress PAL: 0x0010e8d0
     */
    void Resume();
};

/**
 * The vibration of the controllers.
 *
 * @ghidraAddress NTSC-U/C: 0x00435df4
 */
extern ForceFeedbackMgr *TheForceFeedbackMgr;
