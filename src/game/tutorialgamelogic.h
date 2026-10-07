#pragma once

#include "game/gamelogic.h"
#include "game/song.h"
#include "script/dataarray.h"

/**
 * Rules of the tutorial song.
 *
 * The RTTI records the class as deriving from GameLogic and from TaskDoneNotifier, the second base
 * at `+0x4bc`. The second base is not yet declared. Only the members Game uses and the overrides of
 * the pure members of GameLogic are declared. Of those, OnFinish(), OnSection(), and
 * OnPhraseMissed() have empty bodies.
 */
class TutorialGameLogic : public GameLogic {
public:
    /**
     * Construct the logic of the tutorial song with a random seed.
     *
     * @param pSong The song.
     * @param pSongConfig The entry of the song in the "songs" section.
     * @ghidraAddress NTSC-U/C: 0x0013f7e8
     */
    TutorialGameLogic(Song *pSong, DataArray *pSongConfig);

    /**
     * Release the logic.
     *
     * @ghidraAddress NTSC-U/C: 0x0013fcb0
     * @ghidraAddress PAL: 0x00141660
     */
    ~TutorialGameLogic() override;

    /**
     * Report the song position in ticks.
     *
     * @return The position.
     * @ghidraAddress NTSC-U/C: 0x00140230
     * @ghidraAddress PAL: 0x00141bf0
     */
    int GetTick() override;

    /**
     * Report the song position on the song clock.
     *
     * @return The position.
     * @ghidraAddress NTSC-U/C: 0x00140250
     * @ghidraAddress PAL: 0x00141c10
     */
    float GetTime() override;

    /**
     * Act once Start() finished.
     *
     * @ghidraAddress NTSC-U/C: 0x001402f8
     * @ghidraAddress PAL: 0x00141cb8
     */
    void OnStart() override;

    /**
     * Ignore the end of the song. The body is empty.
     *
     * @param bWon Whether the song was won.
     * @ghidraAddress NTSC-U/C: 0x00343af8
     */
    void OnFinish([[maybe_unused]] bool bWon) override {
    }

    /**
     * Act at the start of every bar.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x00140328
     */
    void OnBar(int nBar) override;

    /**
     * Ignore the start of a section. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00343ae8
     */
    void OnSection() override {
    }

    /**
     * Act once CapturePhrase() found the next phrases.
     *
     * @param pTrack The track the phrase was captured on.
     * @param pPlayer The player that captured it.
     * @param nBar The first bar the capture clears.
     * @param bStreak Whether the capture continues the streak of the player.
     * @ghidraAddress NTSC-U/C: 0x00140338
     * @ghidraAddress PAL: 0x00141cf8
     */
    void OnPhraseCaptured(CatchTrack *pTrack, Player *pPlayer, int nBar, bool bStreak) override;

    /**
     * Act once a phrase of a track ended.
     *
     * @param pTrack The track.
     * @ghidraAddress NTSC-U/C: 0x001403d8
     */
    void OnPhraseEnded(Track *pTrack) override;

    /**
     * Ignore a missed phrase. The body is empty.
     *
     * @param pTrack The track the phrase was missed on.
     * @ghidraAddress NTSC-U/C: 0x00343af0
     */
    void OnPhraseMissed([[maybe_unused]] Track *pTrack) override {
    }
};
