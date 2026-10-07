#pragma once

/**
 * Listener the gameplay code tells about the player's actions.
 *
 * The RTTI includes the class name, and the class has no base. Every member does nothing in this
 * class. A tutorial step that waits for an action derives from it and installs itself as
 * TheGameCallback while it runs.
 */
class GameCallback {
public:
    /**
     * Install the listener the gameplay code reports to.
     *
     * @param pCallback The listener, or null to stop reporting.
     * @ghidraAddress NTSC-U/C: 0x0010f020
     * @ghidraAddress PAL: 0x001107b8
     */
    static void Set(GameCallback *pCallback);

    /**
     * Release the listener.
     *
     * @ghidraAddress NTSC-U/C: 0x003443a8
     * @ghidraAddress PAL: 0x003b18e0
     */
    virtual ~GameCallback() {
    }

    /**
     * Report a gem the player hit.
     *
     * @ghidraAddress NTSC-U/C: 0x00343f40
     */
    virtual void OnGemHit() {
    }

    /**
     * Report a gem the player missed.
     *
     * @param nLane The lane of the gem.
     * @ghidraAddress NTSC-U/C: 0x00343f48
     */
    virtual void OnGemMiss([[maybe_unused]] int nLane) {
    }

    /**
     * Report a gem the player let pass.
     *
     * @ghidraAddress NTSC-U/C: 0x00343f50
     */
    virtual void OnGemPass() {
    }

    /**
     * Report a track the player captured.
     *
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x00343f58
     */
    virtual void OnCapture([[maybe_unused]] int nTrack) {
    }

    /**
     * Report an automatic capture.
     *
     * @ghidraAddress NTSC-U/C: 0x00343f60
     */
    virtual void OnAutocapture() {
    }

    /**
     * Report a broken streak.
     *
     * @param nStreak The length of the streak that broke.
     * @ghidraAddress NTSC-U/C: 0x00343f68
     */
    virtual void OnStreakBroken([[maybe_unused]] int nStreak) {
    }

    /**
     * Report a powerup the player deployed.
     *
     * @ghidraAddress NTSC-U/C: 0x00343f70
     */
    virtual void OnPowerup() {
    }

    /**
     * Report a pitch change.
     *
     * @ghidraAddress NTSC-U/C: 0x00343f78
     */
    virtual void OnPitch() {
    }

    /**
     * Report that looping was switched on.
     *
     * @ghidraAddress NTSC-U/C: 0x00343f80
     */
    virtual void OnLoop() {
    }

    /**
     * Report a section change.
     *
     * @param nSection The section.
     * @ghidraAddress NTSC-U/C: 0x00343f88
     */
    virtual void OnSectionChange([[maybe_unused]] int nSection) {
    }

    /**
     * Report a burned pattern.
     *
     * @ghidraAddress NTSC-U/C: 0x00343f90
     */
    virtual void OnBurnPattern() {
    }

    /**
     * Report an erased pattern.
     *
     * @ghidraAddress NTSC-U/C: 0x00343f98
     */
    virtual void OnErase() {
    }

    /**
     * Report an erased section.
     *
     * @ghidraAddress NTSC-U/C: 0x00343fa0
     */
    virtual void OnEraseSection() {
    }

    /**
     * Report a rotation.
     *
     * @param bRight The rotation was to the right.
     * @ghidraAddress NTSC-U/C: 0x00343fa8
     */
    virtual void OnRotate([[maybe_unused]] bool bRight) {
    }

    /**
     * Report that the pattern menu was opened.
     *
     * @ghidraAddress NTSC-U/C: 0x00343fb0
     */
    virtual void OnPatternMenuOpen() {
    }
};

/**
 * The listener the gameplay code reports to, or null.
 *
 * @ghidraAddress NTSC-U/C: 0x003af708
 */
extern GameCallback *TheGameCallback;
