#pragma once

#include <list>
#include <vector>

#include "gfx/gfxutil.h"
#include "gfx/particlearm.h"
#include "gfx/playercamfx.h"
#include "math/color.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "rnd/animatable.h"
#include "rnd/blur.h"
#include "rnd/environ.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/particlesys.h"
#include "rnd/string.h"
#include "rnd/text.h"
#include "rnd/transanim.h"
#include "rnd/transformable.h"
#include "rnd/view.h"
#include "script/dataarray.h"

// The beams and the flight point back at the ship that owns them.
class Beam;
class ShipFlyIn;
template <bool bArg>
class BeamPool;

/**
 * Ship of one player, which rides the tracks of the tunnel and fires beams at the gems.
 *
 * The RTTI of its nested BeamData type names the class. The class is not polymorphic, and the
 * object is 0x260 bytes. The three arms of the ship hold the trodes that fire at the three gem
 * buttons, and each arm's particle system also records the colours of one zone of the energy
 * meter.
 */
class Ship {
public:
    /** The number of arms, one for each gem button. */
    static constexpr int kNumArms = 3;

    /** The two configured offsets of a ship of a game of several players. */
    static constexpr int kNumMultiOffsets = 2;

    /** The two ends of a beam, 0 at the ship and 1 at the gem. */
    static constexpr int kNumBeamEnds = 2;

    /** The colours of the particles of an arm, indexing mArmColors. */
    enum ArmColor {
        kArmColorStartLow = 0,  /*!< The low end of the start colour. */
        kArmColorStartHigh = 1, /*!< The high end of the start colour. */
        kArmColorEndLow = 2,    /*!< The low end of the end colour. */
        kArmColorEndHigh = 3,   /*!< The high end of the end colour. */
        kNumArmColors = 4,      /*!< The number of colours. */
    };

    /** The bits of mFlags. */
    enum Flag {
        kFlagAttacking = 1,   /*!< The ship plays the attack of a bumper. */
        kFlagBumped = 2,      /*!< The ship is knocked across the tracks by a bumper. */
        kFlagBumpReverse = 4, /*!< The knock moves towards a lower track. */
    };

    /**
     * One beam a ship fired.
     *
     * The RTTI names the type. The record is 8 bytes.
     */
    struct BeamData {
        Beam *mBeam;        /*!< The beam. */
        signed char mSlot;  /*!< The gem button the beam was fired from. */
        signed char mTrack; /*!< The track the beam was fired at. */
    };

    /**
     * Build the ship of a player from the scene objects of its colour.
     *
     * The first ship also finds the scene objects every ship shares.
     *
     * @param pszColor The colour of the player.
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001c6b70
     * @ghidraAddress PAL: 0x001cf910
     */
    Ship(const char *pszColor, int nPlayer);

    /**
     * Destroy the ship, the views it created, and every object the ships share.
     *
     * @ghidraAddress NTSC-U/C: 0x001c8090
     * @ghidraAddress PAL: 0x001d0e30
     */
    ~Ship();

    /**
     * Read the configuration every ship shares, and pass it on to the beam pools that exist.
     *
     * @param pConfig The section of the kind of game.
     * @param pDefaults The `gfx` section.
     * @param bReload Whether the configuration was read before.
     * @ghidraAddress NTSC-U/C: 0x001c6470
     * @ghidraAddress PAL: 0x001cf210
     */
    static void LoadConfig(DataArray *pConfig, DataArray *pDefaults, bool bReload);

    /**
     * Give the ship its leader material while the streak multiplier is above 1.
     *
     * @param nMultiplier The multiplier.
     * @ghidraAddress NTSC-U/C: 0x001c82b0
     * @ghidraAddress PAL: 0x001d1050
     */
    void SetMultiplier(int nMultiplier);

    /**
     * Return the ship to its state at the start of a song.
     *
     * @ghidraAddress NTSC-U/C: 0x001c82e8
     * @ghidraAddress PAL: 0x001d1088
     */
    void Reset();

