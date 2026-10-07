#pragma once

#include <list>

#include "met/netlpadpanel.h"
#include "met/songpicpanel.h"
#include "netflow/netlaunchpadplayer.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel of a launchpad that shows the song, the mode, and the power-ups of the game, and up to
 * four players.
 *
 * The RTTI records the class as deriving from SongPicPanel and from NetLPadPanel.
 */
class NetLPadGamePanel : public SongPicPanel, public NetLPadPanel {
public:
    /** The number of player rows. */
    static constexpr int kNumPlayerRows = 4;

    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00174db8
     * @ghidraAddress PAL: 0x00178218
     */
    NetLPadGamePanel(DataArray *pData, const char *pszDir) : SongPicPanel(pData, pszDir) {
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x003586c0
     * @ghidraAddress PAL: 0x003c5a70
     */
    ~NetLPadGamePanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new NetLPadGamePanel(pData, pszDir);
    }

    /**
     * Enter and show the game.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00174df8
     * @ghidraAddress PAL: 0x00178258
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Show the players of the launchpad on the panel's rows.
     *
     * @param pPlayers The players.
     * @ghidraAddress NTSC-U/C: 0x00175270
     * @ghidraAddress PAL: 0x001786d0
     */
    void Update(std::list<NetLaunchpadPlayer> *pPlayers) override;

    /**
     * Show up to four players on the rows of a panel, and hide the remaining rows.
     *
     * Nothing is shown while no launchpad exists. The name is inferred.
     *
     * @param pszPanel The name of the panel.
     * @param pPlayers The players.
     * @ghidraAddress NTSC-U/C: 0x00174828
     * @ghidraAddress PAL: 0x00177c88
     */
    static void ShowPlayers(const char *pszPanel, std::list<NetLaunchpadPlayer> *pPlayers);

    /**
     * Show the band picture, the genre, the tempo, the mode, the remix, and the power-ups of the
     * game.
     *
     * @ghidraAddress NTSC-U/C: 0x00174e28
     * @ghidraAddress PAL: 0x00178288
     */
    void RefreshGame();
};
