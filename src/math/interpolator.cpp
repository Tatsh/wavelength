#include "math/interpolator.h"

#include <cmath>

namespace {

// An input range narrower than this gives a flat line.
constexpr float kMinSpan = 1e-6f;

} // namespace

LinearInterpolator::LinearInterpolator(float fY0, float fY1, float fX0, float fX1) {
    LinearInterpolator::Reset(fY0, fY1, fX0, fX1);
}

void LinearInterpolator::Reset(float fY0, float fY1, float fX0, float fX1) {
    const float fSpan = fX1 - fX0;
    mX0 = fX0;
    mX1 = fX1;
    mY0 = fY0;
    mY1 = fY1;
    if (std::fabs(fSpan) < kMinSpan) {
        mSlope = 0.0f;
    } else {
        mSlope = (fY1 - fY0) / fSpan;
    }
    mOffset = -mX0 * mSlope + mY0;
}

InvExpInterpolator::InvExpInterpolator(float fY0, float fY1, float fX0, float fX1, float fPower) {
    InvExpInterpolator::Reset(fY0, fY1, fX0, fX1, fPower);
}

void InvExpInterpolator::Reset(float fY0, float fY1, float fX0, float fX1, float fPower) {
    const float fSpan = fX1 - fX0;
    mX0 = fX0;
    mX1 = fX1;
    mY0 = fY0;
    mY1 = fY1;
    mInvSpan = std::fabs(fSpan) < kMinSpan ? 1.0f : 1.0f / fSpan;
    mPower = fPower;
    mRange = fY1 - fY0;
}

float InvExpInterpolator::Interp(float fX) {
    const float fCurve = std::pow(1.0f - (fX - mX0) * mInvSpan, mPower);
    return (1.0f - fCurve) * mRange + mY0;
}
