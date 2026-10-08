#pragma once

#include <vector>

#include "memcard/memcarduser.h"
#include "met/errorscreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that asks each memory card slot in turn whether a card is in it, then hands the slots
 * with a card to `o_memcard`.
 *
 * The RTTI records the class as deriving from ErrorScreen and from MemcardUser at `+0xa0`. Its
 * vtables are at `0x003d0b38` and, for MemcardUser, `0x003d0aa8`. The panel
 * `mem_cards_connected_dlg` shows the slot being asked.
 */
class CardsConnectedScreen : public ErrorScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001ad330
     * @ghidraAddress PAL: 0x001b5a60
     */
    explicit CardsConnectedScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00363cc0
     * @ghidraAddress PAL: 0x003d2420
     */
    ~CardsConnectedScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00363df8
     */
    static UIScreen *New(DataArray *pData) {
        return new CardsConnectedScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x00363e38
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the end of the entry to HandleTransitionComplete().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001ad6b0
     * @ghidraAddress PAL: 0x001b5de0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Forget the slots found and start from the first slot.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001ad388
     * @ghidraAddress PAL: 0x001b5ab8
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Move on to the next slot once the status of a slot has arrived.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001ad3c0
     * @ghidraAddress PAL: 0x001b5af0
     */
    void Poll(float fTime) override;

    /**
     * Record the slot when it has a card, and move on with the next poll.
     *
     * @param nStatus The outcome, one of MemcardTask::Status.
     * @ghidraAddress NTSC-U/C: 0x001ad540
     * @ghidraAddress PAL: 0x001b5c70
     */
    void OnCardStatus(int nStatus) override;

private:
    /**
     * Ask the next slot, or hand the slots with a card to `o_memcard` after the last slot.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001ad400
     * @ghidraAddress PAL: 0x001b5b30
     */
    void CheckNextSlot();

    /**
     * Ask the first slot once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001ad718
     * @ghidraAddress PAL: 0x001b5e48
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    std::vector<int> mSlots; // The slots with a card.
    int mAdvance;            // Whether the next poll moves on to the next slot.
    int mNumSlots;           // The number of slots to ask.
};
