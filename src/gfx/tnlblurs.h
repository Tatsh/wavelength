#pragma once

#include <list>

#include "gfx/gfxtunnel.h"
#include "script/dataarray.h"

/**
 * The full-screen blurs of the tunnel, each a span of ticks with an opacity and an enlargement.
 *
 * The class is not polymorphic. RTTI identifies its nested ActiveBlur. The object is 0x34 bytes.
 * The tunnel queues a blur for each phase of a slowdown, and each update moves the opacity and the
 * enlargement of the screen blur toward those of the blur at the song tick by at most the
 * configured steps.
 */
class TnlBlurs {
public:
    /** A queued blur. */
    struct ActiveBlur {
        float mStartTick; /*!< The tick the blur starts at. */
        float mEndTick;   /*!< The tick the blur ends at. */
        float mAlpha;     /*!< The opacity of the blur. */
        int mEnlarge;     /*!< The offset of the blurred copy of the frame, in pixels. */
    };

    /**
     * Make an empty queue with no configuration.
     *
     * @ghidraAddress NTSC-U/C: 0x001d5aa0
     * @ghidraAddress PAL: 0x001de840
     */
    TnlBlurs();

    /**
     * Destroy the queue.
     *
     * @ghidraAddress NTSC-U/C: 0x001d5b30
     * @ghidraAddress PAL: 0x001de8d0
     */
    ~TnlBlurs();

    /**
     * Load the steps and the kinds of blur from the `blurs` entry.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel. The routine does not read it.
     * @param nOption The fourth argument every loader of the tunnel receives. The routine does not
     *                read it.
     * @ghidraAddress NTSC-U/C: 0x001d5b80
     * @ghidraAddress PAL: 0x001de920
     */
    void LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption);

    /**
     * Empty the queue and clear the opacity and the enlargement.
     *
     * @ghidraAddress NTSC-U/C: 0x001d5c60
     * @ghidraAddress PAL: 0x001dea00
     */
    void Reset();

    /**
     * Drop the blurs that have ended, step toward the blur at the song tick, and set the screen
     * blur, or turn it off at an opacity of 0.
     *
     * @ghidraAddress NTSC-U/C: 0x001d5c90
     * @ghidraAddress PAL: 0x001dea30
     */
    void Update();

    /**
     * Queue a blur of a kind in the order of the start ticks.
     *
     * The kind gives offsets to the two ticks, and the opacity and the enlargement of the blur. A
     * blur that ends at or before its start is not queued.
     *
     * @param pszKind The kind, an entry of the `blurs` entry.
     * @param flStartTick The tick the blur starts at, before the offset of the kind.
     * @param flEndTick The tick the blur ends at, before the offset of the kind.
     * @ghidraAddress NTSC-U/C: 0x001d5eb8
     * @ghidraAddress PAL: 0x001dec58
     */
    void Insert(const char *pszKind, float flStartTick, float flEndTick);

    std::list<ActiveBlur> mBlurs; /*!< The queued blurs in the order of the start ticks. */
    float mAlpha;                 /*!< The opacity of the screen blur. */
    float mAlphaChangeUp;         /*!< The largest rise of the opacity per update. */
    float mAlphaChangeDown;       /*!< The largest fall of the opacity per update, negative. */
    int mEnlarge;                 /*!< The enlargement of the screen blur, in pixels. */
    int mEnlargeChangeUp;         /*!< The largest rise of the enlargement per update. */
    int mEnlargeChangeDown;       /*!< The largest fall of the enlargement per update, negative. */
    DataArray *mConfig;           /*!< The `blurs` entry, the kinds of blur. */
    const char *mStartOffsetKey;  /*!< The symbol `start_offset`. */
    const char *mEndOffsetKey;    /*!< The symbol `end_offset`. */
    const char *mAlphaKey;        /*!< The symbol `alpha`. */
    const char *mEnlargeKey;      /*!< The symbol `enlarge`. */
};
