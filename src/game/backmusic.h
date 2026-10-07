#pragma once

#include "game/playmap.h"

/**
 * Background music of a song, played alone or along with a catch track.
 *
 * The RTTI includes the class name. Only the members GameLogic uses are declared.
 */
class BackMusic {
public:
    /**
     * Start the music by itself.
     *
     * @param pPlayMap The map of the song positions.
     * @ghidraAddress NTSC-U/C: 0x001497b0
     * @ghidraAddress PAL: 0x0014b170
     */
    void Start(PlayMap *pPlayMap);

    /**
     * Stop the music.
     *
     * @ghidraAddress NTSC-U/C: 0x001497d8
     * @ghidraAddress PAL: 0x0014b198
     */
    void Stop();
};
