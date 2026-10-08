#pragma once

#include "os/asyncstream.h"
#include "rnd/rndloader.h"
#include "rnd/transanim.h"
#include "script/dataarray.h"

/**
 * Arena the play field flies through, with its camera path, its movie, and its beat effects.
 *
 * The RTTI names the class. The object is 0xa0 bytes with the vptr at +0x9c. Only the members the
 * display reads are declared.
 */
class GfxArena {
public:
    /**
     * Create the arena of a name, through the first registered creator that accepts the name or
     * as a plain arena.
     *
     * @param pszName The arena.
     * @return The arena.
     * @ghidraAddress NTSC-U/C: 0x001ea438
     * @ghidraAddress PAL: 0x001f31d8
     */
    static GfxArena *Create(const char *pszName);

    /**
     * Destroy the arena.
     *
     * @ghidraAddress NTSC-U/C: 0x001ebe30
     * @ghidraAddress PAL: 0x001f4bd0
     */
    virtual ~GfxArena();

    /**
     * Read the configuration of the arena.
     *
     * @ghidraAddress NTSC-U/C: 0x001ebf90
     * @ghidraAddress PAL: 0x001f4d30
     */
    virtual void LoadConfig();

    /**
     * Fit the camera path to the length of the song.
     *
     * @param fSongTicks The length of the song in ticks.
     * @ghidraAddress NTSC-U/C: 0x001ec710
     * @ghidraAddress PAL: 0x001f54b0
     */
    virtual void SetSongTicks(float fSongTicks);

    /**
     * Move the camera along its path and run the arena effects.
     *
     * @param fTick The tick of the song.
     * @param fTime The time of the display.
     * @ghidraAddress NTSC-U/C: 0x001eca40
     * @ghidraAddress PAL: 0x001f57e0
     */
    virtual void Poll(float fTick, float fTime);

    /**
     * Start to stream the movie of the arena, or mark the arena as having none.
     *
     * @ghidraAddress NTSC-U/C: 0x001ec058
     * @ghidraAddress PAL: 0x001f4df8
     */
    void LoadMovie();

    /**
     * Finish the load of the arena once its file is read.
     *
     * @param bHudLoaded Whether the head-up display has started to load.
     * @ghidraAddress NTSC-U/C: 0x001ec0d0
     * @ghidraAddress PAL: 0x001f4e70
     */
    void PollLoad(bool bHudLoaded);

    /**
     * Return the panels to their resting look.
     *
     * @ghidraAddress NTSC-U/C: 0x001ec910
     * @ghidraAddress PAL: 0x001f56b0
     */
    void ResetPanels();

    /**
     * Show the energy of the play field on the panels.
     *
     * @param nZone The zone of the energy. The arena does not read it.
     * @param fEnergy The energy, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001ec930
     * @ghidraAddress PAL: 0x001f56d0
     */
    void SetEnergy(int nZone, float fEnergy);

    /**
     * Show the result of the song on the panels.
     *
     * @param bWon Whether the song was won.
     * @ghidraAddress NTSC-U/C: 0x001ec980
     * @ghidraAddress PAL: 0x001f5720
     */
    void ShowResult(bool bWon);

    /**
     * Run the effect of a beat event of the song.
     *
     * @param nEvent The event, `A` or `B`.
     * @ghidraAddress NTSC-U/C: 0x001ec9b0
     * @ghidraAddress PAL: 0x001f5750
     */
    void HandleBeat(signed char nEvent);

    // +0x00 to +0x17 are not yet identified.
    int mNoMovie;              /*!< Whether the arena has no movie. +0x18 */
    DataArray *mConfig;        /*!< The configuration of the arena. +0x1c */
    AsyncStream *mMovieStream; /*!< The stream of the movie, or null. +0x20 */
    RndLoader *mLoader;        /*!< The loader of the file of the arena, or null. +0x24 */
    // +0x28 to +0x2f are not yet identified.
    Rnd::TransAnim *mCamPath; /*!< The path of the camera. +0x30 */
    // +0x34 to +0x43 are not yet identified.
    float mSongTicks;     /*!< The length of the song SetSongTicks() fitted. +0x44 */
    float mCamPathLength; /*!< The length of the camera path the data expects. +0x48 */
    // +0x4c is not yet identified.
    int mUnlockIndex; /*!< The arena's index in the vector of unlocked arenas. +0x50 */
    // +0x54 to +0x9b are not yet identified.
};
