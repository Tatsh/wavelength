#pragma once

#include "game/pitchtrackriffdata.h"
#include "game/playmap.h"
#include "gs/muse.h"

/**
 * The riffs of a pitch track, addressed by the ticks the song plays at.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred.
 */
class PitchTrackRiffs {
public:
    /**
     * Construct a view of riffs.
     *
     * @param pRiffData The riffs.
     * @param pPlayMap The map of the song positions.
     * @ghidraAddress NTSC-U/C: 0x0012d170
     * @ghidraAddress PAL: 0x0012e948
     */
    PitchTrackRiffs(PitchTrackRiffData *pRiffData, PlayMap *pPlayMap);

    /**
     * Release the view. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x0012d180
     * @ghidraAddress PAL: 0x0012e958
     */
    ~PitchTrackRiffs();

    /**
     * Report the riff of a gem button at the current tick of the song.
     *
     * @param nSlot The gem button.
     * @return The riff.
     * @ghidraAddress NTSC-U/C: 0x0012d1a8
     * @ghidraAddress PAL: 0x0012e980
     */
    Muse *GetRiff(int nSlot);

    /**
     * Report the riff of a gem button at a tick the song plays at.
     *
     * @param nSlot The gem button.
     * @param nTick The tick.
     * @return The riff.
     * @ghidraAddress NTSC-U/C: 0x0012d1d0
     * @ghidraAddress PAL: 0x0012e9a8
     */
    Muse *GetRiff(int nSlot, int nTick);

private:
    PitchTrackRiffData *mRiffData; /*!< The riffs. */
    PlayMap *mPlayMap;             /*!< The map of the song positions. */
};
