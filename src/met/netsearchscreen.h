#pragma once

#include "met/netparamsscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The screen where a player chooses the mode, the skill, and the song of the online games to
 * search for.
 *
 * The RTTI records the class as deriving from NetParamsScreen, and its vtable is at `0x003cdbc8`.
 * The metagame registers the class for the screen type `net_search_screen`. Every list ends with
 * the choice of any entry. The cross button on `search` shows the matching games on `fn_sorted`.
 */
class NetSearchScreen : public NetParamsScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001841d8
     * @ghidraAddress PAL: 0x00188418
     */
    explicit NetSearchScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `net_search_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035b7c0
     * @ghidraAddress PAL: 0x003c9318
     */
    static UIScreen *New(DataArray *pData) {
        return new NetSearchScreen(pData);
    }

    /**
     * Route a chosen button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00186338
     * @ghidraAddress PAL: 0x0018a578
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Fill the list of modes, choose any mode and any skill, and start the entry.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00184210
     * @ghidraAddress PAL: 0x00188450
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Fill the list of skills of the chosen mode, and move the chosen skill along with the list.
     *
     * @ghidraAddress NTSC-U/C: 0x00184ad0
     * @ghidraAddress PAL: 0x00188d10
     */
    void OnModeChanged() override;

    /**
     * Fill the song list with the choice of every song and the songs of the chosen mode and
     * skill, and select the song chosen before.
     *
     * @param nReset Not read.
     * @ghidraAddress NTSC-U/C: 0x00185cb8
     * @ghidraAddress PAL: 0x00189ef8
     */
    void OnChoiceChanged(int nReset) override;

    /**
     * Give the search to `fn_sorted` and go there when `search` is chosen with the cross button.
     * Any other button chosen with the cross button moves the focus back to `search`.
     *
     * @param pMsg The message.
     * @return The result of NetParamsScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x001863a0
     * @ghidraAddress PAL: 0x0018a5e0
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};
