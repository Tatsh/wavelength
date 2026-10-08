#pragma once

#include "math/interpolator.h"
#include "math/transform.h"
#include "rnd/transanim.h"

/**
 * The path the tunnel follows through the world, a transform animation that blends into the next.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0x4c bytes. TnlGeom places
 * every cross section of the tunnel along the path.
 */
class TnlPath {
public:
    /** One transform animation of the path, 0x18 bytes. */
    class Segment {
    public:
        /**
         * Clear the segment.
         *
         * @ghidraAddress NTSC-U/C: 0x001cc1b8
         * @ghidraAddress PAL: 0x001d4f58
         */
        void Reset();

        /**
         * Work out the transform of the segment.
         *
         * A looping segment wraps the frame into its loop, and a frame past the last key extends
         * the last move in a straight line.
         *
         * @param flTick The tick of the song.
         * @param flTime The time of the song in milliseconds.
         * @param pXfm Receives the transform.
         * @ghidraAddress NTSC-U/C: 0x001cc1d8
         * @ghidraAddress PAL: 0x001d4f78
         */
        void Xfm(float flTick, float flTime, Transform *pXfm) const;

        Rnd::TransAnim *mAnim; /*!< The animation, or null for no path. */
        int mLoop;             /*!< Whether the animation loops between mLoopStart and mLoopEnd. */
        int mUseTime;          /*!< Whether the frame follows the time rather than the tick. */
        float mOffset;         /*!< The frame of the animation at tick or time 0. */
        float mLoopStart;      /*!< The first frame of the loop. */
        float mLoopEnd;        /*!< The frame the loop wraps at. */
    };

    /**
     * Construct a path with no animation.
     *
     * @ghidraAddress NTSC-U/C: 0x001cbee0
     * @ghidraAddress PAL: 0x001d4c80
     */
    TnlPath();

    /**
     * Replace the animation at once.
     *
     * @param pAnim The animation, or null for no path.
     * @param bLoop Whether the animation loops.
     * @param bUseTime Whether the frame follows the time rather than the tick.
     * @param flStart The tick or time the animation starts at.
     * @param flFrame The frame of the animation at flStart.
     * @param flLoopStart The first frame of the loop.
     * @param flLoopEnd The frame the loop wraps at.
     * @ghidraAddress NTSC-U/C: 0x001cbf60
     * @ghidraAddress PAL: 0x001d4d00
     */
    void Set(Rnd::TransAnim *pAnim,
             int bLoop,
             int bUseTime,
             float flStart,
             float flFrame,
             float flLoopStart,
             float flLoopEnd);

    /**
     * Blend from the animation in use to a new one.
     *
     * @param pAnim The new animation.
     * @param bLoop Whether the new animation loops.
     * @param bUseTime Whether the frame of the new animation follows the time.
     * @param flLength The length of the blend in ticks.
     * @param flStart The tick the blend starts at.
     * @param flFrame The frame of the new animation at flStart.
     * @param flLoopStart The first frame of the loop of the new animation.
     * @param flLoopEnd The frame the loop of the new animation wraps at.
     * @ghidraAddress NTSC-U/C: 0x001cc010
     * @ghidraAddress PAL: 0x001d4db0
     */
    void BlendTo(Rnd::TransAnim *pAnim,
                 int bLoop,
                 int bUseTime,
                 float flLength,
                 float flStart,
                 float flFrame,
                 float flLoopStart,
                 float flLoopEnd);

    /**
     * Work out the transform of the path.
     *
     * @param flTick The tick of the song.
     * @param flTime The time of the song in milliseconds.
     * @param pXfm Receives the transform.
     * @ghidraAddress NTSC-U/C: 0x001cc098
     * @ghidraAddress PAL: 0x001d4e38
     */
    void Xfm(float flTick, float flTime, Transform *pXfm);

    /** The segment that is blended out, then the segment in use. */
    Segment mSegments[2];

    /** The blend from the first segment to the second over ticks. */
    LinearInterpolator mBlend;
};
