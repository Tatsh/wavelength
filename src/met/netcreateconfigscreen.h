#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"

/**
 * Screen that asks whether to start the program that creates a network configuration.
 *
 * The RTTI records the class as deriving from FreqScreen.
 */
class NetCreateConfigScreen : public FreqScreen {
public:
    /**
     * Construct a screen.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x00177510
     * @ghidraAddress PAL: 0x0017ace8
     */
    explicit NetCreateConfigScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00358c08
     * @ghidraAddress PAL: 0x003c5f70
     */
    ~NetCreateConfigScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     * @ghidraAddress NTSC-U/C: 0x00358ca0
     * @ghidraAddress PAL: 0x003c6008
     */
    static UIScreen *New(DataArray *pData) {
        return new NetCreateConfigScreen(pData);
    }

    /**
     * Route a choice to HandleSelect().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00177548
     * @ghidraAddress PAL: 0x0017ad20
     */
    bool DispatchPriv(Message *pMsg) override;

private:
    /**
     * Start the program on `yes`, or go back to the configurations on `no`.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True for `yes` and `no`, otherwise the result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x001775b0
     * @ghidraAddress PAL: 0x0017ad88
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};
