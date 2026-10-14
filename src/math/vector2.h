#pragma once

#include "os/prnstream.h"

/**
 * Two-component vector, used for texture coordinates.
 *
 * The class is not polymorphic and emits no RTTI descriptor. A mesh vertex stores two vectors
 * back to back at its end, and the binary loads and writes exactly two floats for each. The type
 * is therefore not padded to a quadword.
 */
struct Vector2 {
    float x;
    float y;
};

/**
 * Write a vector as `(x: y:)`.
 *
 * @param stream The stream to write to.
 * @param vector The vector.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00292f98
 * @ghidraAddress PAL: 0x0029c960
 */
PrnStream &operator<<(PrnStream &stream, const Vector2 &vector);

namespace Rnd {

/**
 * Add two two-component vectors.
 *
 * The destination may alias either source because both loads precede both stores. The routine
 * reads y before x and stores y before x, with the x store in the return delay slot.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @param out Receives a plus b, and may alias either input.
 */
void Add(const Vector2 &a, const Vector2 &b, Vector2 &out);

/**
 * Scale a two-component vector.
 *
 * The destination may alias the source because both loads precede both stores. The scale arrives
 * in the first floating-point argument register.
 *
 * @param v The vector.
 * @param flScale The scale.
 * @param out Receives v times flScale.
 */
void Multiply(const Vector2 &v, float flScale, Vector2 &out);

/**
 * Negate a two-component vector.
 *
 * The destination may alias the source because both loads precede both stores.
 *
 * @param v The vector.
 * @param out Receives the negated vector.
 */
void Negate(const Vector2 &v, Vector2 &out);

/**
 * Scale a two-component vector to unit length.
 *
 * The zero vector yields the zero vector rather than a division by zero.
 *
 * @param v The vector.
 * @param out Receives the unit vector.
 */
void Normalize(const Vector2 &v, Vector2 &out);

/**
 * Report the length of a two-component vector.
 *
 * @param v The vector.
 * @return The length.
 */
float Length(const Vector2 &v);

} // namespace Rnd

/**
 * Subtract one two-component vector from another.
 *
 * The destination may alias either source because both loads precede both stores. The routine
 * reads y before x and stores y before x, with the x store in the return delay slot, matching
 * Rnd::Add().
 *
 * @param pA The vector subtracted from.
 * @param pB The vector subtracted.
 * @param pOut Receives pA minus pB, and may alias either input.
 */
void SubVec2(const float *pA, const float *pB, float *pOut);
