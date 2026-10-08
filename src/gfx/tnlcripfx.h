#pragma once

#include "gfx/gfxtunnel.h"
#include "math/transform.h"
#include "math/vector3.h"
#include "rnd/particlesys.h"
#include "rnd/view.h"

/**
 * The crippler one player sends at another, which flies down the track of the victim and then
 * sends a wave along it that bends and twists the panels.
 *
 * The RTTI identifies the class, which is not polymorphic. The object is 0x80 bytes. Some members
 * hold one value while the crippler flies and another once the wave runs.
 */
class TnlCripFX {
public:
    /** The values of mState. */
    enum State {
        kStateIdle = 0, /*!< No crippler. */
        kStateFly = 1,  /*!< The crippler flies down the track. */
        kStateWave = 2, /*!< The wave runs along the track. */
    };

    /**
     * Construct an idle crippler, and for the first one the shared particles.
     *
     * @param nIndex The index of the crippler.
     * @ghidraAddress NTSC-U/C: 0x001f10f8
     * @ghidraAddress PAL: 0x001f9e98
     */
    explicit TnlCripFX(int nIndex);

    /**
     * Release the shared particles.
     *
     * @ghidraAddress NTSC-U/C: 0x001f1280
     * @ghidraAddress PAL: 0x001fa020
     */
    ~TnlCripFX();

    /**
     * Advance the flight or the wave.
     *
     * @param flDelta The time since the last update.
     * @param pTunnel The tunnel.
     * @return Whether the wave ran or ended. A flight and an idle crippler report false.
     * @ghidraAddress NTSC-U/C: 0x001f12b0
     * @ghidraAddress PAL: 0x001fa050
     */
    bool Poll(float flDelta, GfxTunnel *pTunnel);

    /**
     * Draw the crippler, and land it once its flight ends.
     *
     * @param pTunnel The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001f1ae0
     * @ghidraAddress PAL: 0x001fa880
     */
    void Draw(GfxTunnel *pTunnel);

    /**
     * Draw the shared particles at the time of the song.
     *
     * @ghidraAddress NTSC-U/C: 0x001f1d48
     * @ghidraAddress PAL: 0x001faae8
     */
    static void DrawParticles();

    /**
     * Send the crippler at a player.
     *
     * @param pTunnel The tunnel.
     * @param nAttacker The attacking player. The routine does not read it.
     * @param nVictim The crippled player.
     * @ghidraAddress NTSC-U/C: 0x001f1d88
     * @ghidraAddress PAL: 0x001fab28
     */
    void Start(GfxTunnel *pTunnel, char nAttacker, char nVictim);

    /**
     * Stop the crippler.
     *
     * @param pTunnel The tunnel, or null.
     * @param bRestore Whether a running wave puts the panels it bent back.
     * @ghidraAddress NTSC-U/C: 0x001f1e08
     * @ghidraAddress PAL: 0x001faba8
     */
    void Stop(GfxTunnel *pTunnel, bool bRestore);

    /**
     * Report the track of a wave that has reached the victim.
     *
     * @return The track, or -1.
     * @ghidraAddress NTSC-U/C: 0x001f1eb0
     * @ghidraAddress PAL: 0x001fac50
     */
    char ArrivedWaveTrack() const;

    /**
     * Report the track of a running wave and the track it bent before.
     *
     * @param pnTrack Receives the track, or -1.
     * @param pnPrevTrack Receives the track before, or -1.
     * @ghidraAddress NTSC-U/C: 0x001f1ee0
     * @ghidraAddress PAL: 0x001fac80
     */
    void GetWaveTracks(char *pnTrack, char *pnPrevTrack) const;

    /** The milliseconds of the flight, `crippler_fly_time`. @ghidraAddress NTSC-U/C: 0x003afb28 */
    static float sFlyTime;
    /** The milliseconds the view shows after landing. @ghidraAddress NTSC-U/C: 0x003afb2c */
    static float sFadeTime;
    /** The milliseconds of the wave, `crippler_wave_time`. @ghidraAddress NTSC-U/C: 0x003afb30 */
    static float sWaveTime;
    /** The radians per tick of the two waves. @ghidraAddress NTSC-U/C: 0x003afb34 */
    static float sSpatialFreqs[2];
    /** The radians per millisecond of the two waves. @ghidraAddress NTSC-U/C: 0x003afb3c */
    static float sTimeFreqs[2];
    /** `crippler_wave_death_length`. The image never reads it. @ghidraAddress NTSC-U/C: 0x003afb44
     */
    static float sWaveDeathLength;
    /** The heights of the two waves. @ghidraAddress NTSC-U/C: 0x003afb48 */
    static float sWaveMagnitudes[2];
    /** The ticks per millisecond of the flight. @ghidraAddress NTSC-U/C: 0x003afb50 */
    static float sTickVelocity;
    /** The height the flight starts at. @ghidraAddress NTSC-U/C: 0x003afb54 */
    static float sStartHeight;
    /** The rise per millisecond the flight starts with. @ghidraAddress NTSC-U/C: 0x003afb58 */
    static float sStartHeightVelocity;
    /** The change of the rise per millisecond. @ghidraAddress NTSC-U/C: 0x003afb5c */
    static float sHeightAcceleration;
    /** The kick a landing gives the camera. @ghidraAddress NTSC-U/C: 0x003afb60 */
    static float sImpactCamJiggle;
    /** The largest twist of the wave in radians. @ghidraAddress NTSC-U/C: 0x003afb64 */
    static float sMaxTwist;
    /** The radians per tick of the twist. @ghidraAddress NTSC-U/C: 0x003afb68 */
    static float sTwistFreq;
    /** The particles of the cripplers, `crippler.part`. @ghidraAddress NTSC-U/C: 0x003afb8c */
    static Rnd::ParticleSys *sParticles;
    /** The frame of sParticles. @ghidraAddress NTSC-U/C: 0x003afb90 */
    static float sParticleFrame;

    char mState;        /*!< The phase, one of State. */
    char mVictim;       /*!< The crippled player. */
    Rnd::View *mView;   /*!< The view of the crippler, `crippler.view`. */
    Rnd::View *mShadow; /*!< The view of its shadow, `crippler shadow.view`. */
    Transform mXfm;     /*!< The placed transform of the crippler. */
    Vector3 mShadowPos; /*!< The position of the shadow on the track. */
    union {
        float mFlyTick; /*!< While flying, the tick of the crippler. */
        struct {
            char mWaveTrack;     /*!< While the wave runs, the track it bends. */
            char mPrevWaveTrack; /*!< While the wave runs, the track it bent before, or -1. */
        };
    };
    union {
        float mHeight; /*!< While flying, the height of the crippler. */
        int mArrived;  /*!< While the wave runs, whether its front has reached the victim. */
    };
    union {
        float mHeightVelocity; /*!< While flying, the rise per millisecond. */
        float mWaveTick;       /*!< While the wave runs, the tick it started from. */
    };
    float mStartTime; /*!< The time the phase started, in milliseconds. */
    float mDelta;     /*!< The time of the last update. */
};
