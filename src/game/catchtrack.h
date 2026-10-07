#pragma once

#include "game/backmusic.h"
#include "game/catchtrackdata.h"
#include "game/player.h"
#include "game/playmap.h"
#include "game/sectionboundaries.h"
#include "game/track.h"

// GameLogic and CatchTrack refer to each other.
class GameLogic;

/**
 * Track whose phrases a player captures by playing them.
 *
 * The RTTI records the class as deriving from Track. The object is 0x148 bytes, and GameLogic
 * allocates it under the tag "CatchTrack". Only the members GameLogic uses are declared.
 */
class CatchTrack : public Track {
public:
    /**
     * Construct a catch track.
     *
     * @param pLogic The logic of the game.
     * @param pData The gems of the track.
     * @param pfMsPerTick The duration of one tick in milliseconds.
     * @param pPlayMap The map of the song positions.
     * @param pSections The sections of the song.
     * @param nIndex The index of the track among the catch tracks.
     * @param nIntroBars The bars before the first section.
     * @param nNumBars The length of the song in bars.
     * @param nTicksPerBar The length of a bar in ticks.
     * @param nFlags The flags the song records for the track.
     * @ghidraAddress NTSC-U/C: 0x0014c4a8
     * @ghidraAddress PAL: 0x0014de48
     */
    CatchTrack(GameLogic *pLogic,
               CatchTrackData *pData,
               const float *pfMsPerTick,
               PlayMap *pPlayMap,
               SectionBoundaries *pSections,
               int nIndex,
               int nIntroBars,
               int nNumBars,
               int nTicksPerBar,
               int nFlags);

    /**
     * Release the track.
     *
     * @ghidraAddress NTSC-U/C: 0x0014c698
     * @ghidraAddress PAL: 0x0014e038
     */
    ~CatchTrack() override;

    /**
     * Start the track.
     *
     * @ghidraAddress NTSC-U/C: 0x0014c8e0
     * @ghidraAddress PAL: 0x0014e280
     */
    void Start() override;

    /**
     * Stop the track.
     *
     * @ghidraAddress NTSC-U/C: 0x0014ca38
     * @ghidraAddress PAL: 0x0014e3d8
     */
    void Stop() override;

    /**
     * Give the track to the player it plays for.
     *
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x0014cb68
     * @ghidraAddress PAL: 0x0014e508
     */
    void SetPlayer(Player *pPlayer) override;

    /**
     * Act on a note a player played on this track.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0014cde0
     * @ghidraAddress PAL: 0x0014e780
     */
    void HandleInput(Player *pPlayer, const PlayNoteEvent &event) override;

    /**
     * Ignore the input.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0034a950
     * @ghidraAddress PAL: 0x003b7d80
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const BtnEvent<8> &event) override {
    }

    /**
     * Ignore the input.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0034a958
     * @ghidraAddress PAL: 0x003b7d88
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const StickEvent<2> &event) override {
    }

    /**
     * Ignore the input.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0034a960
     * @ghidraAddress PAL: 0x003b7d90
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const StickEvent<6> &event) override {
    }

    /**
     * Ignore the input.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0034a968
     * @ghidraAddress PAL: 0x003b7d98
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const BtnEvent<10> &event) override {
    }

    /**
     * Play a background music track while this track plays.
     *
     * @param pMusic The background music.
     * @ghidraAddress NTSC-U/C: 0x0014c798
     * @ghidraAddress PAL: 0x0014e138
     */
    void AddBackMusic(BackMusic *pMusic);

    /**
     * Place a power-up in a bar.
     *
     * @param nBar The bar.
     * @param nPowerup One of GameLogic::Powerup.
     * @ghidraAddress NTSC-U/C: 0x0014cc50
     * @ghidraAddress PAL: 0x0014e5f0
     */
    void SetPowerup(int nBar, int nPowerup);

    /**
     * Enable the phrases of the track from a bar on.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x0014cc90
     * @ghidraAddress PAL: 0x0014e630
     */
    void Enable(int nBar);

    /**
     * Reset the gems and the display of the track to a song tick.
     *
     * The name is inferred.
     *
     * @param pPlayer The player.
     * @param nTick The song tick to rebuild from.
     * @ghidraAddress NTSC-U/C: 0x0014ce80
     * @ghidraAddress PAL: 0x0014e820
     */
    void Restart(Player *pPlayer, int nTick);

    /**
     * Report whether a phrase of the track is in a bar.
     *
     * @param nBar The bar.
     * @return Whether a phrase is in the bar.
     * @ghidraAddress NTSC-U/C: 0x0014cef8
     * @ghidraAddress PAL: 0x0014e898
     */
    bool HasPhraseAt(int nBar);

    /**
     * Find the phrase that is playing or next plays at a bar.
     *
     * The name is inferred.
     *
     * @param nBar The bar.
     * @param pStartTick Receives the first tick of the phrase.
     * @param pEndTick Receives the tick after the phrase.
     * @return True when a phrase was found.
     * @ghidraAddress NTSC-U/C: 0x0014cf20
     * @ghidraAddress PAL: 0x0014e8c0
     */
    bool FindPhrase(int nBar, int *pStartTick, int *pEndTick);

    /**
     * Report the tick of the first gem at or after a tick.
     *
     * @param nTick The tick.
     * @param pType Receives the type of the gem when not null.
     * @return The tick of the gem, or -1 when no gem follows.
     * @ghidraAddress NTSC-U/C: 0x0014cf40
     * @ghidraAddress PAL: 0x0014e8e0
     */
    int GetNextGemTick(int nTick, int *pType);

    /**
     * Capture the phrase of a bar for a player with the autocatcher power-up.
     *
     * @param nBar The bar.
     * @param pPlayer The player.
     * @return Whether a phrase was captured.
     * @ghidraAddress NTSC-U/C: 0x0014cfd0
     * @ghidraAddress PAL: 0x0014e970
     */
    bool Autocatch(int nBar, Player *pPlayer);

    /**
     * Give a player the freestyle power-up for a number of bars.
     *
     * @param nBar The first bar.
     * @param nBars The number of bars.
     * @param pPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x0014d250
     * @ghidraAddress PAL: 0x0014ebf0
     */
    void Freestyle(int nBar, int nBars, Player *pPlayer);
};
