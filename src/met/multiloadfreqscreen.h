#pragma once

#include <vector>

#include "game/campaign.h"
#include "memcard/memcarduser.h"
#include "met/errorscreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that loads the Freqs of the memory card of each player of a local game, adds the prefab
 * Freqs of the metagame configuration, and hands them all to the Freq selection panels.
 *
 * The RTTI records the class as deriving from ErrorScreen and from MemcardUser at `+0xa0`. Its
 * vtables are at `0x003d1270` and, for MemcardUser, `0x003d11e0`. The panel
 * `multi_load_freq_dlg` shows the slot being read. The first player starts on the Freq of the
 * player's name, and the other players on a Freq of their card or on a random prefab.
 */
class MultiLoadFreqScreen : public ErrorScreen, public MemcardUser {
public:
    /** The number of players whose Freqs the screen loads. */
    static constexpr int kMaxPlayers = 4;

    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001ab338
     * @ghidraAddress PAL: 0x001b3370
     */
    explicit MultiLoadFreqScreen(DataArray *pData) : ErrorScreen(pData), mLoaded(0) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x003632f8
     * @ghidraAddress PAL: 0x003d18f0
     */
    ~MultiLoadFreqScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00363480
     * @ghidraAddress PAL: 0x003d1a78
     */
    static UIScreen *New(DataArray *pData) {
        return new MultiLoadFreqScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x003634c0
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the end of the entry to HandleTransitionComplete().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001abd58
     * @ghidraAddress PAL: 0x001b3ee0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, returning to `m_mode` in a duel and to `m_player` otherwise.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001ab388
     * @ghidraAddress PAL: 0x001b33c0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Move on to the next slot once a load has ended.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001ab9c0
     * @ghidraAddress PAL: 0x001b39f8
     */
    void Poll(float fTime) override;

    /**
     * Add the Freqs of a slot, or read the slot again after a card change.
     *
     * @param nStatus How the load ended, one of MemcardTask::Status.
     * @param pProfiles The Freqs loaded.
     * @ghidraAddress NTSC-U/C: 0x001aba00
     * @ghidraAddress PAL: 0x001b3a50
     */
    void OnFreqsLoaded(int nStatus, std::vector<Campaign> *pProfiles) override;

private:
    /**
     * Load the next slot, or after the last slot hand the Freqs to the selection panels and go
     * to the selection screen.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001ab410
     * @ghidraAddress PAL: 0x001b3448
     */
    void LoadNextSlot();

    /**
     * Forget the Freqs and start the load of the first slot once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001abdc0
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    std::vector<Campaign> mProfiles; // The Freqs of every slot, then the prefabs.
    int mSelected[kMaxPlayers];      // The Freq each player starts on, or -1.
    int mFirstProfile[kMaxPlayers];  // The first Freq of each slot in mProfiles, or -1.
    int mLoaded;                     // Whether the next poll moves on to the next slot.
};
