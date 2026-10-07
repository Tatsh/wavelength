#pragma once

#include "math/interpolator.h"
#include "math/quaternion.h"
#include "math/transform.h"
#include "math/vector3.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/movie.h"
#include "rnd/view.h"
#include "script/dataarray.h"

/**
 * The projector the front end moves between the menu screens.
 *
 * The RTTI includes the class name and records no base. The object is 0xa0 bytes, allocated with
 * the global `operator new`. The vptr follows the members at `+0x94`, and the vtable at
 * `0x003d00d0` runs the type function, the destructor, Draw(), and Poll().
 *
 * The metagame creates the one instance and stores it in Metagame::mGizmo. Each FreqScreen records
 * where the gizmo stands while the screen shows, and MoveTo() eases the gizmo there along an
 * arctangent over the frames the `freq_screen` section of the metagame configuration gives. The
 * gizmo's screen shows either its movie or, while it shows the avatar, the avatar texture.
 */
class Gizmo {
public:
    /**
     * Construct the gizmo from the loaded `gizmo.view`, `gizmo.mesh`, `gizmo movie.mov`, and
     * `freqscreen.mat`, showing the movie.
     *
     * @param nUnused Stored at `+0x40`. No routine reads it, and the metagame passes 0.
     * @ghidraAddress NTSC-U/C: 0x001a4f20
     * @ghidraAddress PAL: 0x001acc10
     */
    explicit Gizmo(int nUnused);

    /**
     * Destroy the gizmo.
     *
     * @ghidraAddress NTSC-U/C: 0x001a5140
     * @ghidraAddress PAL: 0x001ace30
     */
    virtual ~Gizmo();

    /**
     * Draw the gizmo, rendering the avatar first while the gizmo shows it.
     *
     * @ghidraAddress NTSC-U/C: 0x001a5240
     * @ghidraAddress PAL: 0x001acf30
     */
    virtual void Draw();

    /**
     * Advance the gizmo's move and its animations to a time, and place its mesh.
     *
     * Nothing happens while the gizmo is hidden.
     *
     * @param fTime The front-end time.
     * @ghidraAddress NTSC-U/C: 0x001a5290
     * @ghidraAddress PAL: 0x001acf80
     */
    virtual void Poll(float fTime);

    /**
     * Read the frames of the move from the `freq_screen` section.
     *
     * @param pConfig The metagame section of the configuration, which must have `freq_screen`.
     * @ghidraAddress NTSC-U/C: 0x001a4e80
     * @ghidraAddress PAL: 0x001acb70
     */
    static void Init(DataArray *pConfig);

    /**
     * Show or hide the gizmo's view.
     *
     * @param bShowing Whether the view shows.
     * @ghidraAddress NTSC-U/C: 0x001a5180
     * @ghidraAddress PAL: 0x001ace70
     */
    void SetShowing(bool bShowing);

    /**
     * Report whether the gizmo's view shows.
     *
     * @return Whether the view shows.
     * @ghidraAddress NTSC-U/C: 0x001a51b0
     * @ghidraAddress PAL: 0x001acea0
     */
    bool IsShowing() const;

    /**
     * Show the avatar texture or the movie on the gizmo's screen.
     *
     * The avatar texture draws with source alpha blending and the movie additively.
     *
     * @param bShowAvatar Whether the screen shows the avatar texture.
     * @ghidraAddress NTSC-U/C: 0x001a51c0
     * @ghidraAddress PAL: 0x001aceb0
     */
    void SetShowAvatar(bool bShowAvatar);

    /**
     * Report whether the gizmo's screen shows the avatar texture.
     *
     * @return Whether the screen's material blends with source alpha.
     * @ghidraAddress NTSC-U/C: 0x001a5228
     * @ghidraAddress PAL: 0x001acf18
     */
    bool IsShowingAvatar() const;

    /**
     * Report the world transform of the gizmo's mesh.
     *
     * @return The transform.
     * @ghidraAddress NTSC-U/C: 0x001a54a8
     * @ghidraAddress PAL: 0x001ad198
     */
    Transform WorldXfm() const;

    /**
     * Show the gizmo and start moving it from where it stands to a position and an orientation.
     *
     * @param position The position.
     * @param orientation The orientation.
     * @param fTime The front-end time the move starts at.
     * @ghidraAddress NTSC-U/C: 0x001a54d8
     * @ghidraAddress PAL: 0x001ad1c8
     */
    void MoveTo(const Vector3 &position, const Quat &orientation, float fTime);

    /**
     * Place the gizmo at a position and an orientation.
     *
     * A move under way continues towards the new place.
     *
     * @param position The position.
     * @param orientation The orientation.
     * @ghidraAddress NTSC-U/C: 0x001a5540
     * @ghidraAddress PAL: 0x001ad230
     */
    void SetTo(const Vector3 &position, const Quat &orientation);

    alignas(16) Vector3 mPos;     /*!< The position the gizmo stands at or moves to. */
    alignas(16) Quat mRot;        /*!< The orientation the gizmo has or turns to. */
    alignas(16) Vector3 mFromPos; /*!< The position the move started from. */
    alignas(16) Quat mFromRot;    /*!< The orientation the move started from. */
    int mUnused;                  // +0x40, the constructor's argument, which no routine reads.
    float mMoveStart;             /*!< The time the move started, or 0 when no move is under way. */
    Rnd::View *mView;             /*!< The view `gizmo.view`. */
    Rnd::Mat *mMat;               /*!< The material `freqscreen.mat` of the gizmo's screen. */
    Rnd::Mesh *mMesh;             /*!< The mesh `gizmo.mesh`, which Poll() places. */
    Rnd::Movie *mMovie;           /*!< The movie `gizmo movie.mov`. */
    ATanInterpolator mInterp;     /*!< Maps a frame of the move to the fraction moved. */

private:
    /**
     * The elapsed time at which a move starts to show, `panel_gizmo_begin_frame`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8cc
     */
    static float sBeginFrame;

    /**
     * The first frame of the move's curve, `panel_gizmo_anim_start`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8d0
     */
    static float sAnimStart;

    /**
     * The last frame of the move's curve, `panel_gizmo_anim_end`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8d4
     */
    static float sAnimEnd;

    /**
     * How sharply the move eases, `panel_gizmo_interp_severity`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8d8
     */
    static float sSeverity;
};
