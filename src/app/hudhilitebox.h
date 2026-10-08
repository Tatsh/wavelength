#pragma once

#include "math/interpolator.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/view.h"

/**
 * Highlight the tutorial draws round a part of the screen, and an arrow that points at a place.
 *
 * The class is not polymorphic and emits no RTTI. The object is 0x138 bytes. The box is a mesh of
 * sixteen vertices, a corner of fixed size at each end of each edge, that stretches to the
 * rectangle asked for. The box and the arrow fade and move against the song tick.
 */
class HudHiliteBox {
public:
    /** The bits of mChanging, which say what Update() still moves. */
    enum Changing {
        kChangingBoxFade = 0x01,   /*!< The box fades. */
        kChangingBoxMove = 0x02,   /*!< The box moves and stretches. */
        kChangingArrowFade = 0x04, /*!< The arrow fades. */
        kChangingArrowMove = 0x08, /*!< The arrow moves and turns. */
    };

    /** The number of vertices of the box. */
    static constexpr int kNumBoxVerts = 16;

    /**
     * Construct a hidden box and arrow from `HUD hilite_box.mesh`, `HUD hilite_box.mat`,
     * `HUD hilite arrow.view`, and `HUD hilite arrow.mat`.
     *
     * @ghidraAddress NTSC-U/C: 0x001bb6f8
     * @ghidraAddress PAL: 0x001c4498
     */
    HudHiliteBox();

    /**
     * Hide the box and the arrow at once and stop every change.
     *
     * @ghidraAddress NTSC-U/C: 0x001bba80
     * @ghidraAddress PAL: 0x001c4820
     */
    void Reset();

    /**
     * Convert a rectangle in the unit square of the screen to the coordinates of the display.
     *
     * @param pfX0 The left edge, converted in place.
     * @param pfY0 The top edge, converted in place.
     * @param pfX1 The right edge, converted in place.
     * @param pfY1 The bottom edge, converted in place.
     * @ghidraAddress NTSC-U/C: 0x001bbba8
     * @ghidraAddress PAL: 0x001c4948
     */
    static void UnitToScreen(float *pfX0, float *pfY0, float *pfX1, float *pfY1);

    /**
     * Move and stretch the box to a rectangle of the screen.
     *
     * @param fX0 The left edge, in the unit square of the screen.
     * @param fY0 The top edge.
     * @param fX1 The right edge.
     * @param fY1 The bottom edge.
     * @param fDuration The ticks the move takes.
     * @ghidraAddress NTSC-U/C: 0x001bbc18
     * @ghidraAddress PAL: 0x001c49b8
     */
    void SetBoxRect(float fX0, float fY0, float fX1, float fY1, float fDuration);

    /**
     * Fade the box in or out over 480 ticks.
     *
     * @param bShow Whether to show the box.
     * @ghidraAddress NTSC-U/C: 0x001bbdb0
     * @ghidraAddress PAL: 0x001c4b50
     */
    void ShowBox(bool bShow);

    /**
     * Move and turn the arrow to point at a place of the screen.
     *
     * @param fX The horizontal place, in the unit square of the screen.
     * @param fY The vertical place.
     * @param fAngle The direction of the arrow, in degrees.
     * @param fDuration The ticks the move takes.
     * @ghidraAddress NTSC-U/C: 0x001bbe50
     * @ghidraAddress PAL: 0x001c4bf0
     */
    void SetArrowTarget(float fX, float fY, float fAngle, float fDuration);

    /**
     * Fade the arrow in or out over 240 ticks.
     *
     * @param bShow Whether to show the arrow.
     * @ghidraAddress NTSC-U/C: 0x001bbfb8
     * @ghidraAddress PAL: 0x001c4d58
     */
    void ShowArrow(bool bShow);

    /**
     * Advance every change that runs.
     *
     * @param fTick The song tick.
     * @ghidraAddress NTSC-U/C: 0x001bc058
     * @ghidraAddress PAL: 0x001c4df8
     */
    void Update(float fTick);

    Rnd::View *mArrow;                     /*!< `HUD hilite arrow.view`. */
    Rnd::Mat *mArrowMat;                   /*!< `HUD hilite arrow.mat`. */
    Rnd::Mesh *mBox;                       /*!< `HUD hilite_box.mesh`. */
    Rnd::Mat *mBoxMat;                     /*!< `HUD hilite_box.mat`. */
    int mReserved10[5];                    // +0x10, not written by any routine recovered so far.
    unsigned char mBoxVerts[kNumBoxVerts]; /*!< The vertex of the box at each place, row by row. */
    Vector2 mCornerSize;                   /*!< The width and depth of a corner of the box. */
    Vector2 mFromSize;                     /*!< The size of the box when the move started. */
    Vector2 mToSize;                       /*!< The size the box moves to. */
    Vector3 mFromPos;                      /*!< The place of the box when the move started. */
    Vector3 mToPos;                        /*!< The place the box moves to. */
    LinearInterpolator mBoxMove;           /*!< The progress of the box's move against the tick. */
    LinearInterpolator mBoxFade;           /*!< The alpha of the box against the tick. */
    LinearInterpolator mArrowFade;         /*!< The alpha of the arrow against the tick. */
    ATanInterpolator mArrowMove; /*!< The progress of the arrow's move against the tick. */
    Vector3 mFromArrowPos;       /*!< The place of the arrow when the move started. */
    float mFromArrowAngle;   /*!< The direction of the arrow when the move started, in radians. */
    Vector3 mToArrowPos;     /*!< The place the arrow moves to. */
    float mToArrowAngle;     /*!< The direction the arrow turns to, in radians. */
    unsigned char mChanging; /*!< The changes Update() still runs, a set of Changing bits. */
};
