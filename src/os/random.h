#pragma once

/**
 * Seed of NextRandomValue(), 1 until the first draw.
 *
 * NextRandomValue() is the only reader and the only writer.
 */
extern int gRandom;

/**
 * Advance the shared seed and return it.
 *
 * The step is the RANDU generator, the seed times 65539 masked to 31 bits, so every result is
 * non-negative and odd.
 *
 * @return The new seed.
 */
int NextRandomValue();
