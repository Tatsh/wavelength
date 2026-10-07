#pragma once

#include "game/freestyletrack.h"
#include "game/playmap.h"
#include "game/scratchdata.h"
#include "game/sectionlist.h"

/**
 * Freestyle track played as a turntable.
 *
 * The RTTI records the class as deriving from FreestyleTrack. The object is 0x84 bytes, and
 * GameLogic allocates it under the tag "ScratchTrack". Only the members GameLogic uses are
 * declared.
 */
class ScratchTrack : public FreestyleTrack {
public:
    /**
     * Construct a turntable track.
     *
     * @param pData The patterns of the track.
     * @param pSections The sections of the song.
     * @param pPlayMap The map of the song positions.
     * @param pfMsPerTick The duration of one tick in milliseconds.
     * @param nIndex The index of the track in the song.
     * @param nIntroBars The bars before the first section.
     * @param nNumBars The length of the song in bars.
     * @param nTicksPerBar The length of a bar in ticks.
     * @ghidraAddress NTSC-U/C: 0x00137540
     * @ghidraAddress PAL: 0x00138da0
     */
    ScratchTrack(ScratchData *pData,
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
     * @ghidraAddress NTSC-U/C: 0x00137780
     * @ghidraAddress PAL: 0x00138fe0
     */
    ~ScratchTrack() override;

    /**
     * Start the track.
     *
     * @ghidraAddress NTSC-U/C: 0x00137838
     * @ghidraAddress PAL: 0x00139098
     */
    void Start() override;

    /**
     * Stop the track.
     *
     * @ghidraAddress NTSC-U/C: 0x00137858
     * @ghidraAddress PAL: 0x001390b8
     */
    void Stop() override;

    /**
     * Give the track to the player it plays for.
     *
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x00137900
     * @ghidraAddress PAL: 0x00139160
     */
    void SetPlayer(Player *pPlayer) override;

    using FreestyleTrack::HandleInput;

    /**
     * Start a scratch.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00137940
     * @ghidraAddress PAL: 0x001391a0
     */
    void HandleInput(Player *pPlayer, const PlayNoteEvent &event) override;

    /**
     * Move the scratch being played.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x001379f8
     * @ghidraAddress PAL: 0x00139258
     */
    void HandleInput(Player *pPlayer, const StickEvent<2> &event) override;

    /**
     * End the scratch being played.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00137ab0
     * @ghidraAddress PAL: 0x00139310
     */
    void HandleInput(Player *pPlayer, const BtnEvent<10> &event) override;
};
