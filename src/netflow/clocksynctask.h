#pragma once

#include <vector>

#include "os/task.h"

/**
 * Task of a guest that measures the clock of the host.
 *
 * The RTTI includes the name and records Task as the base. The vtable is at `0x003d4998`, and the
 * object is 0x1c bytes. The task asks the host for the time every kRequestInterval polls and keeps
 * the answers. Once kSampleCount answers arrived, it gives JoinpadRT the median offset of the
 * clocks.
 */
class ClockSyncTask : public Task {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039a4b0
     * @ghidraAddress PAL: 0x00408d38
     */
    ~ClockSyncTask() override = default;

    /**
     * One answer of the host.
     *
     * The RTTI names the type, which a standard library vector records.
     */
    class Sample {
    public:
        /**
         * Report whether this answer has a smaller offset than another, the order of the median.
         *
         * Inline.
         *
         * @param other The other answer.
         * @return Whether mOffset is smaller.
         */
        bool operator<(const Sample &other) const {
            return mOffset < other.mOffset;
        }

        float mRoundTrip; /*!< The time from the request to the answer, in milliseconds. */
        float mOffset;    /*!< The clock of the host less the clock of this console. */
    };

    /** The polls between two requests. */
    static constexpr int kRequestInterval = 10;

    /** The answers the task waits for. */
    static constexpr int kSampleCount = 5;

    /**
     * Note an answer of the host while the task runs, and finish once the answers suffice.
     *
     * The name is inferred.
     *
     * @param fRequestTime The clock of this console when it asked.
     * @param fHostTime The clock of the host when it answered.
     * @ghidraAddress NTSC-U/C: 0x0025c800
     * @ghidraAddress PAL: 0x00265860
     */
    void AddSample(float fRequestTime, float fHostTime);

    /**
     * Finish the task and give JoinpadRT the median offset and the mean round trip.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0025caa8
     * @ghidraAddress PAL: 0x00265b08
     */
    void FinishSync();

    std::vector<Sample> mSamples; /*!< The answers. */
    int mCountdown;               /*!< The polls until the next request. */

protected:
    /**
     * Forget the answers, and ask at the next poll.
     *
     * @ghidraAddress NTSC-U/C: 0x0025c6e8
     * @ghidraAddress PAL: 0x00265748
     */
    void OnStart() override;

    /**
     * Ask the host for the time every kRequestInterval polls.
     *
     * @ghidraAddress NTSC-U/C: 0x0025c728
     * @ghidraAddress PAL: 0x00265788
     */
    void OnPoll() override;
};
