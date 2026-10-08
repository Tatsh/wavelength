#pragma once

#include "met/setupnamescreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * Screen where the player chooses the name of a new chatroom of the lobby.
 *
 * The RTTI records the class as deriving from SetupNameScreen.
 */
class SetupNewChatroomScreen : public SetupNameScreen {
public:
    /**
     * Construct a screen with no text.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0035a8a0
     * @ghidraAddress PAL: 0x003c83f0
     */
    explicit SetupNewChatroomScreen(DataArray *pData) : SetupNameScreen(pData) {
    }

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035a740
     * @ghidraAddress PAL: 0x003c8290
     */
    ~SetupNewChatroomScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     * @ghidraAddress NTSC-U/C: 0x0035a860
     * @ghidraAddress PAL: 0x003c83b0
     */
    static UIScreen *New(DataArray *pData) {
        return new SetupNewChatroomScreen(pData);
    }

    /**
     * Enter and widen the `name` entry.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x0017d4c8
     * @ghidraAddress PAL: 0x00181118
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Pass the name to NetNewChatroomScreen and change to it.
     *
     * @ghidraAddress NTSC-U/C: 0x0017d558
     */
    void Submit() override;
};