    /**
     * Colour the arms for the zone of the energy, or make them sputter when it runs out.
     *
     * @param fEnergy The energy, from 0 to 1.
     * @param nZone The zone of the energy.
     * @param fZoneFraction The position of the energy within its zone. The ship does not read it.
     * @ghidraAddress NTSC-U/C: 0x001c8488
     * @ghidraAddress PAL: 0x001d1228
     */
    void SetEnergy(float fEnergy, int nZone, float fZoneFraction);

    /**
     * Fire the ship at a gem.
     *
     * @param nSlot The gem button.
     * @param nTrack The track.
     * @param bHit Whether the gem was hit.
     * @ghidraAddress NTSC-U/C: 0x001c8d48
     * @ghidraAddress PAL: 0x001d1ae8
     */
    void Fire(int nSlot, signed char nTrack, bool bHit);

    /**
     * Move the ship to a track, which ends its beams.
     *
     * @param nTrack The track. The ship does not read it.
     * @param nTrackType The type of the track. The ship does not read it.
     * @param nInstrument The instrument of the track. The ship does not read it.
     * @ghidraAddress NTSC-U/C: 0x001c8e58
     * @ghidraAddress PAL: 0x001d1bf8
     */
    void SetTrack(int nTrack, int nTrackType, int nInstrument);

    /**
     * Show or hide the ship.
     *
     * While the tutorial flight exists, the flight decides whether the ship shows.
     *
     * @param bShown Whether the ship is shown.
     * @ghidraAddress NTSC-U/C: 0x001c8e78
     * @ghidraAddress PAL: 0x001d1c18
     */
    void SetShown(bool bShown);

    /**
     * Hide the ship regardless of SetShown(), or stop hiding it, and end its beams.
     *
     * @param bHidden Whether the ship is hidden.
     * @ghidraAddress NTSC-U/C: 0x001c8ee8
     * @ghidraAddress PAL: 0x001d1c88
     */
    void SetHidden(bool bHidden);

    /**
     * Blend the ship towards a fixed transform, or stop blending.
     *
     * @param pXfm The transform, or null to stop.
     * @param fBlend The blend, from 0 for the ship's own transform to 1 for pXfm.
     * @ghidraAddress NTSC-U/C: 0x001c8f50
     * @ghidraAddress PAL: 0x001d1cf0
     */
    void SetOverride(const float (*pXfm)[Rnd::kXfmRowFloatCount], float fBlend);

    /**
     * Start the move of the ship of a bump.
     *
     * @param bVictim Whether the ship is the bumped one.
     * @param nTrack The track the bumped ship moves to.
     * @ghidraAddress NTSC-U/C: 0x001c8f98
     * @ghidraAddress PAL: 0x001d1d38
     */
    void StartBump(bool bVictim, int nTrack);

    /**
     * Place the ship on the tracks and advance its effects.
     *
     * @param fTicks The ticks since the last poll. The ship does not read them.
     * @param fTimeDelta The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001c9100
     * @ghidraAddress PAL: 0x001d1ea0
     */
    void Poll(float fTicks, float fTimeDelta);

    /**
     * Place the ship for the intro and the outro of the play field.
     *
     * @ghidraAddress NTSC-U/C: 0x001ca2f0
     * @ghidraAddress PAL: 0x001d3090
     */
    void UpdateTransform();

    /**
     * Draw the environment every ship shares.
     *
     * @ghidraAddress NTSC-U/C: 0x001cab50
     * @ghidraAddress PAL: 0x001d38f0
     */
    static void DrawShared();

    /**
     * Draw the ship on its track and record where it lies on the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x001cab78
     * @ghidraAddress PAL: 0x001d3918
     */
    void DrawTrack();

    /**
     * Draw the trodes, the arms, and the attack of a bumper.
     *
     * @ghidraAddress NTSC-U/C: 0x001cabf0
     * @ghidraAddress PAL: 0x001d3990
     */
    void DrawEffects();

