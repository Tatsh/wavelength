#pragma once

#include "script/dataarray.h"

/**
 * Map from an input range to an output range, clamped at both ends.
 *
 * The RTTI includes the class name. The four end points come first and the vptr follows them at
 * `+0x10`. The class is abstract.
 */
class Interpolator {
public:
    /** Release the interpolator. */
    virtual ~Interpolator() {
    }

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

    float mY0; /*!< The output at the start of the range. */
    float mY1; /*!< The output at the end of the range. */
    float mX0; /*!< The start of the input range. */
    float mX1; /*!< The end of the input range. */
};

/**
 * Build the interpolator a configuration array describes.
 *
 * The first symbol selects the class: `linear`, `exp`, `invexp`, `atan`, `table`, or `tablelin`.
 * The first four numbers that follow are the end points in Reset() order. `exp` and `invexp` take
 * an optional exponent (2 by default) and `atan` an optional severity (10 by default). `table` and
 * `tablelin` take the input range and then the table entries.
 *
 * @param pArray The array.
 * @return The new interpolator, or null after a warning for an unknown class.
 * @ghidraAddress NTSC-U/C: 0x002911c0
 * @ghidraAddress PAL: 0x0029ab88
 */
Interpolator *ObjectToInterpolator(const DataArray *pArray);

/**
 * Straight-line interpolator.
 *
 * The RTTI records the class as deriving from Interpolator. The object is 0x1c bytes.
 */
class LinearInterpolator : public Interpolator {
public:
    /**
     * Construct an interpolator.
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
     * Replace the end points.
     *
     * An input range narrower than 1e-6 gives a slope of 0.
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
 * Interpolator whose output follows a power of the input's position in the range.
 *
 * The RTTI records the class as deriving from Interpolator. The object is 0x20 bytes.
 */
class ExpInterpolator : public Interpolator {
public:
    /**
     * Construct an interpolator.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @param fExponent The power.
     * @ghidraAddress NTSC-U/C: 0x00291668
     * @ghidraAddress PAL: 0x0029b030
     */
    ExpInterpolator(float fY0, float fY1, float fX0, float fX1, float fExponent);

    /**
     * Replace the end points and the power.
     *
     * An input range narrower than 1e-6 gives a scale of 1.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @param fExponent The power.
     * @ghidraAddress NTSC-U/C: 0x002916a0
     * @ghidraAddress PAL: 0x0029b068
     */
    void Reset(float fY0, float fY1, float fX0, float fX1, float fExponent);

    /**
     * Replace the end points and preserve the power.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @ghidraAddress NTSC-U/C: 0x002916f8
     * @ghidraAddress PAL: 0x0029b0c0
     */
    void Reset(float fY0, float fY1, float fX0, float fX1) override;

    /**
     * Map an input along the curve.
     *
     * @param fX The input.
     * @return mY0 plus mRange times the input's position in the range raised to mExponent.
     * @ghidraAddress NTSC-U/C: 0x00291718
     * @ghidraAddress PAL: 0x0029b0e0
     */
    float Interp(float fX) override;

    float mExponent;  /*!< The power. */
    float mRange;     /*!< mY1 less mY0. */
    float mInvXRange; /*!< The reciprocal of the input range. */
};

/**
 * Interpolator that mirrors ExpInterpolator, easing out of the start of the range.
 *
 * The RTTI records the class as deriving from Interpolator. The object is 0x20 bytes.
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
     * @param fExponent The power.
     * @ghidraAddress NTSC-U/C: 0x00291760
     * @ghidraAddress PAL: 0x0029b128
     */
    InvExpInterpolator(float fY0, float fY1, float fX0, float fX1, float fExponent);

    /**
     * Replace the end points and the power.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @param fExponent The power.
     * @ghidraAddress NTSC-U/C: 0x00291798
     * @ghidraAddress PAL: 0x0029b160
     */
    void Reset(float fY0, float fY1, float fX0, float fX1, float fExponent);

    /**
     * Replace the end points and preserve the power.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @ghidraAddress NTSC-U/C: 0x002917f0
     * @ghidraAddress PAL: 0x0029b1b8
     */
    void Reset(float fY0, float fY1, float fX0, float fX1) override;

    /**
     * Map an input along the curve.
     *
     * @param fX The input.
     * @return mY0 plus mRange times one less the power of one less the input's position.
     * @ghidraAddress NTSC-U/C: 0x00291810
     * @ghidraAddress PAL: 0x0029b1d8
     */
    float Interp(float fX) override;

    float mExponent;  /*!< The power. */
    float mRange;     /*!< mY1 less mY0. */
    float mInvXRange; /*!< The reciprocal of the input range. */
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
     * Replace the end points and preserve the severity.
     *
     * @param fY0 The output at the start of the range.
     * @param fY1 The output at the end of the range.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @ghidraAddress NTSC-U/C: 0x002919d8
     * @ghidraAddress PAL: 0x0029b3a0
     */
    void Reset(float fY0, float fY1, float fX0, float fX1) override;

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

/**
 * Interpolator that reads the nearest entry of an evenly spaced table.
 *
 * The RTTI records the class as deriving from Interpolator. The object is 0x24 bytes.
 */
class TableInterpolator : public Interpolator {
public:
    /**
     * Construct an interpolator with a zeroed table.
     *
     * @param nCount The table entries.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @ghidraAddress NTSC-U/C: 0x00291a40
     * @ghidraAddress PAL: 0x0029b408
     */
    TableInterpolator(int nCount, float fX0, float fX1);

