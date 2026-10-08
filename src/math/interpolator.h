#pragma once

#include "script/dataarray.h"

/**
 * Map from an input range to an output range, clamped at both ends.
 *
 * The RTTI includes the class name. The four end points come first and the vptr follows them at
 * `+0x10`. The class is abstract. Only the members the front end uses are declared.
 */
class Interpolator {
public:
    /**
     * Map an input inside the range.
     *
     * @param fX The input.
     * @return The output.
     */
    virtual float Interp(float fX) = 0;

    /**
     * Map an input, clamping it to the range first.
     *
     * An input at or before mX0 gives mY0, and one at or after mX1 gives mY1.
     *
     * @param fX The input.
     * @return The output.
     * @ghidraAddress NTSC-U/C: 0x00291160
     * @ghidraAddress PAL: 0x0029ab28
     */
    virtual float Eval(float fX);

    /**
     * Replace the end points.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     */
    virtual void Reset(float fY0, float fY1, float fX0, float fX1) = 0;

    /** Destroy the interpolator. */
    virtual ~Interpolator() = default;

    /**
     * Create an interpolator of the type a script description names.
     *
     * @param pData The description.
     * @return The new interpolator.
     * @ghidraAddress NTSC-U/C: 0x002911c0
     * @ghidraAddress PAL: 0x0029ab88
     */
    static Interpolator *Create(DataArray *pData);

    float mY0; /*!< The output at the start of the range. */
    float mY1; /*!< The output at the end of the range. */
    float mX0; /*!< The start of the input range. */
    float mX1; /*!< The end of the input range. */
};

/**
 * Straight-line interpolator.
 *
 * The RTTI records the class as deriving from Interpolator. The object is 0x1c bytes.
 */
class LinearInterpolator : public Interpolator {
public:
    /**
     * Construct an interpolator through two points.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @ghidraAddress NTSC-U/C: 0x002915b0
     * @ghidraAddress PAL: 0x0029af78
     */
    LinearInterpolator(float fY0, float fY1, float fX0, float fX1);

    /**
     * Replace the end points and recompute the line through them.
     *
     * An input range narrower than 1e-6 gives a flat line through fY0.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @ghidraAddress NTSC-U/C: 0x002915e8
     * @ghidraAddress PAL: 0x0029afb0
     */
    void Reset(float fY0, float fY1, float fX0, float fX1) override;

    /**
     * Map an input along the line.
     *
     * @param fX The input.
     * @return mSlope times the input plus mOffset.
     * @ghidraAddress NTSC-U/C: 0x00291650
     * @ghidraAddress PAL: 0x0029b018
     */
    float Interp(float fX) override;

    float mSlope;  /*!< The change of the output per unit of input. */
    float mOffset; /*!< The output at an input of 0. */
};

/**
 * Interpolator that starts fast and eases into the end of the range.
 *
 * The RTTI records the class as deriving from Interpolator. The object is 0x20 bytes. The output
 * is mY0 plus mRange times one less the remaining part of the range raised to mPower.
 */
class InvExpInterpolator : public Interpolator {
public:
    /**
     * Construct an interpolator.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @param fPower The power of the ease.
     * @ghidraAddress NTSC-U/C: 0x00291760
     * @ghidraAddress PAL: 0x0029b128
     */
    InvExpInterpolator(float fY0, float fY1, float fX0, float fX1, float fPower);

    /**
     * Replace the end points and the power.
     *
     * An input range narrower than 1e-6 is taken as one unit wide.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @param fPower The power of the ease.
     * @ghidraAddress NTSC-U/C: 0x00291798
     * @ghidraAddress PAL: 0x0029b160
     */
    void Reset(float fY0, float fY1, float fX0, float fX1, float fPower);

    /**
     * Replace the end points and keep the power.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @ghidraAddress NTSC-U/C: 0x002917f0
     * @ghidraAddress PAL: 0x0029b1b8
     */
    void Reset(float fY0, float fY1, float fX0, float fX1) override {
        Reset(fY0, fY1, fX0, fX1, mPower);
    }

    /**
     * Map an input along the curve.
     *
     * @param fX The input.
     * @return The output.
     * @ghidraAddress NTSC-U/C: 0x00291810
     * @ghidraAddress PAL: 0x0029b1d8
     */
    float Interp(float fX) override;

    float mPower;   /*!< The power of the ease. */
    float mRange;   /*!< mY1 less mY0. */
    float mInvSpan; /*!< One over the width of the input range. */
};

/**
 * Interpolator that follows an arctangent, which eases in and out of the range.
 *
 * The RTTI records the class as deriving from Interpolator. The object is 0x3c bytes.
 */
class ATanInterpolator : public Interpolator {
public:
    /**
     * Construct an interpolator.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @param fSeverity How sharply the curve turns. A larger value eases harder.
     * @ghidraAddress NTSC-U/C: 0x00291870
     * @ghidraAddress PAL: 0x0029b238
     */
    ATanInterpolator(float fY0, float fY1, float fX0, float fX1, float fSeverity);

    /**
     * Replace the end points and the severity.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @param fSeverity How sharply the curve turns.
     * @ghidraAddress NTSC-U/C: 0x00291918
     * @ghidraAddress PAL: 0x0029b2e0
     */
    void Reset(float fY0, float fY1, float fX0, float fX1, float fSeverity);

    /**
     * Replace the end points and keep the severity.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @ghidraAddress NTSC-U/C: 0x002919d8
     * @ghidraAddress PAL: 0x0029b3a0
     */
    void Reset(float fY0, float fY1, float fX0, float fX1) override {
        Reset(fY0, fY1, fX0, fX1, mSeverity);
    }

    /**
     * Map an input along the curve.
     *
     * @param fX The input.
     * @return The output.
     * @ghidraAddress NTSC-U/C: 0x002919f8
     * @ghidraAddress PAL: 0x0029b3c0
     */
    float Interp(float fX) override;

    LinearInterpolator mLinear; /*!< Maps the input onto the arctangent's argument. */
    float mScale;               /*!< The factor applied to the arctangent. */
    float mOffset;              /*!< The value added after the factor. */
    float mSeverity;            /*!< The severity Reset() received. */
};