    /**
     * Draw the arrow at the edge of the screen that points at a ship off the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x001cad60
     * @ghidraAddress PAL: 0x001d3b00
     */
    void DrawArrow();

    /**
     * Scale the arms and the trodes.
     *
     * @param fScale The scale, 1 for the configured size.
     * @ghidraAddress NTSC-U/C: 0x001c8750
     * @ghidraAddress PAL: 0x001d14f0
     */
    void ScaleArms(float fScale);

    /**
     * Scale one arm and its trode.
     *
     * @param nArm The arm.
     * @param fScale The scale, 1 for the configured size.
     * @ghidraAddress NTSC-U/C: 0x001c8840
     * @ghidraAddress PAL: 0x001d15e0
     */
    void ScaleArm(int nArm, float fScale);

    /** `ship_offset`, scaled by the tunnel. @ghidraAddress NTSC-U/C: 0x0043b210 */
    static Vector3 sOffset;
    /** `ship_multi_offset_start` and `_end`, scaled. @ghidraAddress NTSC-U/C: 0x0043b220 */
    static Vector3 sMultiOffsets[kNumMultiOffsets];
    /** `ship_pivot`. @ghidraAddress NTSC-U/C: 0x0043b240 */
    static Vector3 sPivot;
    /** sOffset plus sPivot, scaled. @ghidraAddress NTSC-U/C: 0x0043b250 */
    static Vector3 sOffsetPivot;
    /** sMultiOffsets plus sPivot, scaled. @ghidraAddress NTSC-U/C: 0x0043b260 */
    static Vector3 sMultiOffsetPivots[kNumMultiOffsets];
    /** `ship_track_label_pitch_color`. @ghidraAddress NTSC-U/C: 0x0043b280 */
    static Color sTrackLabelPitchColor;
    /** `ship_intro_pos`, scaled. @ghidraAddress NTSC-U/C: 0x0043b290 */
    static Vector3 sIntroPos;
    /** `ship_intro_trode_size_mult`. @ghidraAddress NTSC-U/C: 0x0043b2a0 */
    static std::vector<FloatKey> sIntroTrodeSizeMult;
    /** `ship_fire_trode_size_mult`. @ghidraAddress NTSC-U/C: 0x0043b200 */
    static std::vector<FloatKey> sFireTrodeSizeMult;
    /** `view_distance`, scaled. @ghidraAddress NTSC-U/C: 0x0043b2b0 */
    static float sViewDistance;
    /** `ship_fire_offset`. @ghidraAddress NTSC-U/C: 0x003af960 */
    static float sFireOffset;
    /** `ship_fire_scale`. @ghidraAddress NTSC-U/C: 0x003af964 */
    static float sFireScale;
    /** The frame of the last key of sFireTrodeSizeMult. @ghidraAddress NTSC-U/C: 0x003af968 */
    static float sFireTrodeEnd;
    /** `ship_tilt`. @ghidraAddress NTSC-U/C: 0x003af96c */
    static float sTilt;
    /** `ship_scale`, scaled by the tunnel. @ghidraAddress NTSC-U/C: 0x003af970 */
    static float sScale;
    /** `ship_bank_spring`. @ghidraAddress NTSC-U/C: 0x003af974 */
    static float sBankSpring;
    /** `ship_bank_damper`. @ghidraAddress NTSC-U/C: 0x003af978 */
    static float sBankDamper;
    /** `ship_bank_magnitude`. @ghidraAddress NTSC-U/C: 0x003af97c */
    static float sBankMagnitude;
    /** `ship_bank_force`, scaled. @ghidraAddress NTSC-U/C: 0x003af980 */
    static float sBankForce;
    /** `ship_move_miss_beams`, for each end. @ghidraAddress NTSC-U/C: 0x003af988 */
    static int sMoveMissBeams[kNumBeamEnds];
    /** `ship_move_hit_beams`, for each end. @ghidraAddress NTSC-U/C: 0x003af990 */
    static int sMoveHitBeams[kNumBeamEnds];
    /** `ship_move_hit2_beams`, for each end. @ghidraAddress NTSC-U/C: 0x003af998 */
    static int sMoveHit2Beams[kNumBeamEnds];
    /** `ship_sputter_force`, scaled. @ghidraAddress NTSC-U/C: 0x003af9a0 */
    static float sSputterForce;
    /** `ship_bump_height_accel`. @ghidraAddress NTSC-U/C: 0x003af9a4 */
    static float sBumpHeightAccel;
    /** `ship_bump_attack_time`. @ghidraAddress NTSC-U/C: 0x003af9a8 */
    static float sBumpAttackTime;
    /** `ship_bump_ship_move_slowdown`. @ghidraAddress NTSC-U/C: 0x003af9ac */
    static float sBumpShipMoveSlowdown;
    /** `ship_crippled_cam_jiggle_probability`. @ghidraAddress NTSC-U/C: 0x003af9b0 */
    static float sCrippledCamJiggleProbability;
    /** `ship_crippled_cam_jiggle`. @ghidraAddress NTSC-U/C: 0x003af9b4 */
    static float sCrippledCamJiggle;
    /** `ship_intro_end_tick`. @ghidraAddress NTSC-U/C: 0x003af9b8 */
    static float sIntroEndTick;
    /** `ship_intro_blend_end_tick`. @ghidraAddress NTSC-U/C: 0x003af9bc */
    static float sIntroBlendEndTick;
    /** The frame of the last key of sIntroTrodeSizeMult. @ghidraAddress NTSC-U/C: 0x003af9c0 */
    static float sIntroTrodeEnd;
    /** The beams of the missed gems. @ghidraAddress NTSC-U/C: 0x003af9c4 */
    static BeamPool<false> *sMissBeams;
    /** The beams of the hit gems. @ghidraAddress NTSC-U/C: 0x003af9c8 */
    static BeamPool<false> *sHitBeams;
    /** The second beams of the hit gems. @ghidraAddress NTSC-U/C: 0x003af9cc */
    static BeamPool<false> *sHit2Beams;
    /** `ship beam draw.view`, which the beams draw under. @ghidraAddress NTSC-U/C: 0x003af9d0 */
    static Rnd::View *sBeamView;
    /** `ship.env`. @ghidraAddress NTSC-U/C: 0x003af9d8 */
    static Rnd::Environ *sEnviron;
    /** `ship loading.view`. @ghidraAddress NTSC-U/C: 0x003af9dc */
    static Rnd::View *sLoadingView;
    /** `ship win fly-off.tnm`. @ghidraAddress NTSC-U/C: 0x003af9e0 */
    static Rnd::TransAnim *sWinFlyOff;
    /** `ship all.anim`. @ghidraAddress NTSC-U/C: 0x003af9e4 */
    static Rnd::Animatable *sAllAnim;
    /** `ship intro.anim`. @ghidraAddress NTSC-U/C: 0x003af9e8 */
    static Rnd::Animatable *sIntroAnim;
    /** `ship arrow.mesh`, in a game of several players. @ghidraAddress NTSC-U/C: 0x003af9ec */
    static Rnd::Mesh *sArrowMesh;

private:
    /**
     * Set the particles of the crippler of a ship of another console or of a shared screen.
     *
     * @param pSys The particle system.
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001c62a8
     * @ghidraAddress PAL: 0x001cf048
     */
    static void ConfigureCrippler(Rnd::ParticleSys *pSys, int nPlayer);

