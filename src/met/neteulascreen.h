#pragma once

#include <vector>

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * Screen that shows the licence agreement of the lobby server, one page of 15 lines at a time.
 *
 * The RTTI records the class as deriving from FreqScreen. NetServerLogin sets the agreement. The up
 * and down directional buttons turn the pages.
 */
class NetEULAScreen : public FreqScreen {
public:
    /**
     * Construct a screen with no agreement.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017bf60
     * @ghidraAddress PAL: 0x0017fbb8
     */
    explicit NetEULAScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00359ab8
     * @ghidraAddress PAL: 0x003c6e20
     */
    ~NetEULAScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     */
    static UIScreen *New(DataArray *pData) {
        return new NetEULAScreen(pData);
    }

    /**
     * Route a controller message to HandleJoypad().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017c578
     * @ghidraAddress PAL: 0x001801d0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, break the agreement into pages, and show the first page.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x0017c0f8
     * @ghidraAddress PAL: 0x0017fd50
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Set the agreement.
     *
     * The name is inferred.
     *
     * @param pszText The agreement.
     * @ghidraAddress NTSC-U/C: 0x0017bfd8
     * @ghidraAddress PAL: 0x0017fc30
     */
    void SetText(const char *pszText);

private:
    /**
     * Turn a number of pages, stopping at the first and the last page, and show the page.
     *
     * The name is inferred.
     *
     * @param nDelta The pages to turn, negative to turn back.
     * @ghidraAddress NTSC-U/C: 0x0017bfe0
     * @ghidraAddress PAL: 0x0017fc38
     */
    void TurnPage(int nDelta);

    /**
     * Turn the pages with the up and down directional buttons.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleJoypad(), or true while a transition runs.
     * @ghidraAddress NTSC-U/C: 0x0017c5e0
     * @ghidraAddress PAL: 0x00180238
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    const char *mText;          /*!< The agreement. */
    String mWrappedText;        /*!< The agreement broken into lines. */
    int mPage;                  /*!< The page that shows. */
    std::vector<char *> mPages; /*!< The start of each page in mWrappedText, then its end. */
};
