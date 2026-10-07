#pragma once

#include "met/netlpadpanel.h"
#include "met/songpicpanel.h"

/**
 * Panel of the launchpad screens that shows the song and the settings of the online game.
 *
 * The RTTI records the class as deriving from SongPicPanel and NetLPadPanel. Only the routine
 * NetLpadScreen calls is declared, and the routines of the class are not reconstructed.
 */
class NetLPadGamePanel : public SongPicPanel, public NetLPadPanel {
public:
    /**
     * Show the song and the settings of the game database.
     *
     * @ghidraAddress NTSC-U/C: 0x00174e28
     * @ghidraAddress PAL: 0x00178288
     */
    void RefreshGame();
};
