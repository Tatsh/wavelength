#pragma once

#include "gfx/freestylus.h"
#include "gfx/gfxtunnel.h"
#include "gfx/quadraticapproach.h"
#include "gfx/ship.h"
#include "gfx/tnlactivator.h"
#include "gfx/tnlsabretrail.h"
#include "math/interpolator.h"
#include "math/transform.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "rnd/rndtransanim.h"
#include "rnd/rndtransformable.h"
#include "script/dataarray.h"

/**
 * The freestyle of a player, in which the ship turns into the freestylus and the stick moves it
 * about the track.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0xfc bytes. The ship
 * assembles into the freestylus over the enter delay, and the camera of the player slides to
 * follow it. The position, the tumble, and the camera follow the stick through quadratic
 * approaches whose curves come from the `freestyle` configuration.
 */
class TnlFreestyle {
public:
    /**
     * Create the trail, the curves, and the shared freestylus of the first freestyle.
     *
     * @param nPlayer The player.
     * @param nType The kind of freestyle, one of Freestylus::Type.
     * @param pShip The ship of the player.
     * @param pActivator The activator of the player.
     * @param pTunnel The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001e8928
     * @ghidraAddress PAL: 0x001f16c8
     */
    TnlFreestyle(int nPlayer, int nType, Ship *pShip, TnlActivator *pActivator, GfxTunnel *pTunnel);

    /**
     * Delete the shared freestylus, the trail, and the curves.
     *
     * @ghidraAddress NTSC-U/C: 0x001e8b38
     * @ghidraAddress PAL: 0x001f18d8
     */
    ~TnlFreestyle();

    /**
     * Load the settings of the trail, of the freestyle of the kind, and of the lanes.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel, which the freestyle does not read.
     * @param nOption The fourth argument every loader of a player's tunnel graphics receives. A
     *                non-zero value leaves the scale of the freestylus.
     * @param bFreestyle Whether the freestyle settings load.
     * @ghidraAddress NTSC-U/C: 0x001e83c8
     * @ghidraAddress PAL: 0x001f1168
     */
    void LoadConfig(
        DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption, int bFreestyle);

    /**
     * End the freestyle at once, keeping an assembled freestylus.
     *
     * @ghidraAddress NTSC-U/C: 0x001e8c40
     * @ghidraAddress PAL: 0x001f19e0
     */
    void Reset();

    /**
     * Assemble the freestylus, or take it apart.
     *
     * @param bActive Whether the freestyle starts.
     * @param bVictory Whether the freestyle rewards a won song.
     * @param bQuick Whether the change takes 4 ticks instead of the configured delay.
     * @ghidraAddress NTSC-U/C: 0x001e8cd0
     * @ghidraAddress PAL: 0x001f1a70
     */
    void SetActive(bool bActive, int bVictory, bool bQuick);

    /**
     * Record whether the player plays, which grows the beams and draws the trail.
     *
     * @param bPlaying Whether the player plays.
     * @param nColumn The column of the freestyle. The routine does not read it.
     * @ghidraAddress NTSC-U/C: 0x001e8f98
     * @ghidraAddress PAL: 0x001f1d38
     */
    void SetPlaying(bool bPlaying, int nColumn);

    /**
     * Start a sample on the freestylus and on the trail.
     *
     * @ghidraAddress NTSC-U/C: 0x001e9038
     * @ghidraAddress PAL: 0x001f1dd8
     */
    void Sample();

    /**
     * Set the position of the stick.
     *
     * @param stick The position.
     * @ghidraAddress NTSC-U/C: 0x001e9098
     * @ghidraAddress PAL: 0x001f1e38
     */
    void SetStick(const Vector3 &stick);

    /**
     * Sweep the needle of a scratch freestyle for a note.
     *
     * @param flTick The tick of the note.
     * @param flDuration The length of the note in ticks.
     * @ghidraAddress NTSC-U/C: 0x001e90a8
     * @ghidraAddress PAL: 0x001f1e48
     */
    void Scratch(float flTick, float flDuration);

