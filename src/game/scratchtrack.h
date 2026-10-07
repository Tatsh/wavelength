#pragma once

#include "game/freestyletrack.h"
#include "game/scratchtrackdata.h"
#include "gs/muse.h"
#include "gs/scratcher.h"
#include "os/command.h"
#include "os/mem.h"
#include "os/ptr.h"

/**
 * Freestyle track the player scratches on.
 *
 * The RTTI records the class as deriving from FreestyleTrack. Each gem button the player holds
 * plays one scratcher of the set that plays at the song position. Releasing the scratch plays a
 * short sound on the next quantisation boundary.
 */
class ScratchTrack : public FreestyleTrack {
public:
    /** The number of scratchers in a set, one for each gem button. */
    static constexpr int kNumScratchers = ScratchTrackData::kSetSize;

    /**
     * Release a track to the pool heap GameLogic allocated it from.
     *
     * @param pBlock The block.
     */
    static void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Construct a track and its release sounds.
     *
     * @param pData The scratchers to play.
     * @param pSections The section boundaries of the song.
     * @param pPlayMap The map of the song positions.
     * @param pfMsPerTick The length of a tick, in milliseconds.
     * @param nIndex The track's index in the song, also the MIDI channel of the release sounds.
     * @param nIntroBars The intro length, in bars, which the track ignores.
     * @param nNumBars The length of the song, in bars.
     * @param nTicksPerBar The length of a bar, in ticks.
     * @ghidraAddress NTSC-U/C: 0x00137540
     * @ghidraAddress PAL: 0x00138da0
     */
    ScratchTrack(ScratchTrackData *pData,
                 const SectionBoundaries *pSections,
                 PlayMap *pPlayMap,
                 const float *pfMsPerTick,
                 int nIndex,
                 int nIntroBars,
                 int nNumBars,
                 int nTicksPerBar);

    /**
     * Stop the track and release it.
     *
     * @ghidraAddress NTSC-U/C: 0x00137780
     * @ghidraAddress PAL: 0x00138fe0
     */
    ~ScratchTrack() override;

    /**
     * Select the scratchers of the set at the song position.
     *
     * @ghidraAddress NTSC-U/C: 0x00137838
     * @ghidraAddress PAL: 0x00139098
     */
    void Start() override;

    /**
     * Withdraw the scheduled commands and stop every scratcher.
     *
     * @ghidraAddress NTSC-U/C: 0x00137858
     * @ghidraAddress PAL: 0x001390b8
     */
    void Stop() override;

    /**
     * End the active scratch, then assign the player.
     *
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x00137900
     * @ghidraAddress PAL: 0x00139160
     */
    void SetPlayer(Player *pPlayer) override;

    using FreestyleTrack::HandleInput;

    /**
     * Start or end a scratch of the track's player.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00137940
     * @ghidraAddress PAL: 0x001391a0
     */
    void HandleInput(Player *pPlayer, const PlayNoteEvent &event) override;

    /**
     * Move the active scratcher with a stick position of the track's player.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x001379f8
     * @ghidraAddress PAL: 0x00139258
     */
    void HandleInput(Player *pPlayer, const StickEvent<2> &event) override;

    /**
     * Release the scratch of the track's player and play the release sound.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00137ab0
     * @ghidraAddress PAL: 0x00139310
     */
    void HandleInput(Player *pPlayer, const BtnEvent<10> &event) override;

    /**
     * Report whether a scratch is active.
     *
     * @return True while a scratch is active.
     * @ghidraAddress NTSC-U/C: 0x0033feb8
     * @ghidraAddress PAL: 0x003ad3f0
     */
    bool IsActive() override {
        return mActiveButton != kNoButton;
    }

    /**
     * Withdraw the scheduled update and select the scratchers of the set at the song position.
     *
     * @ghidraAddress NTSC-U/C: 0x00138028
     * @ghidraAddress PAL: 0x00139888
     */
    void Refresh() override;

    int mTicksPerBar;                         /*!< The length of a bar, in ticks. */
    int mNumBars;                             /*!< The length of the song, in bars. */
    ScratchTrackData *mData;                  /*!< The scratchers to play. */
    Scratcher *mScratchers[kNumScratchers];   /*!< The scratchers of the current set. */
    int mActiveButton;                        /*!< The button that scratches, or kNoButton. */
    int mLastButton;                          /*!< The button that scratched last. */
    float mX;                                 /*!< The last horizontal stick position. */
    float mY;                                 /*!< The last vertical stick position. */
    Ptr<Command> mUpdateCommand;              /*!< Calls UpdateScratchers(). */
    Ptr<Command> mRetriggerCommand;           /*!< Calls Retrigger(). */
    Ptr<Muse> mReleaseSounds[kNumScratchers]; /*!< The release sound of each button. */

private:
    /** The value of mActiveButton while no scratch is active. */
    static constexpr int kNoButton = -1;

    /**
     * Restart the active scratch, if any, at the last stick position.
     *
     * @ghidraAddress NTSC-U/C: 0x00137b18
     * @ghidraAddress PAL: 0x00139378
     */
    void Retrigger();

    /**
     * Stop the active scratcher and schedule a retrigger.
     *
     * @ghidraAddress NTSC-U/C: 0x00137b48
     * @ghidraAddress PAL: 0x001393a8
     */
    void ReleaseScratch();

    /**
     * End any active scratch and start one.
     *
     * @param nButton The button, which selects the scratcher.
     * @param fX The horizontal stick position.
     * @param fY The vertical stick position.
     * @ghidraAddress NTSC-U/C: 0x00137bd8
     * @ghidraAddress PAL: 0x00139438
     */
    void StartScratch(int nButton, float fX, float fY);

    /**
     * Stop the active scratcher and record the end of the scratch.
     *
     * @ghidraAddress NTSC-U/C: 0x00137d70
     * @ghidraAddress PAL: 0x001395d0
     */
    void EndScratch();

    /**
     * Select the scratchers of the set at the song position, and schedule the next update for the
     * end of the set.
     *
     * @ghidraAddress NTSC-U/C: 0x00137e28
     * @ghidraAddress PAL: 0x00139688
     */
    void UpdateScratchers();

    /**
     * Play a release sound on the next sample quantisation boundary.
     *
     * @param nButton The button whose release sound plays.
     * @ghidraAddress NTSC-U/C: 0x00137fa8
     * @ghidraAddress PAL: 0x00139808
     */
    void PlayReleaseSound(int nButton);
};
