#pragma once

#include <list>

#include "gfx/gfxtunnel.h"
#include "gfx/tnlgem.h"
#include "gfx/tnlgems.h"
#include "gfx/tnltrackrange.h"
#include "gfx/tnlwave.h"
#include "math/vector3.h"
#include "rnd/line.h"
#include "rnd/mat.h"
#include "rnd/particle.h"
#include "rnd/particlesys.h"
#include "script/dataarray.h"

/**
 * The beam that links the gems ahead of a player's cursor on its track.
 *
 * The class is not polymorphic, and the name is inferred from the `connector_beam` section it
 * loads. The object is 0xa4 bytes. The beam is a line through the gems of a window of ticks, which
 * sways with a wave across the track and pulses in time with the beat. Each linked gem has a glow
 * particle.
 */
class TnlConnectorBeam {
public:
    /** A gem the beam links, 0x10 bytes. */
    struct Gem {
        Rnd::Particle *mParticle; /*!< The glow particle, or null. */
        int mOwnParticle;         /*!< Whether mParticle came from mGlow rather than the gem. */
        float mTick;              /*!< The tick of the gem. */
        float mLateral;           /*!< The position across the track, from 0 to 1. */
    };

    /** A straight piece of the beam between two points, 0x50 bytes. */
    struct Span {
        float mTick;        /*!< The tick at the start. */
        float mTickLength;  /*!< The ticks the span covers. */
        float mPhase;       /*!< The position of the start in the window, in cycles. */
        float mPhaseLength; /*!< The cycles the span covers. */
        float mGemPhase;    /*!< The position of the first gem of the pair, in cycles. */
        float mGemPhaseGap; /*!< The cycles between the two gems of the pair. */
        float mArchPhase;   /*!< 0 or pi, alternating from one pair of gems to the next. */
        float mReserved1c;  // +0x1c
        Vector3 mStart;     /*!< The position at the start. */
        Vector3 mDirection; /*!< The step from the start to the end. */
        Vector3 mSwayAxis;  /*!< The unit axis the beam sways along. */
    };

    /**
     * Find the line, the glow particles, and the material of a player's beam.
     *
     * @param pTunnel The tunnel.
     * @param nPlayer The player, which selects `gem_glow<player>.ps`.
     * @param pszColor The colour of the player, whose first letter selects `sabre_<letter>.str`.
     * @param nExcludedType A gem type the beam never links.
     * @param nOtherExcludedType A second gem type the beam never links.
     * @ghidraAddress NTSC-U/C: 0x001f9c88
     * @ghidraAddress PAL: 0x00202a28
     */
    TnlConnectorBeam(GfxTunnel *pTunnel,
                     int nPlayer,
                     const char *pszColor,
                     char nExcludedType,
                     char nOtherExcludedType);

    /**
     * Release the gems and the spans.
     *
     * @ghidraAddress NTSC-U/C: 0x001f9f78
     * @ghidraAddress PAL: 0x00202d18
     */
    ~TnlConnectorBeam();

