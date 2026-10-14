#pragma once

#include "gfx/ship.h"
#include "math/color.h"
#include "math/interpolator.h"
#include "math/transform.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "rnd/rndline.h"
#include "script/dataarray.h"

/**
 * The trail of the scratch freestyle, the ribbon a scratching ship draws as its needle sweeps
 * across the track.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0xb0 bytes. The ribbon
 * is the `scratch ribbon` of the ship, and its points come in pairs that the needle fills one
 * segment at a time.
 */
class TnlSabreTrail {
public:
    /** The values of mState. */
    enum State {
        kStateHidden = 0,  /*!< The ribbon is hidden. */
        kStateDrawing = 1, /*!< The needle draws new segments. */
        kStateShown = 2,   /*!< The ribbon shows the segments it has. */
    };

    /** The first point of the segments, after the two points of the needle. */
    static constexpr int kFirstSegmentPoint = 2;

    /** The value of mHead while no segment is drawn. */
    static constexpr int kNoHead = -4;

    /**
     * Take the scratch ribbon of a ship and the colours of its points.
     *
     * @param nPlayer The player, whose colour the trail takes.
     * @param pShip The ship.
     * @ghidraAddress NTSC-U/C: 0x001e6998
     * @ghidraAddress PAL: 0x001ef738
     */
    TnlSabreTrail(int nPlayer, Ship *pShip);

    /**
     * Destroy the trail.
     *
     * @ghidraAddress NTSC-U/C: 0x001e6b90
     * @ghidraAddress PAL: 0x001ef930
     */
    ~TnlSabreTrail();

    /**
     * Load the settings of the ribbon and of the sweeps of the needle, and make the ribbon blank.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @ghidraAddress NTSC-U/C: 0x001e6bc0
     * @ghidraAddress PAL: 0x001ef960
     */
    void LoadConfig(DataArray *pConfig, DataArray *pDefaults);

    /**
     * Start the next segment of the ribbon, wrapping back to the first one past the last.
     *
     * @param pPos The position the new segment starts at, or null for the end of the last segment.
     * @ghidraAddress NTSC-U/C: 0x001e6db0
     * @ghidraAddress PAL: 0x001efb50
     */
    void AddSegment(const Vector3 *pPos);

    /**
     * Start drawing segments.
     *
     * @ghidraAddress NTSC-U/C: 0x001e6e90
     * @ghidraAddress PAL: 0x001efc30
     */
    void Start();

    /**
     * Stop drawing segments, or wait for the sample in progress to end.
     *
     * @param bForce Whether to stop during a sample.
     * @ghidraAddress NTSC-U/C: 0x001e6ee0
     * @ghidraAddress PAL: 0x001efc80
     */
    void Stop(bool bForce);

    /**
     * Move every point of the ribbon to the origin.
     *
     * @ghidraAddress NTSC-U/C: 0x001e6f60
     * @ghidraAddress PAL: 0x001efd00
     */
    void Clear();

    /**
     * Show or hide the ribbon.
     *
     * @param bShowing Whether the ribbon is shown.
     * @ghidraAddress NTSC-U/C: 0x001e6fc8
     * @ghidraAddress PAL: 0x001efd68
     */
    void SetShowing(bool bShowing);

    /**
     * Sweep the needle to the other side for a note of a scratch, unless a sample sweeps it.
     *
     * @param flTick The tick of the note.
     * @param flDuration The length of the note in ticks.
     * @ghidraAddress NTSC-U/C: 0x001e7058
     * @ghidraAddress PAL: 0x001efdf8
     */
    void Scratch(float flTick, float flDuration);

    /**
     * Start a sample, sweeps of the needle to random places for sSampleDuration ticks.
     *
     * @ghidraAddress NTSC-U/C: 0x001e7100
     * @ghidraAddress PAL: 0x001efea0
     */
    void StartSample();

    /**
     * Advance the sample, starting the next sweep when one ends.
     *
     * @return The place of the needle across the track.
     * @ghidraAddress NTSC-U/C: 0x001e71f0
     * @ghidraAddress PAL: 0x001eff90
     */
    float UpdateSample();

    /**
     * Place the needle at the ship and draw the segment the needle is in.
     *
     * @param flBlend The colour across the stick, from -1 to 1.
     * @param xfm The world transform of the ship.
     * @param pAxis The first axis of the path, which the trail does not read.
     * @ghidraAddress NTSC-U/C: 0x001e72e8
     * @ghidraAddress PAL: 0x001f0088
     */
    void Update(float flBlend, const Transform &xfm, const Vector3 *pAxis);

    /**
     * Report the place a sweep ends at, the other side from the place it starts at.
     *
     * @param flFrom The place the sweep starts at.
     * @param flSweep The width of a sweep.
     * @return The place.
     * @ghidraAddress NTSC-U/C: 0x001e68e0
     * @ghidraAddress PAL: 0x001ef680
     */
    static float SweepTarget(float flFrom, float flSweep);

    /**
     * Report the place a random sweep of a sample ends at, on the other side from where it starts.
     *
     * @param flFrom The place the sweep starts at.
     * @param flSweep The width of a sweep.
     * @return The place.
     * @ghidraAddress NTSC-U/C: 0x001e6900
     * @ghidraAddress PAL: 0x001ef6a0
     */
    static float Wobble(float flFrom, float flSweep);

    /**
     * The angle per tick of the envelope of a sample, `sample_envelope_frequency`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afabc
     */
    static float sSampleFrequency;

    /**
     * The ticks of a sample, `sample_duration`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afac0
     */
    static float sSampleDuration;

    /**
     * The shortest and the longest ticks of a sweep of a sample, `sample_sweep_duration`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b570
     */
    static Vector2 sSampleSweepTicks;

    float mSweep;                   /*!< The width of a sweep, `sweep`. */
    float mNeedleLength;            /*!< The length of the needle, `needle_length`. */
    unsigned char mReserved08[0x8]; // +0x08 to +0x0f are not yet identified.
    Color mColor;                   /*!< The colour of the player. */
    Color mPointColors[3];          /*!< The colours at the ends and the middle of the stick. */
    RndLine *mRibbon;               /*!< The scratch ribbon of the ship. */
    Vector3 mRibbonOffset;          /*!< The translation the ribbon had. */
    LinearInterpolator mNeedle;     /*!< The place of the needle over the ticks of a sweep. */
    int mState;                     /*!< One of State. */
    int mRestart;                   /*!< Whether the next segment starts at the needle. */
    int mSweeping;                  /*!< Whether the needle sweeps. */
    int mNewSegment;                /*!< Whether the next update starts a segment. */
    int mSampling;                  /*!< Whether a sample is in progress. */
    int mStopPending;               /*!< Whether the trail stops at the end of the sample. */
    float mSampleEnd;               /*!< The tick the sample ends at. */
    int mNumPoints;                 /*!< The points of the ribbon. */
    int mHead;                      /*!< The first point of the segment drawn, or kNoHead. */
};
