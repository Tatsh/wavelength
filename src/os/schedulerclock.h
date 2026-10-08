#pragma once

/**
 * Clock that runs at a settable rate on the cycles the system clock accumulates.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the one owner, Scheduler.
 * The object is 0x18 bytes. The time is kept in system cycles and reported in milliseconds.
 */
class SchedulerClock {
public:
    /**
     * Construct a stopped clock at a time, at the normal rate.
     *
     * @param fMs The time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00282a58
     * @ghidraAddress PAL: 0x0028c328
     */
    explicit SchedulerClock(float fMs);

    /**
     * Start a stopped clock.
     *
     * @ghidraAddress NTSC-U/C: 0x00282aa8
     * @ghidraAddress PAL: 0x0028c378
     */
    void Start();

    /**
     * Stop a running clock, adding the scaled cycles since the start to the time.
     *
     * @ghidraAddress NTSC-U/C: 0x00282b08
     * @ghidraAddress PAL: 0x0028c3d8
     */
    void Stop();

    /**
     * Report whether the clock runs.
     *
     * @return Non-zero while the clock runs.
     * @ghidraAddress NTSC-U/C: 0x00282b98
     * @ghidraAddress PAL: 0x0028c468
     */
    int IsRunning() const;

    /**
     * Set the time, measuring from now.
     *
     * @param fMs The time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00282ba0
     * @ghidraAddress PAL: 0x0028c470
     */
    void SetTime(float fMs);

    /**
     * Change the rate, keeping the time measured so far.
     *
     * @param fRate The rate, 1 for real time.
     * @ghidraAddress NTSC-U/C: 0x00282c18
     * @ghidraAddress PAL: 0x0028c4e8
     */
    void SetRate(float fRate);

    /**
     * Report the rate.
     *
     * @return The rate.
     * @ghidraAddress NTSC-U/C: 0x00282c68
     * @ghidraAddress PAL: 0x0028c538
     */
    float GetRate() const;

    /**
     * Report the time.
     *
     * @return The time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00282c70
     * @ghidraAddress PAL: 0x0028c540
     */
    float GetTime();

private:
    unsigned long long mStart; // The system cycles at the last start.
    long long mElapsed;        // The scaled cycles measured before the last start.
    int mRunning;              // Non-zero while the clock runs.
    float mRate;               // The rate the cycles since the start are scaled by.
};
