#pragma once

#include "met/savefreqscreen.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that saves the Freq the Freq maker made, under the name the maker chose.
 *
 * The RTTI records the class as deriving from SaveFreqScreen. The object is 0xf0 bytes and its
 * vtables are at `0x003d1060` and, for MemcardUser, `0x003d0fd0`. The metagame registers the class
 * for the screen type `save_edited_freq_screen`, and the front-end description's
 * `fmaker_save_freq` and `fmaker_save_freq_net` screens are two. FreqMakerMainScreen sets the
 * name and whether the save may replace a saved Freq before the screen opens.
 */
class SaveEditedFreqScreen : public SaveFreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00363740
     * @ghidraAddress PAL: 0x003d1e90
     */
    explicit SaveEditedFreqScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00363658
     * @ghidraAddress PAL: 0x003d1d98
     */
    ~SaveEditedFreqScreen() override;

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `save_edited_freq_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x003636f8
     * @ghidraAddress PAL: 0x003d1e48
     */
    static UIScreen *New(DataArray *pData) {
        return new SaveEditedFreqScreen(pData);
    }

    /**
     * Route the end of the entry, and pass every other message to SaveFreqScreen.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001ac380
     * @ghidraAddress PAL: 0x001b49e0
     */
    bool DispatchPriv(Message *pMsg) override;

#ifdef VIDEO_STANDARD_PAL
    /**
     * Offer the save, cancel, and continue dialog once when the memory card changed during a save
     * that replaces the Freq, and handle every other outcome as SaveFreqScreen does.
     *
     * @param nStatus How the save ended, as SaveFreqScreen::OnFreqSaved() receives it.
     * @param nSpace The space the save needs, when the memory card is too full.
     * @ghidraAddress PAL: 0x001b48b8
     */
    void OnFreqSaved(int nStatus, int nSpace) override;
#endif

    /**
     * Start the save under mFreqName once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001ac3e8
     * @ghidraAddress PAL: 0x001b4a48
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    String mFreqName; /*!< The name the Freq is saved under. +0xd0 */
};
