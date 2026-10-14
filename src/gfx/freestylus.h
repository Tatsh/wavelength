#pragma once

#include "app/rampanimator.h"
#include "gfx/gfxtunnel.h"
#include "gfx/particlearm.h"
#include "gfx/substepparticles.h"
#include "math/vector3.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndparticlesys.h"
#include "rnd/rndtransformable.h"
#include "rnd/rndview.h"

/**
 * The freestylus, the instrument a ship becomes for a freestyle.
 *
 * The class is not polymorphic, and the name comes from the names of its views. The object is
 * 0xe0 bytes, and the tunnel has one, which the freestyles of every player share. The stick of
 * the player tumbles the freestylus, and the beams grow while the player plays.
 */
class Freestylus {
public:
    /** The kinds of freestyle. */
    enum Type {
        kTypeAxe = 1,     /*!< The guitar freestyle, `axe`. */
        kTypeScratch = 3, /*!< The scratch freestyle, `scratch`. */
    };

    /**
     * Find the views, animations, and particle systems of the freestylus.
     *
     * @param pTunnel The tunnel, which the constructor does not read.
     * @param nType The kind of freestyle, one of Type.
     * @ghidraAddress NTSC-U/C: 0x001e7570
     * @ghidraAddress PAL: 0x001f0310
     */
    Freestylus(GfxTunnel *pTunnel, int nType);

    /**
     * Destroy the freestylus.
     *
     * @ghidraAddress NTSC-U/C: 0x001e7a80
     * @ghidraAddress PAL: 0x001f0820
     */
    ~Freestylus();

    /**
     * Scale the freestylus and its particles, the first time only.
     *
     * @param flScale The scale, `freestyler_scale`.
     * @ghidraAddress NTSC-U/C: 0x001e7ae8
     * @ghidraAddress PAL: 0x001f0888
     */
    void Setup(float flScale);

    /**
     * End the flash and restart the particles of the button.
     *
     * @ghidraAddress NTSC-U/C: 0x001e7c58
     * @ghidraAddress PAL: 0x001f09f8
     */
    void Reset();

    /**
     * Pose the pitch and the colour of the freestylus.
     *
     * @param flPitch The pitch, from 0 to 1.
     * @param flColor The colour, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001e7c88
     * @ghidraAddress PAL: 0x001f0a28
     */
    void SetPitchColor(float flPitch, float flColor);

    /**
     * Turn the basis of the tumble about its X and Y axes, and keep it orthonormal.
     *
     * @param flTime The time since the last update.
     * @param flTumbleX The rate of the turn about the Y axis.
     * @param flTumbleY The rate of the turn about the X axis.
     * @ghidraAddress NTSC-U/C: 0x001e7ce0
     * @ghidraAddress PAL: 0x001f0a80
     */
    void Spin(float flTime, float flTumbleX, float flTumbleY);

    /**
     * Start the flash.
     *
     * @ghidraAddress NTSC-U/C: 0x001e80b8
     * @ghidraAddress PAL: 0x001f0e58
     */
    void Flash();

    /**
     * Free every particle of the freestylus and of its arm.
     *
     * @ghidraAddress NTSC-U/C: 0x001e80d0
     * @ghidraAddress PAL: 0x001f0e70
     */
    void ClearParticles();

    /**
     * Advance the animations and scale the freestylus.
     *
     * @param bPlaying Whether the player plays, which advances the particles of the button.
     * @param flTicks The ticks since the last update, which the freestylus does not read.
     * @param flTime The time since the last update.
     * @param flScale The scale of the assembly, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001e8130
     * @ghidraAddress PAL: 0x001f0ed0
     */
    void Update(bool bPlaying, float flTicks, float flTime, float flScale);

    /**
     * Draw the freestylus under a parent.
     *
     * @param pParent The transform the freestylus hangs from.
     * @param bAssembled Whether the ship has assembled, which the freestylus does not read.
     * @ghidraAddress NTSC-U/C: 0x001e81f8
     * @ghidraAddress PAL: 0x001f0f98
     */
    void Draw(RndTransformable *pParent, int bAssembled);

    /**
     * The angle of the tumble per unit of its rate, `tumble_angle_per_ms`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afab0
     */
    static float sTumbleAnglePerMs;

    RndView *mView;             /*!< `freestylus.view`. */
    RndView *mTempo;            /*!< `freestylus tempo.view`. */
    RndAnimatable *mPitch;      /*!< `freestylus pitch.view`. */
    RndAnimatable *mColor;      /*!< `freestylus color.mnm`. */
    RndView *mTumble;           /*!< `freestylus tumble.view`. */
    RndView *mBeams;            /*!< `freestylus beams.view`. */
    RampAnimator mBeamGrow;     /*!< `freestylus beamgrow.mnm`. */
    RampAnimator mSample;       /*!< `freestylus sample.tnm`. */
    ParticleArm mArm;           /*!< `freestylus2.part` on the view. */
    RndParticleSys *mParticles; /*!< `freestylus.part`. */
    SubstepParticles mButton;   /*!< `freestylus button 0.part`. */
    Vector3 mButtonForce;       /*!< The force of the button particles before the tumble. */
    float mTime;                /*!< The time the button particles have run. */
    RndView *mFlash;            /*!< `freestylus flash.view`. */
    float mFlashTime;           /*!< The time the flash started, or kNoFlash. */
    Vector3 mBasis[3];          /*!< The basis of the tumble. */
    float mScale;               /*!< The scale Setup() gave, or kNotSetUp. */

    /** The value of mFlashTime while no flash plays. */
    static constexpr float kNoFlash = -1e9f;

    /** The value of mScale before Setup(). */
    static constexpr float kNotSetUp = -1.0f;
};
