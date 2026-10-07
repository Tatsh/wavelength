#pragma once

#include "gs/axecontour.h"
#include "gs/axeharmony.h"

/**
 * Player of one guitar contour fitted to a harmony. The stick transposes the contour and sweeps
 * the filter.
 *
 * The class is not polymorphic. The RTTI of AxeTrack's vector of players includes the class name.
 * AxeTrack builds one for each gem button. While active the voice sets the software effects of the
 * synthesiser.
 */
class Axer {
public:
    /**
     * Construct an inactive voice.
     *
     * @param pContour The contour.
     * @param pHarmony The harmony.
     * @param nReserved Stored and not read here.
     * @param fOutputLevel The output level of the synthesiser while the voice is active.
     * @ghidraAddress NTSC-U/C: 0x00157628
     * @ghidraAddress PAL: 0x00158eb0
     */
    Axer(AxeContour *pContour, const AxeHarmony *pHarmony, int nReserved, float fOutputLevel);

    /**
     * Deactivate the voice.
     *
     * @ghidraAddress NTSC-U/C: 0x00157678
     * @ghidraAddress PAL: 0x00158f00
     */
    ~Axer();

    /**
     * Change the contour, carrying the position of a playing contour over to the new one.
     *
     * @param pContour The contour.
     * @ghidraAddress NTSC-U/C: 0x001576c0
     * @ghidraAddress PAL: 0x00158f48
     */
    void SetContour(AxeContour *pContour);

    /**
     * Change the harmony and fit the contour to it.
     *
     * @param pHarmony The harmony.
     * @ghidraAddress NTSC-U/C: 0x00157738
     * @ghidraAddress PAL: 0x00158fc0
     */
    void SetHarmony(const AxeHarmony *pHarmony);

    /**
     * Report the ticks of one loop of the contour.
     *
     * @return The ticks.
     * @ghidraAddress NTSC-U/C: 0x00157758
     * @ghidraAddress PAL: 0x00158fe0
     */
    int GetLength();

    /**
     * Start playing the contour unless the voice is active.
     *
     * @param nPosition The position in the loop.
     * @ghidraAddress NTSC-U/C: 0x00157778
     * @ghidraAddress PAL: 0x00159000
     */
    void Activate(int nPosition);

    /**
     * Stop playing and restore the full output level.
     *
     * @ghidraAddress NTSC-U/C: 0x00157800
     * @ghidraAddress PAL: 0x00159088
     */
    void Deactivate();

    /**
     * Report whether the voice is active.
     *
     * @return Non-zero while active.
     * @ghidraAddress NTSC-U/C: 0x00157858
     * @ghidraAddress PAL: 0x001590e0
     */
    int IsActive() const;

    /**
     * Transpose the contour within the range of the harmony from the horizontal stick position.
     *
     * @param fX The horizontal stick position, from -1 to 1.
     * @ghidraAddress NTSC-U/C: 0x00157860
     * @ghidraAddress PAL: 0x001590e8
     */
    void SetX(float fX);

    /**
     * Set the sweep of the software filter. A change of 0.02 or less is ignored.
     *
     * @param fSweep The sweep.
     * @ghidraAddress NTSC-U/C: 0x00157940
     * @ghidraAddress PAL: 0x001591c8
     */
    void SetSweep(float fSweep);

private:
    /**
     * Fit the contour to the harmony.
     *
     * @param nTranspose The notes the contour moves by.
     * @ghidraAddress NTSC-U/C: 0x00157990
     * @ghidraAddress PAL: 0x00159218
     */
    void FitContour(int nTranspose);

    /**
     * Send the sweep to the synthesiser.
     *
     * @param fSweep The sweep.
     * @ghidraAddress NTSC-U/C: 0x001579b8
     * @ghidraAddress PAL: 0x00159240
     */
    void ApplySweep(float fSweep);

    /**
     * Do nothing. Activate() calls it.
     *
     * @ghidraAddress NTSC-U/C: 0x001579e8
     * @ghidraAddress PAL: 0x00159270
     */
    void ApplyReserved();

    AxeContour *mContour;       /*!< The contour. */
    const AxeHarmony *mHarmony; /*!< The harmony. */
    int mReserved08;            // +0x08, stored by the constructor, not read here.
    float mOutputLevel;         /*!< The output level of the synthesiser while active. */
    unsigned char mChannel;     /*!< The MIDI channel of the contour. */
    int mTranspose;             /*!< The notes the contour is moved by. */
    float mSweep;               /*!< The sweep of the software filter. */
    int mActive;                /*!< Whether the voice is active. */
};
