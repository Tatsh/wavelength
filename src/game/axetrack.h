#pragma once

#include <vector>

#include "game/axetrackdata.h"
#include "game/freestyletrack.h"
#include "game/playmap.h"
#include "game/sectionboundaries.h"
#include "gs/axecontour.h"
#include "gs/axeharmony.h"
#include "gs/axer.h"
#include "os/command.h"
#include "os/mem.h"
#include "os/ptr.h"

/**
 * Freestyle track played as a guitar.
 *
 * The RTTI records the class as deriving from FreestyleTrack. The object is 0x88 bytes, and
 * GameLogic allocates it under the tag "AxeTrack". Each gem button has an Axer that plays the notes
 * of the button over the chord of the song position. The stick selects the note and bends the
 * pitch while a button is held.
 */
class AxeTrack : public FreestyleTrack {
public:
    /** The number of gem buttons, each with an Axer. */
    static constexpr int kNumAxers = AxeTrackData::kSetSize;

    /**
     * Release a track to the pool heap GameLogic allocated it from.
     *
     * @param pBlock The block.
     */
    static void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Construct a guitar track.
     *
     * @param pData The notes and chords of the track.
     * @param pSections The sections of the song.
     * @param pPlayMap The map of the song positions.
     * @param pfMsPerTick The duration of one tick in milliseconds.
     * @param nIndex The index of the track in the song.
     * @param nIntroBars The bars before the first section, which the track ignores.
     * @param nNumBars The length of the song in bars.
     * @param nTicksPerBar The length of a bar in ticks.
     * @ghidraAddress NTSC-U/C: 0x00147438
     * @ghidraAddress PAL: 0x00148df8
     */
    AxeTrack(AxeTrackData *pData,
             SectionBoundaries *pSections,
             PlayMap *pPlayMap,
             const float *pfMsPerTick,
             int nIndex,
             int nIntroBars,
             int nNumBars,
             int nTicksPerBar);

    /**
     * Stop the track and release its players.
     *
     * @ghidraAddress NTSC-U/C: 0x00147688
     * @ghidraAddress PAL: 0x00149048
     */
    ~AxeTrack() override;

    /**
     * Give the players the chord and the notes of the song position, and keep them updated.
     *
     * @ghidraAddress NTSC-U/C: 0x00147768
     * @ghidraAddress PAL: 0x00149128
     */
    void Start() override;

    /**
     * Withdraw the scheduled commands and stop every player.
     *
     * @ghidraAddress NTSC-U/C: 0x00147798
     * @ghidraAddress PAL: 0x00149158
     */
    void Stop() override;

    /**
     * End the held note and give the track to the player it plays for.
     *
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x00147888
     * @ghidraAddress PAL: 0x00149248
     */
    void SetPlayer(Player *pPlayer) override;

    using FreestyleTrack::HandleInput;

    /**
     * Start the note of a pressed gem button, or end it when the button is released.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x001478c8
     * @ghidraAddress PAL: 0x00149288
     */
    void HandleInput(Player *pPlayer, const PlayNoteEvent &event) override;

    /**
     * Move the held note with the stick.
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

    /**
     * Report whether a gem button is held.
     *
     * @return True while a note sounds.
     * @ghidraAddress NTSC-U/C: 0x00347d98
     * @ghidraAddress PAL: 0x003b51c8
     */
    bool IsActive() override {
        return mActiveButton != kNoButton;
    }

    /**
     * Withdraw the scheduled commands and give the players the chord and the notes of the song
     * position again.
     *
     * @ghidraAddress NTSC-U/C: 0x00147dd8
     * @ghidraAddress PAL: 0x00149798
     */
    void Refresh() override;

    int mTicksPerBar;                 /*!< The length of a bar, in ticks. */
    int mNumBars;                     /*!< The length of the song, in bars. */
    AxeTrackData *mData;              /*!< The notes and chords to play. */
    std::vector<Axer *> mAxers;       /*!< The player of each gem button. */
    int mActiveButton;                /*!< The held gem button, or kNoButton. */
    float mX;                         /*!< The last horizontal stick position. */
    float mY;                         /*!< The last vertical stick position. */
    const AxeHarmony *mHarmony;       /*!< The chord the players have. */
    AxeContour *mContours[kNumAxers]; /*!< The notes each player has. */
    Ptr<Command> mHarmonyCommand;     /*!< Calls UpdateHarmony(). */
    Ptr<Command> mContourCommand;     /*!< Calls UpdateContours(). */

private:
    static constexpr int kNoButton = -1;

    /**
     * Start the note of a gem button at the song position, ending any held note.
     *
     * @param nButton The gem button.
     * @param fX The horizontal stick position.
     * @param fY The vertical stick position.
     * @ghidraAddress NTSC-U/C: 0x00147a18
     * @ghidraAddress PAL: 0x001493d8
     */
    void StartNote(int nButton, float fX, float fY);

    /**
     * End the held note, if any.
     *
     * @ghidraAddress NTSC-U/C: 0x00147b20
     * @ghidraAddress PAL: 0x001494e0
     */
    void EndNote();

    /**
     * Give the players the chord of the song position, and schedule the next change.
     *
     * @ghidraAddress NTSC-U/C: 0x00147ba8
     * @ghidraAddress PAL: 0x00149568
     */
    void UpdateHarmony();

    /**
     * Give the players the notes of the song position, and schedule the next change.
     *
     * @ghidraAddress NTSC-U/C: 0x00147ca8
     * @ghidraAddress PAL: 0x00149668
     */
    void UpdateContours();
};
