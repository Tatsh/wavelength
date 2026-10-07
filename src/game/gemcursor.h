#pragma once

#include "game/catchtrackdata.h"
#include "game/playmap.h"
#include "gs/muse.h"

/**
 * Position at one gem of a catch track, walked in the order the song plays the gems.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. A cursor whose index is -1
 * is past the last gem. The play map repeats stretches of the song, and mOffset maps the tick of
 * the gem in the song as written to the tick the song plays it at.
 */
class GemCursor {
public:
    /**
     * Construct a cursor at the first gem at or after a tick.
     *
     * @param pPlayMap The play map of the song.
     * @param pData The gems.
     * @param nTick The tick the song plays at.
     * @ghidraAddress NTSC-U/C: 0x00117428
     * @ghidraAddress PAL: 0x00118bc0
     */
    GemCursor(PlayMap *pPlayMap, CatchTrackData *pData, int nTick);

    /**
     * Construct a cursor past the last gem of no track.
     *
     * @ghidraAddress NTSC-U/C: 0x001174c8
     * @ghidraAddress PAL: 0x00118c60
     */
    GemCursor();

    /**
     * Copy another cursor.
     *
     * @param other The cursor.
     * @ghidraAddress NTSC-U/C: 0x001174e8
     * @ghidraAddress PAL: 0x00118c80
     */
    GemCursor(const GemCursor &other);

    /**
     * Copy another cursor.
     *
     * @param other The cursor.
     * @return This cursor.
     * @ghidraAddress NTSC-U/C: 0x00117510
     * @ghidraAddress PAL: 0x00118ca8
     */
    GemCursor &operator=(const GemCursor &other);

    /**
     * Report whether the cursor is at a gem.
     *
     * @return Whether the index is not -1.
     * @ghidraAddress NTSC-U/C: 0x00117548
     * @ghidraAddress PAL: 0x00118ce0
     */
    bool IsValid() const;

    /**
     * Report the tick the song plays the gem at.
     *
     * @return The tick of the gem plus mOffset.
     * @ghidraAddress NTSC-U/C: 0x00117558
     * @ghidraAddress PAL: 0x00118cf0
     */
    int GetTick() const;

    /**
     * Report the lane of the gem.
     *
     * @return The lane.
     * @ghidraAddress NTSC-U/C: 0x00117590
     * @ghidraAddress PAL: 0x00118d28
     */
    int GetLane() const;

    /**
     * Report the music of the gem.
     *
     * @return The music.
     * @ghidraAddress NTSC-U/C: 0x001175b8
     * @ghidraAddress PAL: 0x00118d50
     */
    Muse *GetMuse() const;

    /**
     * Move to the next gem.
     *
     * @ghidraAddress NTSC-U/C: 0x001175e0
     * @ghidraAddress PAL: 0x00118d78
     */
    void Advance();

    /**
     * Move to the first gem the song plays at or after a tick, following the loops of the play
     * map.
     *
     * @param nTick The tick the song plays at.
     * @param bSkipCurrent Whether to step past the current gem first.
     * @param pIsLast Receives whether the search arrived at the last loop of the play map.
     * @ghidraAddress NTSC-U/C: 0x00117620
     * @ghidraAddress PAL: 0x00118db8
     */
    void AdvanceToTick(int nTick, bool bSkipCurrent, int *pIsLast);

    /**
     * Move to the next gem in a lane.
     *
     * In a loop that repeats without end, the cursor moves past the last gem once it returns to
     * the gem it started from.
     *
     * @param nLane The lane.
     * @ghidraAddress NTSC-U/C: 0x001177e0
     * @ghidraAddress PAL: 0x00118f78
     */
    void AdvanceToLane(int nLane);

    /**
     * Report whether two cursors are at the same gem.
     *
     * @param other The other cursor.
     * @return Whether both cursors are past the last gem, or both are at one gem of one track.
     * @ghidraAddress NTSC-U/C: 0x001178a0
     * @ghidraAddress PAL: 0x00119038
     */
    bool operator==(const GemCursor &other) const;

    /**
     * Move to the next gem.
     *
     * @return A copy of the cursor at its new position.
     * @ghidraAddress NTSC-U/C: 0x00117908
     * @ghidraAddress PAL: 0x001190a0
     */
    GemCursor Next();

private:
    CatchTrackData *mData; /*!< The gems. */
    PlayMap *mPlayMap;     /*!< The play map of the song. */
    int mIndex;            /*!< The index of the gem, or -1 past the last gem. */
    int mOffset;           /*!< The ticks added to the tick of each gem. */
};
