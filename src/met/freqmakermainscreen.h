#pragma once

#include <vector>

#include "game/campaign.h"
#include "met/freqmakerundoscreen.h"
#include "met/keyboarduser.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The main screen of the Freq maker, where the player picks a prefab Freq or a random one, opens
 * the custom screen, types the name of the Freq, and saves it.
 *
 * The RTTI records the class as deriving from FreqMakerUndoScreen and from KeyboardUser at
 * `+0x70`. The object is 0x1c0 bytes and its vtables are at `0x003cfeb8` and, for KeyboardUser,
 * `0x003cfe98`. The metagame registers the class for the screen type `f_maker_main_screen`, and
 * the front-end description's `f_maker` screen is one. Its `f_maker` panel has the buttons
 * `prefabs`, `random`, `custom`, `name`, and `save`, and its `f_maker_p` AvatarPanel shows the
 * Freq of the first player. The screen edits the Freq in place.
 *
 * FreqConfirmScreen prepares the screen before it opens. Saving hands the Freq to the
 * `fmaker_save_freq` SaveEditedFreqScreen, or to `fmaker_save_freq_net` online.
 */
class FreqMakerMainScreen : public FreqMakerUndoScreen, public KeyboardUser {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00360d28
     * @ghidraAddress PAL: 0x003cf240
     */
    explicit FreqMakerMainScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00360c68
     * @ghidraAddress PAL: 0x003cf180
     */
    ~FreqMakerMainScreen() override;

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `f_maker_main_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00360c28
     * @ghidraAddress PAL: 0x003cf140
     */
    static UIScreen *New(DataArray *pData) {
        return new FreqMakerMainScreen(pData);
    }

    /**
     * Route chosen buttons, focus changes, controller buttons, and the end of the entry, and pass
     * on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a0338
     * @ghidraAddress PAL: 0x001a8018
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Show the Freq and list the prefabs, then start the entry.
     *
     * Entering from a screen outside the Freq maker forgets the typed name. A new Freq that has
     * not gone to the save screen yet starts again from a default profile and the first prefab.
     * Returning from the keyboard checks the typed name, sending an empty name or one with spaces
     * at either end to `no_empty_filename_screen` or `no_lead_trail_spaces_screen`, and otherwise
     * moves the focus to `save`. The name of the Freq is written on `f_maker_p_01.txt`.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019eb48
     * @ghidraAddress PAL: 0x001a6860
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Start the exit, and turn the camera to the bust of the Freq unless the custom screen
     * follows.
     *
     * @param pNextScreen The screen that replaces this one, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019f5b8
     * @ghidraAddress PAL: 0x001a7200
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Restore the profile the screen started from and forget the name.
     *
     * @ghidraAddress NTSC-U/C: 0x0019f938
     * @ghidraAddress PAL: 0x001a7588
     */
    void Undo() override;

    /**
     * Receive the name the player typed, and go to the save screen when saving waits for it.
     *
     * @param pszText The name.
     * @return Zero when the save screen opened, otherwise 1.
     * @ghidraAddress NTSC-U/C: 0x0019f7b8
     * @ghidraAddress PAL: 0x001a7400
     */
    int ReceiveKeyboardText(const char *pszText) override;

    /**
     * Find the description of a prefab among the `prefabs` and `locked_prefabs` of the metagame
     * configuration.
     *
     * @param pszPrefab The prefab, a symbol.
     * @return The description, or null.
     * @ghidraAddress NTSC-U/C: 0x0019f6e8
     * @ghidraAddress PAL: 0x001a7330
     */
    DataArray *FindPrefab(const char *pszPrefab);

    /**
     * Open the keyboard to type the name, starting from the current name or `player_1`.
     *
     * @ghidraAddress NTSC-U/C: 0x0019f828
     * @ghidraAddress PAL: 0x001a7470
     */
    void OpenKeyboard();

    /**
     * Show the help of the `save` button, naming the memory card.
     *
     * @param pComponent The component that has the focus.
     * @ghidraAddress NTSC-U/C: 0x0019f9f0
     * @ghidraAddress PAL: 0x001a7640
     */
    void ShowSaveHelp(UIComponent *pComponent);

    /**
     * Give the first player the name and mark the profile as the player's own, then go to the
     * save screen.
     *
     * Saving under another name than the profile's, or saving a new Freq, never overwrites a
     * saved Freq.
     *
     * @ghidraAddress NTSC-U/C: 0x0019fa88
     * @ghidraAddress PAL: 0x001a7768
     */
    void GotoSave();

    /**
     * Show the help of the component that received the focus.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0019fc28
     * @ghidraAddress PAL: 0x001a7908
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Show the help of the focused component once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0019fc58
     * @ghidraAddress PAL: 0x001a7938
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Go to the save screen when `save` was chosen with the cross button.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0019fca0
     * @ghidraAddress PAL: 0x001a7980
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Act on a button about to be chosen.
     *
     * The cross button on `random` gives every part but the right arm and the instrument a random
     * unlocked choice and colour, and the torso a random emblem. On `save` without a typed name,
     * or with the name `Player 1`, it opens the keyboard and saves once the name arrives. On
     * `name` it opens the keyboard, or `f_maker_name_locked_screen` when the name is locked. Left
     * and right on `prefabs` step through the prefabs.
     *
     * @param pMsg The message.
     * @return True when the keyboard opened or a prefab was loaded, otherwise the result of
     *         FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x0019fd10
     * @ghidraAddress PAL: 0x001a79f0
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Return to the Freq menu on the triangle button, through `f_maker_lose_main_changes` when
     * the Freq changed.
     *
     * @param pMsg The message.
     * @return True while the screen moves, otherwise the result of FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x001a0198
     * @ghidraAddress PAL: 0x001a7e78
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mReserved74[3];                 // +0x74, not yet recovered.
    int mPrefabIndex;                   /*!< The entry of mPrefabs that `prefabs` shows. +0x80 */
    std::vector<const char *> mPrefabs; /*!< The prefabs, then the unlocked locked prefabs. */
    int mEditing;      /*!< 1 when the screen edits the player's Freq, 0 for a new one. +0x94 */
    int mNameTyped;    /*!< Non-zero once a name arrived from the keyboard. */
    int mChanged;      /*!< Non-zero once the Freq changed. */
    int mSaveStarted;  /*!< Non-zero once the Freq went to the save screen. +0xa0 */
    int mNameTrimmed;  /*!< Non-zero when spaces were removed from the typed name. */
    int mSaveOnName;   /*!< Non-zero while saving waits for the keyboard. */
    String mFreqName;  /*!< The name of the Freq. +0xac */
    Campaign mProfile; /*!< The profile the screen started from. +0xc0 */
    DataArray *mPrefabsData;       /*!< The `prefabs` of the metagame configuration. +0x1b8 */
    DataArray *mLockedPrefabsData; /*!< The `locked_prefabs` of the metagame configuration. */
};
