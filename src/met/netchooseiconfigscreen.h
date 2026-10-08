#pragma once

#include <list>

#include "met/freqscreen.h"
#include "netflow/inetconfig.h"
#include "script/dataarray.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"

/**
 * Screen that lists the network configurations of the memory card, with a button for each and a
 * button that creates one.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x7c bytes and its vtable
 * is at `0x003cd7e8`.
 */
class NetChooseIConfigScreen : public FreqScreen {
public:
    /**
     * Construct a screen with no configurations.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x00176d88
     * @ghidraAddress PAL: 0x0017a1e8
     */
    explicit NetChooseIConfigScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00358b08
     * @ghidraAddress PAL: 0x003c5e70
     */
    ~NetChooseIConfigScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     */
    static UIScreen *New(DataArray *pData) {
        return new NetChooseIConfigScreen(pData);
    }

    /**
     * Replace the configurations and choose the first.
     *
     * @param configs The configurations.
     * @ghidraAddress NTSC-U/C: 0x00176e10
     * @ghidraAddress PAL: 0x0017a270
     */
    void SetConfigs(const std::list<InetConfig> &configs);

    /**
     * Label a button for each of the first seven configurations, hide the other buttons, and move
     * the view to fit the buttons shown.
     *
     * The focus goes to the first configuration, or to `create` when there is none.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00176e40
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Route a choice and a change of focus to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001772c8
     * @ghidraAddress PAL: 0x0017aaa0
     */
    bool DispatchPriv(Message *pMsg) override;

private:
    /**
     * Show or hide the description of the chosen configuration.
     *
     * @param bShow Whether to show the description.
     * @ghidraAddress NTSC-U/C: 0x001771c0
     * @ghidraAddress PAL: 0x0017a998
     */
    void ShowDescription(bool bShow);

    /**
     * Open the screen that creates a configuration on `create`, or connect with the chosen
     * configuration.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00177358
     * @ghidraAddress PAL: 0x0017ab30
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Choose the configuration of the button with the focus and show its description.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00177428
     * @ghidraAddress PAL: 0x0017ac00
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    std::list<InetConfig> mConfigs; // The configurations of the memory card.
    int mSelected;                  // The index of the chosen configuration.
};
