#pragma once

#include "met/netmainpanel.h"
#include "msg/joypadinputmsg.h"
#include "msg/lobbychatroomsmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uipanel.h"

/**
 * Panel of the online main screen that lists the chat rooms of the lobby.
 *
 * The RTTI records the class as deriving from NetMainPanel. Its vtable is at `0x003ccd70`. The
 * cross button on the cursor switches to the chosen room, and the square button opens the screen
 * that creates a room.
 */
class NetMainLobbiesPanel : public NetMainPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00172fb0
     * @ghidraAddress PAL: 0x001763f8
     */
    NetMainLobbiesPanel(DataArray *pData, const char *pszDir) : NetMainPanel(pData, pszDir) {
    }

    /**
     * Create a panel from its script description.
     *
     * Metagame::RegisterScreenClasses() registers the routine for the entry type
     * `fn_main_lobbies_panel`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x003581b8
     * @ghidraAddress PAL: 0x003c5568
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new NetMainLobbiesPanel(pData, pszDir);
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x003580b0
     */
    ~NetMainLobbiesPanel() override {
    }

    /**
     * Route the chat rooms, the choice, and the controller to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00173148
     * @ghidraAddress PAL: 0x00176590
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Take the focus, select the cursor of the list, and show the help of the focused panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00173020
     * @ghidraAddress PAL: 0x00176468
     */
    void Focus() override;

    /**
     * Lose the focus, dim the cursor of the list, and show the help of the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x001730b8
     * @ghidraAddress PAL: 0x00176500
     */
    void Unfocus() override;

    /**
     * Request the chat rooms of the lobby.
     *
     * @ghidraAddress NTSC-U/C: 0x00172fe8
     * @ghidraAddress PAL: 0x00176430
     */
    void RequestUpdate() override;

private:
    /**
     * Fill the list while the panel is loaded, and request the chat rooms again later.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001731f8
     * @ghidraAddress PAL: 0x00176640
     */
    bool HandleChatrooms(LobbyChatroomsMsg *pMsg);

    /**
     * Confirm the switch to the chosen room when the cross button chooses `cursor`.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001732b8
     * @ghidraAddress PAL: 0x00176700
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Open the screen that creates a room with the square button, then handle the button as
     * NetMainPanel does.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True during a transition, otherwise the result of NetMainPanel::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x00173348
     * @ghidraAddress PAL: 0x00176790
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};
