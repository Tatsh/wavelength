#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"

/**
 * The screen that lists saved remixes to choose one.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003ce718`. The description's `grey_read_only` and `grey_unplayable` grey out the remixes
 * that cannot be changed and the ones that cannot be played. Only the members RemixCopyDelScreen
 * uses are declared, and the routines of the class are not reconstructed.
 */
class RemixSelectScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no remixes.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001898b0
     * @ghidraAddress PAL: 0x0018fe48
     */
    explicit RemixSelectScreen(DataArray *pData);

    /**
     * Handle a message sent to the screen.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00189de0
     * @ghidraAddress PAL: 0x00190378
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, and list the remixes.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00189938
     * @ghidraAddress PAL: 0x0018fed0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    int mReserved70[4];  // +0x70, the listed remixes, a vector of RemixInfo not yet recovered.
    int mGreyReadOnly;   /*!< Whether remixes that cannot be changed show greyed out. */
    int mGreyUnplayable; /*!< Whether remixes that cannot be played show greyed out. */
};
