#pragma once

#include <cstddef>
#include <vector>

#include "game/playmap.h"
#include "gs/muse.h"
#include "os/command.h"
#include "os/mem.h"
#include "os/ptr.h"
#include "os/string.h"

/**
 * Background music of a song, played alone or along with a catch track.
 *
 * The class is not polymorphic. The RTTI records the command it schedules, and the allocation tag
 * includes the class name. The music is one muse for each bar of the song, intro bars included,
 * and each bar the song plays starts the muse of the bar it maps to.
 */
class BackMusic {
public:
    /**
     * Allocate music, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    static void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "BackMusic", 0);
    }

    /**
     * Release music.
     *
     * @param pBlock The block.
     */
    static void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Construct music with no muse for any bar.
     *
     * @param pszName The name of the track.
     * @param nIntroBars The bars before bar 0.
     * @param nNumBars The length of the song in bars.
     * @param nTicksPerBar The song ticks in one bar.
     * @ghidraAddress NTSC-U/C: 0x00149548
     * @ghidraAddress PAL: 0x0014af08
     */
    BackMusic(const char *pszName, int nIntroBars, int nNumBars, int nTicksPerBar);

    /**
     * Stop the music and release it.
     *
     * @ghidraAddress NTSC-U/C: 0x00149680
     * @ghidraAddress PAL: 0x0014b040
     */
    ~BackMusic();

    /**
     * Report the name of the track.
     *
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x00149750
     * @ghidraAddress PAL: 0x0014b110
     */
    const char *GetName() const;

    /**
     * Set the muse of a bar.
     *
     * @param nBar The bar, negative for an intro bar.
     * @param pMuse The muse.
     * @ghidraAddress NTSC-U/C: 0x00149758
     * @ghidraAddress PAL: 0x0014b118
     */
    void SetBar(int nBar, Muse *pMuse);

    /**
     * Start the music by itself, unless it plays.
     *
     * @param pPlayMap The map of the song positions.
     * @ghidraAddress NTSC-U/C: 0x001497b0
     * @ghidraAddress PAL: 0x0014b170
     */
    void Start(PlayMap *pPlayMap);

    /**
     * Stop the music, if it plays.
     *
     * @ghidraAddress NTSC-U/C: 0x001497d8
     * @ghidraAddress PAL: 0x0014b198
     */
    void Stop();

private:
    /**
     * Play the muse of the bar the song plays from the current tick, and schedule the next bar.
     *
     * @ghidraAddress NTSC-U/C: 0x00149840
     * @ghidraAddress PAL: 0x0014b200
     */
    void PlayBar();

    String mName;                  /*!< The name of the track. */
    std::vector<Ptr<Muse> > mBars; /*!< The muse of each bar, intro bars first. */
    Ptr<Command> mBarCommand;      /*!< The command that calls PlayBar(). */
    Muse *mPlaying;                /*!< The muse that plays, or null when stopped. */
    int mIntroBars;                /*!< The bars before bar 0. */
    int mNumBars;                  /*!< The length of the song in bars. */
    int mTicksPerBar;              /*!< The song ticks in one bar. */
    PlayMap *mPlayMap;             /*!< The map of the song positions. */
};
