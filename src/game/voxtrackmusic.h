#pragma once

#include <vector>

#include "game/catchtrackdata.h"
#include "game/playmap.h"
#include "gs/multimuse.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Player of the gems of a vocal track, one bar of the song at a time.
 *
 * The RTTI records the command the class schedules. The class is not polymorphic. The member names
 * are inferred. Each bar of the song has a MultiMuse with the music of the gems of the bar, and
 * each bar the song plays starts the MultiMuse of the bar of the song it maps to.
 */
class VoxTrackMusic {
public:
    /**
     * Construct the player and collect the music of the gems bar by bar.
     *
     * @param pGems The gems.
     * @param pPlayMap The map of the song positions.
     * @param nTicksPerBar The song ticks in one bar.
     * @param nNumBars The length of the song in bars.
     * @ghidraAddress NTSC-U/C: 0x00143ab8
     * @ghidraAddress PAL: 0x00145448
     */
    VoxTrackMusic(CatchTrackData *pGems, PlayMap *pPlayMap, int nTicksPerBar, int nNumBars);

    /**
     * Withdraw the scheduled command and release the player.
     *
     * The MultiMuse objects of the bars are not released.
     *
     * @ghidraAddress NTSC-U/C: 0x00143bd8
     * @ghidraAddress PAL: 0x00145568
     */
    ~VoxTrackMusic();

    /**
     * Schedule the first bar at tick 0.
     *
     * @ghidraAddress NTSC-U/C: 0x00143c50
     * @ghidraAddress PAL: 0x001455e0
     */
    void Start();

    /**
     * Withdraw the scheduled command.
     *
     * @ghidraAddress NTSC-U/C: 0x00143c90
     * @ghidraAddress PAL: 0x00145620
     */
    void Stop();

    /**
     * Silence the music, or play the current bar again from the current tick.
     *
     * @param bMuted Whether the music is silent.
     * @ghidraAddress NTSC-U/C: 0x00143cc8
     * @ghidraAddress PAL: 0x00145658
     */
    void SetMuted(bool bMuted);

private:
    /**
     * Start the music of the bar the song reaches, and schedule the next bar.
     *
     * @ghidraAddress NTSC-U/C: 0x00143dd0
     * @ghidraAddress PAL: 0x00145760
     */
    void StartBar();

    /**
     * Fill the MultiMuse of each bar with the music of the gems in the bar.
     *
     * @param pGems The gems.
     * @param nNumBars The length of the song in bars.
     * @ghidraAddress NTSC-U/C: 0x00143ee0
     * @ghidraAddress PAL: 0x00145870
     */
    void BuildBars(CatchTrackData *pGems, int nNumBars);

    int mTicksPerBar;               /*!< The song ticks in one bar. */
    PlayMap *mPlayMap;              /*!< The map of the song positions. */
    Ptr<Command> mBarCommand;       /*!< The command that calls StartBar(). */
    std::vector<MultiMuse *> mBars; /*!< The music of each bar of the song. */
    int mBar;                       /*!< The bar of the song that plays, or -1. */
    int mLastBar;                   /*!< The bar of the song that played before mBar. */
    int mMuted;                     /*!< Nonzero while the music is silent. */
};
