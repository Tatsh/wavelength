#pragma once

#include <list>

#include "app/rampanimator.h"
#include "gfx/gfxtunnel.h"
#include "gfx/tnlgeom.h"
#include "math/interpolator.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "rnd/animatable.h"
#include "rnd/mesh.h"
#include "rnd/particlesys.h"
#include "rnd/text.h"
#include "rnd/transformable.h"
#include "rnd/view.h"

/**
 * The boundary across the tunnel at the start of each section of a song, with the name of the
 * section and a flash when the camera looks through it.
 *
 * The RTTI of the nested Section identifies the class, which is not polymorphic. The object is
 * 0x94 bytes.
 */
class TnlDivider {
public:
    /** One boundary, 0xc bytes. */
    struct Section {
        /**
         * Order sections by tick.
         *
         * @param other The section to compare against.
         * @return Whether this section comes first.
         */
        bool operator<(const Section &other) const {
            return mTick < other.mTick;
        }

        float mTick;        /*!< The tick of the boundary. */
        float mTextScale;   /*!< The scale of the text of the boundary. */
        const char *mLabel; /*!< The text of the boundary, or null for the default text. */
    };

    /**
     * Construct the boundary from its objects and fit them to the layout.
     *
     * @ghidraAddress NTSC-U/C: 0x001ef868
     * @ghidraAddress PAL: 0x001f8608
     */
    TnlDivider();

    /**
     * Scale the emission rates of the particles of the boundary.
     *
     * @param flScale The factor.
     * @ghidraAddress NTSC-U/C: 0x001f0008
     * @ghidraAddress PAL: 0x001f8da8
     */
    void ScaleParticles(float flScale);

    /**
     * Remove every boundary.
     *
     * @ghidraAddress NTSC-U/C: 0x001f0048
     * @ghidraAddress PAL: 0x001f8de8
     */
    void Reset();

    /**
     * Add a boundary, dropping those more than a bar behind the song.
     *
     * A boundary at the tick of one already added is ignored.
     *
     * @param flTick The tick of the boundary.
     * @param pszLabel The text of the boundary, or null for the default text.
     * @param flTextScale The scale of the text.
     * @ghidraAddress NTSC-U/C: 0x001f0068
     * @ghidraAddress PAL: 0x001f8e08
     */
    void AddSection(float flTick, const char *pszLabel, float flTextScale);

    /**
     * Report the ticks since the last boundary at or before a tick.
     *
     * @param flTick The tick.
     * @return The ticks, or 1e9 for a tick before every boundary.
     * @ghidraAddress NTSC-U/C: 0x001f0290
     * @ghidraAddress PAL: 0x001f9030
     */
    float TicksSinceSection(float flTick) const;

    /**
     * Move the boundary to the current section and animate it.
     *
     * @param flDelta The time since the last update.
     * @param pTunnel The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001f02f8
     * @ghidraAddress PAL: 0x001f9098
     */
    void Poll(float flDelta, GfxTunnel *pTunnel);

    /**
     * Draw the boundary, then the current environment.
     *
     * @ghidraAddress NTSC-U/C: 0x001f0770
     * @ghidraAddress PAL: 0x001f9510
     */
    void Draw();

    /**
     * Set the scale and the offset every boundary is built with.
     *
     * @param pScale The scale.
     * @param pOffset The offset.
     * @ghidraAddress NTSC-U/C: 0x001f0940
     * @ghidraAddress PAL: 0x001f96e0
     */
    static void SetLayout(const Vector3 *pScale, const Vector3 *pOffset);

    /**
     * The milliseconds of the flash.
     *
     * @ghidraAddress NTSC-U/C: 0x003afb24
     */
    static float sFlashTime;

    /**
     * The scale every boundary is built with.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b5f0
     */
    static Vector3 sScale;

    /**
     * The offset every boundary is built with.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b600
     */
    static Vector3 sOffset;

    Rnd::View *mView;                 /*!< `boundary.view`. */
    Rnd::View *mTransparentView;      /*!< `boundary transparent.view`, which draws the text. */
    Rnd::View *mTopView;              /*!< `boundary top.view`, placed on the path. */
    Rnd::View *mTextScaleView;        /*!< `boundary text scale.view`. */
    Rnd::View *mConditionView;        /*!< `boundary_condition.view`. */
    Rnd::Animatable *mMessageAnim;    /*!< `boundary msg.tnm`. */
    Rnd::ParticleSys *mParticles[2];  /*!< `boundary.part` and `boundary add.part`. */
    Vector2 mEmitRates[2];            /*!< The emission rates of mParticles as loaded. */
    Vector2 mSizes[2];                /*!< The sizes of mParticles as loaded. */
    Rnd::Text *mText;                 /*!< `boundary msg`. */
    Rnd::Mesh *mFlashMesh;            /*!< `boundary flash.mesh`. */
    RampAnimator mFlash;              /*!< The flash, animating `boundary flash fade.tnm`. */
    float mConditionFrame;            /*!< The frame of mConditionView. */
    LinearInterpolator mParticleFade; /*!< The emission over the ticks since the boundary. */
    int mFlashing;                    /*!< Whether the boundary of the finish flashes. */
    std::list<Section> mSections;     /*!< The boundaries, sorted by tick. */
    std::list<Section>::iterator mCurrent; /*!< The boundary shown. */
    int mStarted;                          /*!< Whether mCurrent is set. */
    int mEnabled;                          /*!< Whether the boundary shows. */

private:
    /**
     * Set the text of the boundary for a section and show its particles or its flash.
     *
     * @param section The section, or the end of mSections.
     * @ghidraAddress NTSC-U/C: 0x001f07b8
     * @ghidraAddress PAL: 0x001f9558
     */
    void UpdateText(std::list<Section>::iterator section);

    /**
     * Remove every boundary.
     *
     * @ghidraAddress NTSC-U/C: 0x001f0248
     * @ghidraAddress PAL: 0x001f8fe8
     */
    void ClearSections();

    /**
     * Place a transformable at the transform of the path at a tick.
     *
     * @param pTrans The transformable.
     * @param pGeom The geometry of the tracks.
     * @param flTick The tick.
     * @ghidraAddress NTSC-U/C: 0x001ee0e8
     * @ghidraAddress PAL: 0x001f6e88
     */
    static void PlaceOnPath(Rnd::Transformable *pTrans, TnlGeom *pGeom, float flTick);
};
