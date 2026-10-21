#pragma once

namespace Sch {

/**
 * Clock the scheduler reads its due times from, in nanoseconds.
 *
 * The class is not polymorphic and has no RTTI. Its name comes from the debugging symbols of the
 * North American demo release. The demo's Pause() and Resume() have the same instructions as this
 * class's Pause() and Resume(). Only the member this reconstruction reads is recovered; the rest of
 * the 0x28-byte layout is recorded as unknown words to preserve the offsets.
 *
 * The clock's own reading is in milliseconds and Now() returns nanoseconds. The scale is not a
 * mystery constant: it is the double at `0x008263f8` divided by the 1000 that `0x00466360` returns,
 * which is one million nanoseconds to the millisecond.
 *
 * mOriginMs is public because MainLoop::Poll() reads it directly and the image has no accessor for
 * it. It sits amid unrecovered words, so the members below are grouped by access rather than by
 * offset, and each trailing comment records the real offset.
 */
class SystemTime {
public:
    SystemTime();

    /**
     * Read the clock.
     *
     * @return The current reading in nanoseconds.
     */
    long long Now();

    /**
     * Record a reference reading for the next comparison.
     *
     * @param nNanoseconds The reading to record.
     */
    void Mark(long long nNanoseconds);

    /**
     * Stop the clock, remembering how far it had run.
     *
     * Does nothing while the clock is already stopped. GameManagerImpl's pause handler is the
     * caller. The title is inferred.
     */
    void Pause();

    /**
     * Restart a stopped clock from where it stopped.
     *
     * Moves mStartMs so that the elapsed time excludes the pause. Does nothing while the clock is
     * running. GameManagerImpl's unpause handler is the caller. The title is inferred.
     */
    void Resume();

    /**
     * Stop the clock and move its paused running time forward.
     *
     * Pause() is inlined first. The running time then grows by nAmount times one million divided
     * by mNsPerUnit. HxScript::Clock() is the caller. The title is inferred.
     *
     * @param nAmount The amount to advance by.
     */
    void Advance(int nAmount);

private:
    /** The millisecond reading the current run started at. `+0x10` */
    long long mOriginMs;

    double mNsPerUnit; // +0x00 nanoseconds in one clock unit

    long long mStartMs; // +0x08 the reading the running time is measured from

    int mRunning; // +0x18 set to 1 on construction, cleared while paused

    int mReserved1c; // +0x1c never accessed

    long long mPausedRunMs; // +0x20 the running time at the last pause, cleared on construction
};

} // namespace Sch
