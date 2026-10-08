#pragma once

#include <list>

#include "game/avatarpartset.h"
#include "met/avatarpanel.h"
#include "met/netlpadpanel.h"
#include "msg/joypadinputmsg.h"
#include "netflow/netlaunchpadplayer.h"
#include "script/dataarray.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uipanel.h"

/**
 * Panel of a launchpad with a button for each player. The panel shows the chosen player's Freq,
 * games, connection, and colour, and lets the host remove a player.
 *
 * The RTTI records the class as deriving from AvatarPanel and from NetLPadPanel.
 */
class NetLPadPlayerPanel : public AvatarPanel, public NetLPadPanel {
public:
    /** The number of player buttons. */
    static constexpr int kNumPlayerButtons = 4;

    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00175290
     * @ghidraAddress PAL: 0x001786f0
     */
    NetLPadPlayerPanel(DataArray *pData, const char *pszDir) : AvatarPanel(pData, pszDir) {
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x003585a0
     * @ghidraAddress PAL: 0x003c5950
     */
    ~NetLPadPlayerPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x00358670
     * @ghidraAddress PAL: 0x003c5a20
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new NetLPadPlayerPanel(pData, pszDir);
    }

    /**
     * Route the controller and the focus messages.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00175df8
     * @ghidraAddress PAL: 0x00179258
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter with the first player chosen, and dim the panel.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001752d8
     * @ghidraAddress PAL: 0x00178738
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Take the focus, light the tab and the background, and show the help of the panel.
     *
     * The panel does nothing while it is not loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00175868
     * @ghidraAddress PAL: 0x00178cc8
     */
    void Focus() override;

    /**
     * Dim the tab, the background, and the focused button, and show the help of the launchpad.
     *
     * @ghidraAddress NTSC-U/C: 0x00175a78
     * @ghidraAddress PAL: 0x00178ed8
     */
    void Unfocus() override;

    /**
     * Show the players on the panel's rows and buttons, and give the focus back to the player who
     * had it, or to the first player.
     *
     * @param pPlayers The players.
     * @ghidraAddress NTSC-U/C: 0x00175360
     * @ghidraAddress PAL: 0x001787c0
     */
    void Update(std::list<NetLaunchpadPlayer> *pPlayers) override;

private:
    /**
     * Show the games, the connection, and the colour of a player.
     *
     * The name is inferred.
     *
     * @param pPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001756d8
     * @ghidraAddress PAL: 0x00178b38
     */
    void ShowPlayer(NetLaunchpadPlayer *pPlayer);

    /**
     * Show the help for the chosen player. The host may remove another player.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00175c98
     * @ghidraAddress PAL: 0x001790f8
     */
    void UpdateHelp();

    /**
     * Update the help when the focus moves while the panel has the focus.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00175e88
     * @ghidraAddress PAL: 0x001792e8
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Choose a player with up and down, return to the buttons with the triangle or left button,
     * or let the host remove the chosen player with the square button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True while a transition runs or for the cross button, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x00175ed0
     * @ghidraAddress PAL: 0x00179330
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mReserved104[3];        // +0x104, not yet identified.
    int mSelected;              /*!< The chosen player. */
    int mReserved114;           // +0x114, not yet identified.
    AvatarPartSet mReserved118; // +0x118, constructed but not yet seen in use.
};
