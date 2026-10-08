#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * Screen that shows the news of the lobby server after a login.
 *
 * The RTTI records the class as deriving from FreqScreen. NetServerLogin sets the news.
 */
class NetWelcomeScreen : public FreqScreen {
public:
    /**
     * Construct a screen.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017be50
     * @ghidraAddress PAL: 0x0017faa8
     */
    explicit NetWelcomeScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x003599e0
     * @ghidraAddress PAL: 0x003c6d48
     */
    ~NetWelcomeScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     * @ghidraAddress NTSC-U/C: 0x00359a78
     * @ghidraAddress PAL: 0x003c6de0
     */
    static UIScreen *New(DataArray *pData) {
        return new NetWelcomeScreen(pData);
    }

    /**
     * Enter, show the news, and focus the first button.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x0017be88
     * @ghidraAddress PAL: 0x0017fae0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    const char *mNews; /*!< The news to show. */
};