    /**
     * Advance the assembly, the curves, and the freestylus, and place the ship and the camera.
     *
     * @param flTicks The ticks since the last poll.
     * @param flTime The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001e90d8
     * @ghidraAddress PAL: 0x001f1e78
     */
    void Poll(float flTicks, float flTime);

    /**
     * Draw the needle of a scratch freestyle at the ship.
     *
     * @ghidraAddress NTSC-U/C: 0x001e9450
     * @ghidraAddress PAL: 0x001f21f0
     */
    void UpdateTrail();

    /**
     * Draw the freestylus, and the trail of a scratch freestyle.
     *
     * @ghidraAddress NTSC-U/C: 0x001e94f8
     * @ghidraAddress PAL: 0x001f2298
     */
    void Draw();

    /**
     * Work out the transforms of the ship and of the camera from the stick.
     *
     * @param bCam Whether the camera follows the freestyle.
     * @param pAnim The animation the ship plays, or null.
     * @param pStick The position of the stick.
     * @param pShipXfm Receives the rows of the transform of the ship.
     * @param pCamXfm Receives the transform of the camera.
     * @param flTime The time since the last poll.
     * @param flTicks The ticks since the last poll, which the routine does not read.
     * @param flElapsed The ticks since the freestyle started.
     * @ghidraAddress NTSC-U/C: 0x001e9570
     * @ghidraAddress PAL: 0x001f2310
     */
    void ComputeXfms(bool bCam,
                     RndTransAnim *pAnim,
                     const Vector3 *pStick,
                     float (*pShipXfm)[kTransformRowFloatCount],
                     Transform *pCamXfm,
                     float flTime,
                     float flTicks,
                     float flElapsed);

    /**
     * Hand the ship and the camera to the freestyle.
     *
     * @ghidraAddress NTSC-U/C: 0x001e9b20
     * @ghidraAddress PAL: 0x001f28c0
     */
    void BeginPass();

    /**
     * Give the ship and the camera back.
     *
     * @ghidraAddress NTSC-U/C: 0x001e9bb0
     * @ghidraAddress PAL: 0x001f2950
     */
    void EndPass();

    /**
     * Handle the end of the assembly. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x001e9c48
     * @ghidraAddress PAL: 0x001f29e8
     */
    void OnAssembled();

    /**
     * Handle the start of the disassembly. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x001e9c50
     * @ghidraAddress PAL: 0x001f29f0
     */
    void OnDisassemble();

    /**
     * Report the track the ship of the freestyle is over.
     *
     * @return The last track whose left edge is at or left of the ship, or 0.
     * @ghidraAddress NTSC-U/C: 0x001e9c58
     * @ghidraAddress PAL: 0x001f29f8
     */
    int GetTrack();

    /**
     * Report what the freestyle takes over.
     *
     * @param pbCam Receives whether the camera follows the freestyle.
     * @param pbShip Receives whether the freestyle places the ship.
     * @param pbShowShip Receives whether the ship stays shown.
     * @ghidraAddress NTSC-U/C: 0x001e9cb0
     * @ghidraAddress PAL: 0x001f2a50
     */
    void GetPassFlags(int *pbCam, int *pbShip, int *pbShowShip);

    /**
     * The delay of the assembly in ticks, `enter_delay`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afaa0
     */
    static float sEnterDelay;

    /**
     * The delay of the disassembly in ticks, `exit_delay`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afaa4
     */
    static float sExitDelay;

    /**
     * The greatest speed of the position, `max_change_per_ms`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afaa8
     */
    static float sMaxChangePerMs;

    /**
     * The greatest speed of the camera, `cam_max_change_per_ms`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afaac
     */
    static float sCamMaxChangePerMs;

