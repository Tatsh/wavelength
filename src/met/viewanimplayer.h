#pragma once

#include "rnd/animatable.h"

/**
 * Player of the entry animation of a view, which loops the cursor sound while a cursor of the
 * animation moves.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object is 0x10 bytes.
 * The front-end screens and panels that animate a view embed one.
 */
class ViewAnimPlayer {
public:
    /**
     * Construct a player with no animation.
     *
     * @ghidraAddress NTSC-U/C: 0x0019d1b8
     * @ghidraAddress PAL: 0x001a4ed0
     */
    ViewAnimPlayer();

    /**
     * Use an animation, and move it to its first frame.
     *
     * @param pAnim The animation.
     * @ghidraAddress NTSC-U/C: 0x0019d1c8
     * @ghidraAddress PAL: 0x001a4ee0
     */
    void SetAnim(Rnd::Animatable *pAnim);

    /**
     * Start playing the animation from its first frame.
     *
     * @param fNowMs The current time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019d208
     * @ghidraAddress PAL: 0x001a4f20
     */
    void Start(float fNowMs);

    /**
     * Stop playing, and stop the cursor sound.
     *
     * @ghidraAddress NTSC-U/C: 0x0019d248
     * @ghidraAddress PAL: 0x001a4f60
     */
    void Stop();

    /**
     * Move the animation to the current time, stop at its last frame, and loop the cursor sound
     * while a cursor of the animation is between its first and its last frame.
     *
     * @param fNowMs The current time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019d268
     * @ghidraAddress PAL: 0x001a4f80
     */
    void Poll(float fNowMs);

    /**
     * Report whether the animation plays.
     *
     * @return Whether the animation plays.
     * @ghidraAddress NTSC-U/C: 0x0019d3e0
     * @ghidraAddress PAL: 0x001a50f8
     */
    bool IsPlaying();

    /**
     * Stop playing and forget the animation.
     *
     * @ghidraAddress NTSC-U/C: 0x0019d408
     * @ghidraAddress PAL: 0x001a5120
     */
    void Clear();

    float mStartMs;         /*!< The time the animation started, or 0 while it does not play. */
    float mEndFrame;        /*!< The last frame of the animation. */
    int mCursorMoving;      /*!< Non-zero while the cursor sound loops. */
    Rnd::Animatable *mAnim; /*!< The animation, or null. */
};
