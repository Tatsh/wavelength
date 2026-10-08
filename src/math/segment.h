#pragma once

/**
 * Line segment a collision query is run along.
 *
 * The type has no RTTI, and its name is inferred. Each end is a 16-byte vector whose fourth float
 * is padding.
 */
struct Segment {
    float mStart[4]; /*!< Origin. The fourth float is padding. */
    float mEnd[4];   /*!< Far end. The fourth float is padding. */
};