    /**
     * The scale of a stick towards the lanes ahead, 1 for several players on this console and 2
     * otherwise.
     *
     * @ghidraAddress NTSC-U/C: 0x003afab4
     */
    static float sLaneScale;

    /**
     * The reciprocal of sLaneScale.
     *
     * @ghidraAddress NTSC-U/C: 0x003afab8
     */
    static float sInvLaneScale;

    /**
     * The freestylus the freestyles share, or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003afac4
     */
    static Freestylus *sFreestylus;

    /**
     * The reach of the stick across and along the track, `range`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b4b0
     */
    static Vector2 sRange;

    /**
     * The reciprocals of sRange.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b4b8
     */
    static Vector2 sInvRange;

    /**
     * The place of the freestyle, `center`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b4c0
     */
    static Vector3 sCenter;

    /**
     * The place of a constrained freestyle, `constrained_center`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b4d0
     */
    static Vector3 sConstrainedCenter;

    /**
     * The offset of the camera, `cam_offset`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b4e0
     */
    static Vector3 sCamOffset;

    /**
     * The offset the camera looks at, `cam_look_offset`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b4f0
     */
    static Vector3 sCamLookOffset;

    /**
     * The offset of the camera of a victory, `cam_win_offset`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b500
     */
    static Vector3 sCamWinOffset;

    /**
     * The offset the camera of a victory looks at, `cam_win_look_offset`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b510
     */
    static Vector3 sCamWinLookOffset;

    /**
     * The reach of the camera, `cam_range`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b520
     */
    static Vector2 sCamRange;

    /**
     * The reach of the camera of a constrained freestyle, `constrained_cam_range`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b528
     */
    static Vector2 sConstrainedCamRange;

    /**
     * The curve of the position, `change_func`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b530
     */
    static Vector3 sChangeFunc;

    /**
     * The curve of the tumble, `tumble_change_func`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b540
     */
    static Vector3 sTumbleChangeFunc;

    /**
     * The curve of the camera, `cam_change_func`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b550
     */
    static Vector3 sCamChangeFunc;

    /**
     * The curve of the constraint, `constraint_change_func`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b560
     */
    static Vector3 sConstraintChangeFunc;

    TnlSabreTrail *mTrail;          /*!< The trail of a scratch freestyle. */
    int mPlayer;                    /*!< The player. */
    int mActive;                    /*!< Whether the freestyle runs. */
    int mAssembled;                 /*!< Whether the freestylus has assembled. */
    int mVictory;                   /*!< Whether the freestyle rewards a won song. */
    int mPlaying;                   /*!< Whether the player plays. */
    int mType;                      /*!< The kind of freestyle, one of Freestylus::Type. */
    GfxTunnel *mTunnel;             /*!< The tunnel. */
    Ship *mShip;                    /*!< The ship of the player. */
    TnlActivator *mActivator;       /*!< The activator of the player. */
    float mStartTick;               /*!< The tick the freestyle started at. */
    LinearInterpolator *mAssembly;  /*!< The assembly over the ticks of a change. */
    LinearInterpolator *mCamBlend;  /*!< The blend of the camera over the ticks of a change. */
    unsigned char mReserved34[0xc]; // +0x34 to +0x3f are not yet identified.
    Vector3 mStick;                 /*!< The position of the stick. */
    Vector3 mShipPos;               /*!< The place of the ship before the path. */
    Vector3 mPathAxis;              /*!< The first axis of the path at the ship. */
    QuadraticApproach mX;           /*!< The position across the track. */
    QuadraticApproach mY;           /*!< The position along the track. */
    QuadraticApproach mTumbleX;     /*!< The tumble from the speed across the track. */
    QuadraticApproach mTumbleY;     /*!< The tumble from the speed along the track. */
    QuadraticApproach mCamX;        /*!< The camera across the track. */
    QuadraticApproach mCamY;        /*!< The camera along the track. */
    QuadraticApproach mConstraint;  /*!< The constraint of the arena. */
};
