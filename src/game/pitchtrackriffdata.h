#pragma once

#include <vector>

#include "gs/muse.h"
#include "mid/tickobj.h"
#include "os/ptr.h"
#include "os/string.h"

/**
 * The riffs a pitch track plays, one set of riffs for each tick that starts a set.
 *
 * The class is not polymorphic. The name comes from the RTTI of its nested class RiffSet. The
 * member names are inferred.
 */
class PitchTrackRiffData {
public:
    /** The riff of each gem button. */
    struct RiffSet {
        Ptr<Muse> mRiffs[3]; /*!< The riff of each gem button. */
    };

    /**
     * Construct riff data with no riffs.
     *
     * @ghidraAddress NTSC-U/C: 0x0012ca98
     * @ghidraAddress PAL: 0x0012e270
     */
    PitchTrackRiffData();

    /**
     * Release the riffs.
     *
     * @ghidraAddress NTSC-U/C: 0x0012caf0
     * @ghidraAddress PAL: 0x0012e2c8
     */
    ~PitchTrackRiffData();

    /**
     * Add the riffs that start at a tick.
     *
     * @param nTick The tick.
     * @param pRiff0 The riff of the first gem button.
     * @param pRiff1 The riff of the second gem button.
     * @param pRiff2 The riff of the third gem button.
     * @ghidraAddress NTSC-U/C: 0x0012cbc0
     * @ghidraAddress PAL: 0x0012e398
     */
    void AddRiffs(int nTick, Muse *pRiff0, Muse *pRiff1, Muse *pRiff2);

    /**
     * Report the riffs of a set.
     *
     * @param nIndex The set.
     * @param ppRiff0 Receives the riff of the first gem button.
     * @param ppRiff1 Receives the riff of the second gem button.
     * @param ppRiff2 Receives the riff of the third gem button.
     * @ghidraAddress NTSC-U/C: 0x0012cf00
     * @ghidraAddress PAL: 0x0012e6d8
     */
    void GetRiffs(int nIndex, Muse **ppRiff0, Muse **ppRiff1, Muse **ppRiff2) const;

    /**
     * Report the riff of one gem button at a tick.
     *
     * @param nTick The tick.
     * @param nSlot The gem button, 0 to 2.
     * @return The riff, or null when no set starts at or before the tick or the button is out of
     *     range.
     * @ghidraAddress NTSC-U/C: 0x0012cf70
     * @ghidraAddress PAL: 0x0012e748
     */
    Muse *GetRiff(int nTick, int nSlot) const;

    /**
     * Find the last set that starts at or before a tick.
     *
     * @param nTick The tick.
     * @return The set, or -1.
     * @ghidraAddress NTSC-U/C: 0x0012d030
     * @ghidraAddress PAL: 0x0012e808
     */
    int FindRiffs(int nTick) const;

    /**
     * Set whether the track takes part in a remix.
     *
     * @param bEnabled Whether the track takes part.
     * @ghidraAddress NTSC-U/C: 0x0012d160
     * @ghidraAddress PAL: 0x0012e938
     */
    void SetEnabled(bool bEnabled);

    /**
     * Report whether the track takes part in a remix.
     *
     * @return mEnabled.
     * @ghidraAddress NTSC-U/C: 0x0012d168
     * @ghidraAddress PAL: 0x0012e940
     */
    bool IsEnabled() const;

private:
    /**
     * Build an entry for a set of riffs.
     *
     * Inline. AddRiffs() and FindRiffs() expand it.
     *
     * @param nTick The tick the set starts at.
     * @param pRiff0 The riff of the first gem button.
     * @param pRiff1 The riff of the second gem button.
     * @param pRiff2 The riff of the third gem button.
     * @return The entry.
     */
    static TickObj<RiffSet> MakeEntry(int nTick, Muse *pRiff0, Muse *pRiff1, Muse *pRiff2) {
        TickObj<RiffSet> entry{Sch::Tick(),
                               {{Ptr<Muse>(pRiff0), Ptr<Muse>(pRiff1), Ptr<Muse>(pRiff2)}}};
        entry.mPosition.mTick = nTick;
        return entry;
    }

    /**
     * Order entries by tick.
     *
     * Inline. The searches of mRiffSets expand it.
     *
     * @param first The first entry.
     * @param second The second entry.
     * @return Whether first starts before second.
     */
    static bool TickBefore(const TickObj<RiffSet> &first, const TickObj<RiffSet> &second) {
        return first.mPosition.mTick < second.mPosition.mTick;
    }

public:
    std::vector<TickObj<RiffSet>> mRiffSets; /*!< The sets, by the tick they start at. */
    int mReserved10;                         // +0x10, set to 1 and not read by any routine here.
    bool mEnabled;                           /*!< Whether the track takes part in a remix. */
    String mName;                            /*!< The name of the track. */
};
