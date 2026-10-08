#pragma once

#include "met/netmainpanel.h"
#include "msg/lobbyplayersmsg.h"
#include "msg/playerrankmsg.h"
#include "rnd/mesh.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel of the online main screen that lists the players of the lobby and draws the Freq of the
 * selected player on the mesh `fn_main_play_freq.mesh`.
 *
 * The RTTI records the class as deriving from NetMainPanel. Its vtable is at `0x003ccf98`.
 */
class NetMainPlayersPanel : public NetMainPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x001721c0
     * @ghidraAddress PAL: 0x00175608
     */
    NetMainPlayersPanel(DataArray *pData, const char *pszDir)
        : NetMainPanel(pData, pszDir), mShowAvatar(0) {
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00357c50
     * @ghidraAddress PAL: 0x003c5000
     */
    ~NetMainPlayersPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new NetMainPlayersPanel(pData, pszDir);
    }

    /**
     * Route the players and the ranks of the lobby to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00172490
     * @ghidraAddress PAL: 0x001758d8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter and draw the Freq of the selected player.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00172290
     * @ghidraAddress PAL: 0x001756d8
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Stop drawing the Freq and exit.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001722b0
     * @ghidraAddress PAL: 0x001756f8
     */
    void Exit(bool bForce, float fTime) override;

    /**
     * Take the focus, move the cursor of the list, and show the help of the focused panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00172308
     * @ghidraAddress PAL: 0x00175750
     */
    void Focus() override;

    /**
     * Lose the focus, dim the cursor of the list, and show the help of the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x001723a0
     * @ghidraAddress PAL: 0x001757e8
     */
    void Unfocus() override;

    /**
     * Draw the panel and the Freq of the selected player.
     *
     * @ghidraAddress NTSC-U/C: 0x00172430
     * @ghidraAddress PAL: 0x00175878
     */
    void Draw() override;

    /**
     * Finish the load and hide the mesh of the Freq.
     *
     * @ghidraAddress NTSC-U/C: 0x001721f8
     * @ghidraAddress PAL: 0x00175640
     */
    void FinishLoad() override;

    /**
     * Request the players of the lobby.
     *
     * @ghidraAddress NTSC-U/C: 0x001722d0
     * @ghidraAddress PAL: 0x00175718
     */
    void RequestUpdate() override;

private:
    /**
     * Fill the list while the panel is loaded, and request the players again later.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00172520
     * @ghidraAddress PAL: 0x00175968
     */
    bool HandleLobbyPlayers(LobbyPlayersMsg *pMsg);

    /**
     * Record the rank of a player in the list while the panel is loaded.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001725e8
     * @ghidraAddress PAL: 0x00175a30
     */
    bool HandlePlayerRank(PlayerRankMsg *pMsg);

    Rnd::Mesh *mAvatarMesh; // The mesh `fn_main_play_freq.mesh` the Freq is drawn on.
    int mShowAvatar;        // Whether Draw() draws the Freq, from Enter() to Exit().
};
