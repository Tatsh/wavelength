#include "gfx/quadraticapproach.h"

QuadraticApproach::QuadraticApproach(const Vector3 &coeffs, float fValue, float fMaxSpeed) {
    mMaxSpeed = fMaxSpeed;
    mValue = fValue;
    mSquare = coeffs.x;
    mLinear = coeffs.y;
    mConstant = coeffs.z;
}

QuadraticApproach::~QuadraticApproach() = default;

float QuadraticApproach::Step(float fDelta, float fTarget) {
    float fDistance = fTarget - mValue;
    bool bDown = false;
    if (fDistance < 0.0f) {
        fDistance = -fDistance;
        bDown = true;
    }
    float fSpeed = (mSquare * fDistance * fDistance) + (mLinear * fDistance) + mConstant;
    if (mMaxSpeed < fSpeed) {
        fSpeed = mMaxSpeed;
    } else if (fSpeed < 0.0f) {
        return 0.0f;
    }
    float fStep = fDelta * fSpeed;
    if (fDistance < fStep) {
        fStep = fDistance;
    }
    if (!bDown) {
        mValue += fStep;
        return fSpeed;
    }
    mValue -= fStep;
    return -fSpeed;
}