    /**
     * Load the shared settings from the `connector_beam` section.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel, whose panel width scales the width of the line.
     * @ghidraAddress NTSC-U/C: 0x001fa010
     * @ghidraAddress PAL: 0x00202db0
     */
    void LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel);

    /**
     * Forget the last pulse and the end of the window, and drop the multiplier colouring.
     *
     * @ghidraAddress NTSC-U/C: 0x001fa2d8
     * @ghidraAddress PAL: 0x00203078
     */
    void Reset();

    /**
     * Move the window onto a track and link its gems.
     *
     * The line is hidden instead when the window is empty, when there are no gems, or when the
     * track is negative.
     *
     * @param nTrack The track.
     * @param pGems The gems, or null.
     * @param flStartTick The start of the window.
     * @param flEndTick The end of the window.
     * @ghidraAddress NTSC-U/C: 0x001fa310
     * @ghidraAddress PAL: 0x002030b0
     */
    void SetWindow(char nTrack, TnlGems *pGems, float flStartTick, float flEndTick);

    /**
     * Rebuild the spans when the tunnel changes a range that overlaps the window.
     *
     * @param pRange The range that changed.
     * @ghidraAddress NTSC-U/C: 0x001fab08
     * @ghidraAddress PAL: 0x002038a8
     */
    void OnRangeChanged(const TnlTrackRange *pRange);

    /**
     * Relink the gems when a gem the beam may link appears inside the window.
     *
     * @param nTrack The track of the gem.
     * @param nType The type of the gem, or -1.
     * @param pGems The gems.
     * @param flTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x001fab50
     * @ghidraAddress PAL: 0x002038f0
     */
    void OnGemAdded(char nTrack, char nType, TnlGems *pGems, float flTick);

    /**
     * Relink the gems when a span of ticks that overlaps the window changes on the track.
     *
     * @param nTrack The track.
     * @param pGems The gems.
     * @param flStartTick The start of the span.
     * @param flEndTick The end of the span.
     * @ghidraAddress NTSC-U/C: 0x001fabe0
     * @ghidraAddress PAL: 0x00203980
     */
    void OnTicksChanged(char nTrack, TnlGems *pGems, float flStartTick, float flEndTick);

    /**
     * Detach the particle of a gem that leaves the tunnel from the linked gems.
     *
     * @param pGem The gem.
     * @ghidraAddress NTSC-U/C: 0x001fac38
     * @ghidraAddress PAL: 0x002039d8
     */
    void OnGemRemoved(const TnlGem *pGem);

    /**
     * Pulse the beam on a beat inside the window.
     *
     * The first pulse after the energy runs out also chooses a new sway frequency.
     *
     * @param flTick The tick of the beat.
     * @ghidraAddress NTSC-U/C: 0x001facf0
     * @ghidraAddress PAL: 0x00203a90
     */
    void Pulse(float flTick);

    /**
     * Set the energy level from the energy model.
     *
     * A rise in the level, or a full level, boosts the glow.
     *
     * @param flEnergy The input of the energy model.
     * @ghidraAddress NTSC-U/C: 0x001fae08
     * @ghidraAddress PAL: 0x00203ba8
     */
    void SetEnergy(float flEnergy);

    /**
     * Colour the glows with the multiplier until a tick and relink the gems.
     *
     * @param pGems The gems.
     * @param flEndTick The tick the multiplier colouring ends at.
     * @ghidraAddress NTSC-U/C: 0x001faea0
     * @ghidraAddress PAL: 0x00203c40
     */
    void StartMultiplier(TnlGems *pGems, float flEndTick);

    /**
     * Drop the multiplier colouring and return the glows to white.
     *
     * @ghidraAddress NTSC-U/C: 0x001faef0
     * @ghidraAddress PAL: 0x00203c90
     */
    void StopMultiplier();

    /**
     * Advance the glows and the energy, and lay the line through the spans.
     *
     * @param bCaptured Whether the track is captured, which leaves the glows unchanged.
     * @param flDelta The ticks since the last poll.
     * @param flAlpha The opacity of the line.
     * @ghidraAddress NTSC-U/C: 0x001faf88
     * @ghidraAddress PAL: 0x00203d28
     */
    void Poll(bool bCaptured, float flDelta, float flAlpha);

    TnlWave mSway;            /*!< The wave across the track, along the length of the beam. */
    TnlWave mBeat;            /*!< The wave in time that scales the sway. */
    float mDecayRate;         /*!< The energy lost per tick. */
    float mMinFrequency;      /*!< The lowest sway frequency, `min_frequency`. */
    float mMaxFrequency;      /*!< The highest sway frequency, `max_frequency`. */
    std::list<Gem> mGems;     /*!< The linked gems, in tick order. */
    std::list<Span> mSpans;   /*!< The spans between the linked gems, in tick order. */
    unsigned char mNumSpans;  /*!< The number of spans. */
    GfxTunnel *mTunnel;       /*!< The tunnel. */
    Rnd::Line *mLine;         /*!< The line, `sabre_<letter>.str`. */
    Rnd::Mat *mLineMat;       /*!< The material of mLine. */
    char mExcludedType;       /*!< A gem type the beam never links. */
    char mOtherExcludedType;  /*!< A second gem type the beam never links. */
    char mTrack;              /*!< The track of the window, or -1. */
    float mStartTick;         /*!< The start of the window. */
    float mEndTick;           /*!< The end of the window. */
    float mInvWindowLength;   /*!< One over the length of the window. */
    float mPulseTick;         /*!< The tick of the last pulse. Earlier gems are not linked. */
    float mLineEndTick;       /*!< The end of the window the line last covered. */
    float mGlowSize;          /*!< The size of the glows before the boost. */
    float mGlowBoost;         /*!< The boost of the glow size, which falls back to 0. */
    float mEnergyLevel;       /*!< The output of the energy model. */
    float mEnergy;            /*!< The energy of the sway, which falls at mDecayRate. */
    Rnd::ParticleSys *mGlow;  /*!< The glow particles, `gem_glow<player>.ps`. */
    Rnd::Mat *mMultiplierMat; /*!< The material whose colour the glows take under a multiplier. */
    int mMultiplier;          /*!< Whether the glows have the multiplier colouring. */
    float mMultiplierEndTick; /*!< The tick the multiplier colouring ends at. */

private:
    /**
     * Link one gem, with its particle or a new glow particle.
     *
     * @param flTick The tick of the gem.
     * @param flLateral The position across the track, from 0 to 1.
     * @param pParticle The particle of the gem, or null.
     * @ghidraAddress NTSC-U/C: 0x001fa560
     * @ghidraAddress PAL: 0x00203300
     */
    void AddGem(float flTick, float flLateral, Rnd::Particle *pParticle);

    /**
     * Rebuild the spans between the linked gems, and move the glows onto the gems.
     *
     * @ghidraAddress NTSC-U/C: 0x001fa6b8
     * @ghidraAddress PAL: 0x00203458
     */
    void RebuildSpans();
};