    /**
     * Set the particles of an arm of a ship of another console or of a shared screen.
     *
     * @param pSys The particle system.
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001c6388
     * @ghidraAddress PAL: 0x001cf128
     */
    static void ConfigureArm(Rnd::ParticleSys *pSys, int nPlayer);

    /**
     * Fire a beam from a pool, stealing the oldest beam when the pool is empty.
     *
     * @param pList The list of the beams of this family.
     * @param pPool The pool.
     * @param nSlot The gem button.
     * @param nTrack The track.
     * @param fTick The tick of the gem.
     * @param fStart The time the beam starts.
     * @param fEnd The time the beam ends.
     * @ghidraAddress NTSC-U/C: 0x001c88e8
     * @ghidraAddress PAL: 0x001d1688
     */
    void AddBeam(std::list<BeamData> *pList,
                 BeamPool<false> *pPool,
                 signed char nSlot,
                 signed char nTrack,
                 float fTick,
                 float fStart,
                 float fEnd);

    /**
     * Return every beam of the ship to its pool.
     *
     * @ghidraAddress NTSC-U/C: 0x001c8bc8
     * @ghidraAddress PAL: 0x001d1968
     */
    void ClearBeams();

    /**
     * Advance the beams of one family, returning the ended ones to the pool.
     *
     * @param pList The list of the beams.
     * @param pPool The pool.
     * @param pXfm The transform of the ship.
     * @param pColor The colour of the ship.
     * @param bMoveStart Whether the end at the ship follows the ship.
     * @param bMoveEnd Whether the end at the gem follows the gem.
     * @ghidraAddress NTSC-U/C: 0x001ca5b0
     * @ghidraAddress PAL: 0x001d3350
     */
    void UpdateBeams(std::list<BeamData> *pList,
                     BeamPool<false> *pPool,
                     const float (*pXfm)[Rnd::kXfmRowFloatCount],
                     const Color *pColor,
                     bool bMoveStart,
                     bool bMoveEnd);

