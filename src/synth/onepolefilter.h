#pragma once

/**
 * Value that moves toward a target by a fixed fraction of the distance at each step.
 *
 * The RTTI includes the class name. The class is not polymorphic, and the object is 0xc bytes.
 * SynthPS2 smooths the volume of each MIDI channel and the sweep of the software filter with one.
 */
class OnePoleFilter {
public:
    /** Construct a filter at 0 that jumps straight to its target. */
    OnePoleFilter() : mCoefficient(1.0f), mValue(0.0f), mTarget(0.0f) {
    }

    /**
     * Set the value the filter moves toward.
     *
     * @param fTarget The target.
     * @ghidraAddress NTSC-U/C: 0x003969a0
     * @ghidraAddress PAL: 0x00405060
     */
    void SetTarget(float fTarget) {
        mTarget = fTarget;
    }

    /**
     * Move the value one step toward the target.
     *
     * @return The new value.
     */
    float Update() {
        mValue = mValue + mCoefficient * (mTarget - mValue);
        return mValue;
    }

    float mCoefficient; /*!< The fraction of the distance covered at each step. */
    float mValue;       /*!< The current value. */
    float mTarget;      /*!< The value the filter moves toward. */
};
