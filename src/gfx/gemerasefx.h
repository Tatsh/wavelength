#pragma once

#include <list>

#include "gfx/gfxtunnel.h"

/**
 * The bursts of the gems a player erases, each started at its time.
 *
 * The class is not polymorphic. The type of its list of gems records the name. The object is 0xc
 * bytes.
 */
class GemEraseFX {
public:
    /** One burst, 0x10 bytes. */
    class Gem {
    public:
        /**
         * Construct a burst.
         *
         * @param flTime The song time of the burst.
         * @param nTrack The track.
         * @param nPlayer The player whose burst views to use, or a negative value for the shared
         *                ones.
         * @param flTick The tick of the gem.
         * @param flLateral The position across the track, 0 to 1.
         * @ghidraAddress NTSC-U/C: 0x001f31f0
         * @ghidraAddress PAL: 0x001fbf90
         */
        Gem(float flTime, char nTrack, char nPlayer, float flTick, float flLateral);

        /**
         * Report whether a burst comes before a time, the order of the list.
         *
         * @param gem The burst.
         * @param flTime The time.
         * @return Whether the burst is earlier.
         */
        static bool Before(const Gem &gem, float flTime) {
            return gem.mTime < flTime;
        }

        float mTime;    /*!< The song time of the burst. */
        char mTrack;    /*!< The track. */
        char mPlayer;   /*!< The player whose burst views to use, or negative. */
        float mTick;    /*!< The tick of the gem. */
        float mLateral; /*!< The position across the track, 0 to 1. */
    };

    /**
     * Construct an empty list.
     *
     * @param pTunnel The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001f3210
     * @ghidraAddress PAL: 0x001fbfb0
     */
    explicit GemEraseFX(GfxTunnel *pTunnel);

    /**
     * Release the list.
     *
     * @ghidraAddress NTSC-U/C: 0x001f3288
     * @ghidraAddress PAL: 0x001fc028
     */
    ~GemEraseFX();

    /**
     * Schedule a burst.
     *
     * @param nTrack The track.
     * @param nPlayer The player whose burst views to use, or a negative value for the shared ones.
     * @param flTime The song time of the burst.
     * @param flTick The tick of the gem.
     * @param flLateral The position across the track, 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001f32d8
     * @ghidraAddress PAL: 0x001fc078
     */
    void Add(char nTrack, char nPlayer, float flTime, float flTick, float flLateral);

    /**
     * Cancel every burst.
     *
     * @ghidraAddress NTSC-U/C: 0x001f3400
     * @ghidraAddress PAL: 0x001fc1a0
     */
    void Clear();

    /**
     * Start the bursts whose time has come.
     *
     * @ghidraAddress NTSC-U/C: 0x001f3420
     * @ghidraAddress PAL: 0x001fc1c0
     */
    void Poll();

    std::list<Gem> mGems; /*!< The bursts, sorted by time. */
    GfxTunnel *mTunnel;   /*!< The tunnel. */
};
