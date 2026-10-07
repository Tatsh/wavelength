#pragma once

#include "rnd/view.h"

/**
 * Player of the animation of a view that reveals text, with the `TEXT` sound.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Once started, Poll() runs
 * the view from the start time and plays the `TEXT` cue of FxMidi while any of the view's
 * animations is between its first and its last frame. The routines of the class are not
 * reconstructed.
 */
class TextAnimPlayer {
public:
    /**
     * Construct a player with no view, stopped.
     *
     * @ghidraAddress NTSC-U/C: 0x0019d1b8
     * @ghidraAddress PAL: 0x001a4ed0
     */
    TextAnimPlayer();

    /**
     * Set the view, at its first frame, and record its length.
     *
     * @param pView The view.
     * @ghidraAddress NTSC-U/C: 0x0019d1c8
     * @ghidraAddress PAL: 0x001a4ee0
     */
    void SetView(Rnd::View *pView);

    /**
     * Start the animation at a time.
     *
     * @param flTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019d208
     * @ghidraAddress PAL: 0x001a4f20
     */
    void Start(float flTime);

    /**
     * Stop the animation and its sound.
     *
     * @ghidraAddress NTSC-U/C: 0x0019d248
     * @ghidraAddress PAL: 0x001a4f60
     */
    void Stop();

    /**
     * Run the animation to a time, and start or stop its sound.
     *
     * @param flTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019d268
     * @ghidraAddress PAL: 0x001a4f80
     */
    void Poll(float flTime);

    /**
     * Report whether the animation runs.
     *
     * @return Whether it was started and has not finished.
     * @ghidraAddress NTSC-U/C: 0x0019d3e0
     * @ghidraAddress PAL: 0x001a50f8
     */
    bool IsPlaying() const;

    /**
     * Stop the animation and forget the view.
     *
     * @ghidraAddress NTSC-U/C: 0x0019d408
     * @ghidraAddress PAL: 0x001a5120
     */
    void Clear();

    float mStart;     /*!< The time the animation started, or 0. */
    float mLength;    /*!< The length of the animation. */
    int mSoundOn;     /*!< Whether the `TEXT` cue plays. */
    Rnd::View *mView; /*!< The view, or null. */
};
