#pragma once

#include "math/vector3.h"

/**
 * Value that moves toward a target at a speed quadratic in the distance left.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0x14 bytes.
 */
class QuadraticApproach {
public:
    /**
     * Construct a value at rest.
     *
     * @param coeffs The coefficients of the speed, the square term in x, the linear term in y,
     *               and the constant in z.
     * @param fValue The value.
     * @param fMaxSpeed The greatest speed.
     * @ghidraAddress NTSC-U/C: 0x001e2bb0
     * @ghidraAddress PAL: 0x001eb950
     */
    QuadraticApproach(const Vector3 &coeffs, float fValue, float fMaxSpeed);

    /**
     * Destroy the value.
     *
     * @ghidraAddress NTSC-U/C: 0x001e2bd8
     * @ghidraAddress PAL: 0x001eb978
     */
    ~QuadraticApproach();

    /**
     * Move the value toward a target without passing it.
     *
     * The speed is the quadratic of the distance to the target, limited to the greatest speed. A
     * negative speed leaves the value where it is.
     *
     * @param fDelta The time since the last step.
     * @param fTarget The target.
     * @return The speed, negative when the value moves down, or 0 when it does not move.
     * @ghidraAddress NTSC-U/C: 0x001e2c00
     * @ghidraAddress PAL: 0x001eb9a0
     */
    float Step(float fDelta, float fTarget);

    float mSquare;   /*!< The coefficient of the square of the distance. */
    float mLinear;   /*!< The coefficient of the distance. */
    float mConstant; /*!< The speed at no distance. */
    float mMaxSpeed; /*!< The greatest speed. */
    float mValue;    /*!< The value. */
};
