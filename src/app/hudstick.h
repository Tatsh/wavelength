#pragma once

#include "math/vector2.h"
#include "rnd/animatable.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/transformable.h"

/**
 * Marker on the controller stick diagram of the head-up display, which the tutorial moves.
 *
 * The class is not polymorphic and emits no RTTI. The object is 0xc bytes. The title is inferred
 * from the `gfx_stick_diagram` script command that drives it.
 */
class HudStick {
public:
    /** The materials of the marker, the indices of mMats. */
    enum Direction {
        kDirectionInOut = 0,  /*!< `HUD in_out.mat`, for a push in or out. */
        kDirectionUpDown = 1, /*!< `HUD up_dn.mat`, for a push up or down. */
        kNumDirections = 2,   /*!< The number of materials. */
    };

    /**
     * Construct a hidden marker.
     *
     * The marker is `HUD<hud>r_stick.mesh`. The material animations `HUD in_out.mnm` and
     * `HUD up_dn.mnm` join pAnims.
     *
     * @param chHud The letter of the head-up display layout.
     * @param pAnims The animation the material animations join.
     * @param pParent The transform the marker moves with.
     * @ghidraAddress NTSC-U/C: 0x001b8f20
     * @ghidraAddress PAL: 0x001c1cc0
     */
    HudStick(char chHud, Rnd::Animatable *pAnims, Rnd::Transformable *pParent);

    /**
     * Hide the marker.
     *
     * @ghidraAddress NTSC-U/C: 0x001b9158
     * @ghidraAddress PAL: 0x001c1ef8
     */
    void Reset();

    /**
     * Hide the marker.
     *
     * @ghidraAddress NTSC-U/C: 0x001b9178
     * @ghidraAddress PAL: 0x001c1f18
     */
    void Hide();

    /**
     * Show the marker at a place on the diagram with one of its materials.
     *
     * @param nDirection The material, one of Direction.
     * @param pPosition The place, as x and z on the diagram.
     * @ghidraAddress NTSC-U/C: 0x001b91a8
     * @ghidraAddress PAL: 0x001c1f48
     */
    void Show(int nDirection, const Vector2 *pPosition);

    Rnd::Mesh *mMesh;                /*!< The marker. */
    Rnd::Mat *mMats[kNumDirections]; /*!< The materials, indexed by Direction. */
};
