#pragma once

#include "game/playmap.h"
#include "game/sectionboundaries.h"
#include "game/track.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Track the player plays freely with the analog sticks rather than by hitting gems.
 *
 * The RTTI records the class as deriving from Track. AxeTrack and ScratchTrack derive from it.
 * Only the members a ScratchTrack uses are declared.
 */
class FreestyleTrack : public Track {
public:
    /**
     * Construct a track.
     *
     * @param pSections The section boundaries of the song.
     * @param pPlayMap The map of the song positions.
     * @param pfMsPerTick The length of a tick, in milliseconds.
     * @param nIndex The track's index in the song.
     * @param nNumBars The length of the song, in bars.
     * @param nTicksPerBar The length of a bar, in ticks.
     * @ghidraAddress NTSC-U/C: 0x0014ef80
     * @ghidraAddress PAL: 0x001508e0
     */
    FreestyleTrack(const SectionBoundaries *pSections,
                   PlayMap *pPlayMap,
                   const float *pfMsPerTick,
                   int nIndex,
                   int nNumBars,
                   int nTicksPerBar);

    /**
     * Release the track.
     *
     * @ghidraAddress NTSC-U/C: 0x0014f078
     * @ghidraAddress PAL: 0x001509d8
     */
    ~FreestyleTrack() override;

    /**
     * Stop the track with the song. The member does nothing in this class.
     *
     * @ghidraAddress NTSC-U/C: 0x0014f0e8
     * @ghidraAddress PAL: 0x00150a48
     */
    void Stop() override;

    /**
     * Assign the player and restart the rumble.
     *
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x0014f0f0
     * @ghidraAddress PAL: 0x00150a50
     */
    void SetPlayer(Player *pPlayer) override;

    using Track::HandleInput;

    /**
     * Act on a note the track's player played.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0014f258
     * @ghidraAddress PAL: 0x00150bb8
     */
    void HandleInput(Player *pPlayer, const PlayNoteEvent &event) override;

    /**
     * Ignore the event.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0034b6d8
     * @ghidraAddress PAL: 0x003b8b08
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const BtnEvent<8> &event) override {
    }

    /**
     * Pass a stick position of the track's player to the graphics.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0014f330
     * @ghidraAddress PAL: 0x00150c90
     */
    void HandleInput(Player *pPlayer, const StickEvent<2> &event) override;

    /**
     * Pass a stick position of the track's player to the graphics.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0014f3b8
     * @ghidraAddress PAL: 0x00150d18
     */
    void HandleInput(Player *pPlayer, const StickEvent<6> &event) override;

    /**
     * Report whether the track has an active note.
     *
     * @return True while a note sounds.
     */
    virtual bool IsActive() = 0;

    /** Rebuild what the track plays from the song position. */
    virtual void Refresh() = 0;

    /**
     * Wrap a tick into the repeat period of the track.
     *
     * @param nTick The tick.
     * @return The tick within the period.
     * @ghidraAddress NTSC-U/C: 0x0014f930
     * @ghidraAddress PAL: 0x00151290
     */
    int WrapTick(int nTick);

    /**
     * Report the tick at which the span that starts at a tick ends.
     *
     * @param nTick The tick.
     * @param nStart The first tick of the span.
     * @param nEnd The tick after the span, or -1 when the span has no end.
     * @return The end tick.
     * @ghidraAddress NTSC-U/C: 0x0014f980
     * @ghidraAddress PAL: 0x001512e0
     */
    int SpanEnd(int nTick, int nStart, int nEnd);

    int mTickOffset; /*!< The offset of the game tick from the song tick after a restart. */
};
