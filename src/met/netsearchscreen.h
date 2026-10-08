#pragma once

#include "met/netparamsscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"

/**
 * Screen where the player chooses the mode, the skill, and the song of the online games to search
 * for, each of which may be any.
 *
 * The RTTI records the class as deriving from NetParamsScreen, and the vtable is at `0x003cdbc8`.
 * The class adds no members. The destructor at `0x0035b6f0` (PAL `0x003c9248`) is
 * compiler-generated. The search opens `fn_sorted`.
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
    explicit NetSearchScreen(DataArray *pData) : NetParamsScreen(pData) {
    }

    /**
     * Create a screen from its script description.
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
     * Route a choice to HandleSelect(), or any other message to NetParamsScreen.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00186338
     * @ghidraAddress PAL: 0x0018a578
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * List the modes with `param_any` last, and choose any mode and any skill.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x00184210
     * @ghidraAddress PAL: 0x00188450
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * List the skills of the chosen mode with `param_any` last, keeping the chosen skill where the
     * lists differ.
     *
     * @ghidraAddress NTSC-U/C: 0x00184ad0
     * @ghidraAddress PAL: 0x00188d10
     */
    void OnModeChanged() override;

    /**
     * List `search_all` and the songs of the chosen mode and skill, keeping the song already
     * selected.
     *
     * The entry of that song counts the entries before the songs as the previous listing left
     * them.
     *
     * @param bReset Not read.
     * @ghidraAddress NTSC-U/C: 0x00185cb8
     * @ghidraAddress PAL: 0x00189ef8
     */
    void OnChoiceChanged(int bReset) override;

    /**
     * Search with the choices from `search`, or move the focus to `search` from any other
     * component.
     *
     * @param pMsg The message.
     * @return The result of NetParamsScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x001863a0
     * @ghidraAddress PAL: 0x0018a5e0
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};