    /**
     * Advance the knock of a bumper.
     *
     * @param fTimeDelta The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001ca890
     * @ghidraAddress PAL: 0x001d3630
     */
    void PollBump(float fTimeDelta);

    /**
     * Advance the crippler, and shake the camera of a crippled player of this console.
     *
     * @param fTimeDelta The time since the last poll.
     * @param bCrippled Whether the ship is crippled.
     * @ghidraAddress NTSC-U/C: 0x001caa58
     * @ghidraAddress PAL: 0x001d37f8
     */
    void PollCrippler(float fTimeDelta, bool bCrippled);

    /**
     * Blend the ship of a solo game from the slide of the camera to its place on the tracks.
     *
     * Part of Poll().
     *
     * @param fTick The tick of the song.
     * @param pCamFX The camera.
     * @param pXfm The transform of the ship, which the blend updates.
     */
    inline void
    PollSoloIntro(float fTick, PlayerCamFX *pCamFX, float (*pXfm)[Rnd::kXfmRowFloatCount]);

    /**
     * Blend the ship of a game of several players from its intro to its place on the tracks.
     *
     * Part of Poll().
     *
     * @param fTick The tick of the song.
     * @param pCamFX The camera.
     * @param pXfm The transform of the ship, which the blend updates.
     */
    inline void PollIntro(float fTick, PlayerCamFX *pCamFX, float (*pXfm)[Rnd::kXfmRowFloatCount]);

public:
    Rnd::View *mView;                       /*!< `ship_<c>.view`. */
    Rnd::Mesh *mMesh;                       /*!< `ship_<c>.mesh`. */
    Rnd::View *mPsAnim;                     /*!< `ship ps anim <c>.view`, created. */
    Rnd::View *mBackPsAnim;                 /*!< `ship backps anim <c>.view`, created. */
    Rnd::View *mFxView;                     /*!< `ship fx_<c>.view`. */
    Rnd::Mat *mMat;                         /*!< `ship_<c>.mat`. */
    Rnd::Mat *mLeaderMat;                   /*!< `ship leader_<c>.mat`, or null. */
    Rnd::Text *mName;                       /*!< `ship name_<c>.txt`. */
    Rnd::String *mScratchRibbon;            /*!< `scratch ribbon_<c>.line`. */
    int mPlayer;                            /*!< The player. */
    Rnd::TransAnim *mIntroFlyIn;            /*!< `ship intro fly-in_<c>.tnm`. */
    Rnd::TransAnim *mIntro;                 /*!< `ship intro_<c>.tnm`. */
    float mStartMs;                         /*!< The system time UpdateTransform() started at. */
    float mFireTimes[kNumArms];             /*!< The time each arm last fired, or -1e9. */
    Rnd::Animatable *mArmAnims[kNumArms];   /*!< `ship arm <side>_<c>.tnm`. */
    Rnd::Transformable *mArmXfms[kNumArms]; /*!< `ship <side>_<c>`. */
    ParticleArm mArms[kNumArms];            /*!< `ship <side>_<c>.ps` on mArmXfms. */
    Rnd::Mesh *mTrodes[kNumArms];           /*!< `ship trode core <side>_<c>.mesh`. */
    ParticleArm mThruster;                  /*!< `ship ass_<c>.part` on mMesh. */
    Rnd::ParticleSys *mThruster2;           /*!< `ship ass_<c>2.part`. */
    Rnd::ParticleSys *mThruster3;           /*!< `ship ass_<c>3.part`. */
    // +0x9c is not yet identified.
    /*!< The start and end colours of each arm's particles, the colours of one energy zone. */
    Color mArmColors[kNumArms][kNumArmColors];
    Rnd::Mat *mTrodeMats[kNumArms]; /*!< The material of each trode. */
    float mTrodeSize;               /*!< The scale of the first trode. */
    float mArmSizeLow;              /*!< The low size of the particles of the first arm. */
    float mArmSizeHigh;             /*!< The high size of the particles of the first arm. */
    std::list<BeamData> mMissBeams; /*!< The beams of missed gems. */
    std::list<BeamData> mHitBeams;  /*!< The beams of hit gems. */
    std::list<BeamData> mHit2Beams; /*!< The second beams of hit gems. */
    Vector2 mScreenPos;             /*!< Where DrawTrack() last saw the ship on the screen. */
    Color mColor;                   /*!< The colour of the player. */
    float mSpread;                  /*!< How far apart the ships of a shared screen fly. */
    float mArrowAspect;             /*!< The slope of the edge arrow. Nothing writes it. +0x1b4 */
    int mHasOverride;               /*!< Whether SetOverride() gave a transform. */
    int mIntroActive;               /*!< Whether the intro still blends the ship in. */
    float mOverrideBlend;           /*!< The blend SetOverride() gave. */
    /*!< The transform of the intro, or the transform SetOverride() gave. +0x1d0 */
    float mXfm[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    int mShown;                          /*!< Whether SetShown() shows the ship. */
    int mHidden;                         /*!< Whether SetHidden() hides the ship. */
    int mMulti;                          /*!< Whether the game is not solo. */
    unsigned char mFlags;                /*!< A combination of Flag. */
    signed char mAttackTrack;            /*!< The track the attack of a bumper plays on. */
    float mAttackTime;                   /*!< The time the attack started. */
    Rnd::View *mBumperAttack;            /*!< `bumper attack.view`. */
    Rnd::Blur *mBumperBlur;              /*!< `ship bumper_<c>.blur`. */
    float mBumpStart;                    /*!< The time the knock started. */
    float mBumpEnd;                      /*!< The time the knock ends. */
    Rnd::TransAnim *mBumperFly;          /*!< `bumper fly.tnm`. */
    float mBumpHeight;                   /*!< The height of the knock. */
    float mBumpVel;                      /*!< The vertical speed of the knock. */
    Rnd::ParticleSys *mCripplerPs;       /*!< `ship crippler_<c>.part`. */
    float mCripplerTime;                 /*!< The time the crippler particles have run. */
    Rnd::ParticleSys *mBumperPs;         /*!< `ship bumper_<c>.ps`. */
    float mBumperTime;                   /*!< The time the bumper particles have run. */
    float mArrowStart;                   /*!< The time the ship left the screen, or -1e9. */
    Rnd::Animatable::SecondOrder *mBank; /*!< The spring of the bank of the ship. */
    ShipFlyIn *mFlyIn;                   /*!< The flight of the tutorial, or null. */
};
