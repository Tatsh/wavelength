#pragma once

#include "met/freqpanel.h"
#include "msg/lobbyplayersmsg.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel that lists the ten players of the top of the ranking.
 *
 * The RTTI records the class as deriving from FreqPanel.
 */
class NetRankedPlayersPanel : public FreqPanel {
public:
    /** The number of rows the panel lists. */
    static constexpr int kNumRows = 10;

    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00172638
     * @ghidraAddress PAL: 0x00175a80
     */
    NetRankedPlayersPanel(DataArray *pData, const char *pszDir) : FreqPanel(pData, pszDir) {
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00357da8
     * @ghidraAddress PAL: 0x003c5158
     */
    ~NetRankedPlayersPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x00357e40
     * @ghidraAddress PAL: 0x003c51f0
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new NetRankedPlayersPanel(pData, pszDir);
    }

    /**
     * Route the list of players to HandleLobbyPlayers().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00172800
     * @ghidraAddress PAL: 0x00175c48
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter with empty rows and request the top of the ranking.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00172670
     * @ghidraAddress PAL: 0x00175ab8
     */
    void Enter(bool bForce, float fTime) override;

private:
    /**
     * Fill the rows with the ranks and the names of the players, while the panel is loaded.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00172868
     * @ghidraAddress PAL: 0x00175cb0
     */
    bool HandleLobbyPlayers(LobbyPlayersMsg *pMsg);
};
