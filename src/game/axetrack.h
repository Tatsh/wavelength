#pragma once

#include "game/axecontour.h"
#include "game/freestyletrack.h"
#include "game/playmap.h"
#include "game/sectionlist.h"

/**
 * Freestyle track played as a guitar.
 *
 * The RTTI records the class as deriving from FreestyleTrack. The object is 0x88 bytes, and
 * GameLogic allocates it under the tag "AxeTrack". Only the members GameLogic uses are declared.
 */
class AxeTrack : public FreestyleTrack {
public:
    /**
     * Construct a guitar track.
     *
     * @param pContour The notes of the track.
     * @param pSections The sections of the song.
     * @param pPlayMap The map of the song positions.
     * @param pfMsPerTick The duration of one tick in milliseconds.
     * @param nIndex The index of the track in the song.
     * @param nIntroBars The bars before the first section.
     * @param nNumBars The length of the song in bars.
     * @param nTicksPerBar The length of a bar in ticks.
     * @ghidraAddress NTSC-U/C: 0x00147438
     * @ghidraAddress PAL: 0x00148df8
     */
    AxeTrack(AxeContour *pContour,
             SectionList *pSections,
             PlayMap *pPlayMap,
             const float *pfMsPerTick,
             int nIndex,
             int nIntroBars,
             int nNumBars,
             int nTicksPerBar);

    /**
     * Release the track.
     *
     * @ghidraAddress NTSC-U/C: 0x00147688
     * @ghidraAddress PAL: 0x00149048
     */
    ~AxeTrack() override;

    /**
     * Start the track.
     *
     * @ghidraAddress NTSC-U/C: 0x00147768
     * @ghidraAddress PAL: 0x00149128
     */
    void Start() override;

    /**
     * Stop the track.
     *
     * @ghidraAddress NTSC-U/C: 0x00147798
     * @ghidraAddress PAL: 0x00149158
     */
    void Stop() override;

    /**
     * Give the track to the player it plays for.
     *
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x00147888
     * @ghidraAddress PAL: 0x00149248
     */
    void SetPlayer(Player *pPlayer) override;

    using FreestyleTrack::HandleInput;

    /**
     * Start a note.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x001478c8
     * @ghidraAddress PAL: 0x00149288
     */
    void HandleInput(Player *pPlayer, const PlayNoteEvent &event) override;

    /**
     * Move the note being played.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00147980
     * @ghidraAddress PAL: 0x00149340
     */
    void HandleInput(Player *pPlayer, const StickEvent<2> &event) override;

    /**
     * Ignore the input.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00347da8
     * @ghidraAddress PAL: 0x003b51d8
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const BtnEvent<10> &event) override {
    }
};
