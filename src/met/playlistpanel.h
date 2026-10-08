#pragma once

#include "met/focuschangepanel.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel `jbox_list` of the jukebox playlist, whose square button turns the selected entry on or
 * off.
 *
 * The RTTI records the class as deriving from FocusChangePanel. Its vtable is at `0x003d0228`. The
 * triangle and the left button give the focus back to `jbox`.
 */
class PlaylistPanel : public FocusChangePanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x001a8050
     * @ghidraAddress PAL: 0x001afd40
     */
    PlaylistPanel(DataArray *pData, const char *pszDir) : FocusChangePanel(pData, pszDir) {
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x003620c0
     * @ghidraAddress PAL: 0x003d05d8
     */
    ~PlaylistPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x00362168
     * @ghidraAddress PAL: 0x003d0680
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new PlaylistPanel(pData, pszDir);
    }

    /**
     * Route the controller to HandleJoypad().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a8128
     * @ghidraAddress PAL: 0x001afe18
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Take the focus and move the cursor of the list.
     *
     * @ghidraAddress NTSC-U/C: 0x001a8088
     * @ghidraAddress PAL: 0x001afd78
     */
    void Focus() override;

    /**
     * Lose the focus and dim the cursor of the list.
     *
     * @ghidraAddress NTSC-U/C: 0x001a80d8
     * @ghidraAddress PAL: 0x001afdc8
     */
    void Unfocus() override;

private:
    /**
     * Give the focus back to `jbox`, or turn the selected entry on or off with the square button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True for the cross button, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x001a8190
     * @ghidraAddress PAL: 0x001afe80
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};