    /**
     * Construct an interpolator from the numbers of an array.
     *
     * The end outputs are taken before the table is filled. They are therefore those of the
     * zeroed table.
     *
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @param pArray The array.
     * @param nFirst The index of the first table entry in the array.
     * @ghidraAddress NTSC-U/C: 0x00291a90
     * @ghidraAddress PAL: 0x0029b458
     */
    TableInterpolator(float fX0, float fX1, const DataArray *pArray, int nFirst);

    /**
     * Release the table.
     *
     * @ghidraAddress NTSC-U/C: 0x00291b30
     * @ghidraAddress PAL: 0x0029b4f8
     */
    ~TableInterpolator() override;

    /**
     * Read the entry nearest an input.
     *
     * @param fX The input.
     * @return The entry, the first or the last outside the range.
     * @ghidraAddress NTSC-U/C: 0x00291db0
     */
    float Interp(float fX) override;

    /**
     * Report that a table cannot be rebuilt from end points.
     *
     * @param fY0 Not used.
     * @param fY1 Not used.
     * @param fX0 Not used.
     * @param fX1 Not used.
     * @ghidraAddress NTSC-U/C: 0x00291d90
     * @ghidraAddress PAL: 0x0029b758
     */
    void Reset(float fY0, float fY1, float fX0, float fX1) override;

    /**
     * Fill the table by sampling another interpolator evenly across its range.
     *
     * @param source The interpolator to sample.
     * @param nCount The table entries.
     * @ghidraAddress NTSC-U/C: 0x00291b98
     * @ghidraAddress PAL: 0x0029b560
     */
    virtual void Resample(Interpolator &source, int nCount);

    /**
     * Size the table, zeroing it when it is reallocated, and set the input range.
     *
     * @param nCount The table entries.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @ghidraAddress NTSC-U/C: 0x00291cb8
     * @ghidraAddress PAL: 0x0029b680
     */
    virtual void SetSize(int nCount, float fX0, float fX1);

    /**
     * Recompute the spacing of the entries from the input range.
     *
     * @ghidraAddress NTSC-U/C: 0x00291e18
     * @ghidraAddress PAL: 0x0029b7e0
     */
    virtual void Update();

    float mStep;     /*!< The input range covered by one entry. */
    float mInvStep;  /*!< The entries per unit of input, or 1 for a range narrower than 1e-6. */
    float *mTable{}; /*!< The entries. */
    int mCount{};    /*!< The entries of mTable. */
};

/**
 * Interpolator that blends linearly between the two entries of a table around an input.
 *
 * The RTTI records the class as deriving from TableInterpolator. The object is 0x28 bytes.
 */
class TableLinInterpolator : public TableInterpolator {
public:
    /**
     * Construct an interpolator by sampling another one.
     *
     * @param source The interpolator to sample.
     * @param nCount The table entries.
     * @ghidraAddress NTSC-U/C: 0x00291e80
     * @ghidraAddress PAL: 0x0029b848
     */
    TableLinInterpolator(Interpolator &source, int nCount);

    /**
     * Construct an interpolator from the numbers of an array.
     *
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @param pArray The array.
     * @param nFirst The index of the first table entry in the array.
     * @ghidraAddress NTSC-U/C: 0x00291ef8
     * @ghidraAddress PAL: 0x0029b8c0
     */
    TableLinInterpolator(float fX0, float fX1, const DataArray *pArray, int nFirst);

    /**
     * Release the slopes.
     *
     * @ghidraAddress NTSC-U/C: 0x00291fd8
     * @ghidraAddress PAL: 0x0029b9a0
     */
    ~TableLinInterpolator() override;

    /**
     * Blend the two entries around an input.
     *
     * @param fX The input.
     * @return The blend, the first or the last entry outside the range.
     * @ghidraAddress NTSC-U/C: 0x002921a8
     */
    float Interp(float fX) override;

    /**
     * Size the slopes, then fill the table by sampling another interpolator.
     *
     * @param source The interpolator to sample.
     * @param nCount The table entries.
     * @ghidraAddress NTSC-U/C: 0x00292030
     * @ghidraAddress PAL: 0x0029b9f8
     */
    void Resample(Interpolator &source, int nCount) override;

    /**
     * Size the slopes and the table, and set the input range.
     *
     * @param nCount The table entries.
     * @param fX0 The start of the input range.
     * @param fX1 The end of the input range.
     * @ghidraAddress NTSC-U/C: 0x002920b0
     * @ghidraAddress PAL: 0x0029ba78
     */
    void SetSize(int nCount, float fX0, float fX1) override;

    /**
     * Recompute the spacing and the difference between each pair of neighbouring entries.
     *
     * @ghidraAddress NTSC-U/C: 0x00292140
     * @ghidraAddress PAL: 0x0029bb08
     */
    void Update() override;

    float *mSlopes; /*!< Each entry less the one before it, one fewer than the entries. */
};
