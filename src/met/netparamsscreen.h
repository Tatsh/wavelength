#pragma once

#include <vector>

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "os/joypad.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uipanel.h"
#include "ui/uiscreen.h"

/**
 * Screen where the player chooses the mode and the skill of an online game, and a song from the
 * list of a song panel.
 *
 * The RTTI records the class as deriving from FreqScreen. Subclasses fill mModes and mSkills and
 * react to the choices.
 */
class NetParamsScreen : public FreqScreen {
public:
    /**
     * Construct a screen from the `song_panel_name`, `button_panel_name`, and `tri_back` of its
     * description.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017fd10
     * @ghidraAddress PAL: 0x00183f50
     */
    explicit NetParamsScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035b3b8
     * @ghidraAddress PAL: 0x003c8f10
     */
    ~NetParamsScreen() override {
    }

    /**
     * Route the choice, the left and right buttons, and the controller messages.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00180058
     * @ghidraAddress PAL: 0x00184298
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Exit and drop the names of the modes and the skills.
     *
     * @param pNextScreen The screen that is entering.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x0017fe40
     * @ghidraAddress PAL: 0x00184080
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Enter, give the focus to the button panel, and label the choices.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x0017fdc8
     * @ghidraAddress PAL: 0x00184008
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Label the `mode` and `skill` components with the chosen mode and skill, and disable `skill`
     * for a mode after mLastSkillMode.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0017ff08
     * @ghidraAddress PAL: 0x00184148
     */
    virtual void UpdateLabels();

    /**
     * React to a choice on the `mode` component. The base routine does nothing.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0035b530
     * @ghidraAddress PAL: 0x003c9088
     */
    virtual void OnModeChanged() {
    }

    /**
     * React to a change of the mode or the skill by listing the songs. The base routine does
     * nothing.
     *
     * The name is inferred.
     *
     * @param bReset Non-zero to select the first entry of the song list rather than the song
     *               already selected. The routines here pass 0.
     * @ghidraAddress NTSC-U/C: 0x0035b538
     * @ghidraAddress PAL: 0x003c9090
     */
    virtual void OnChoiceChanged([[maybe_unused]] int bReset) {
    }

    int mMode;                    /*!< The chosen entry of mModes. */
    int mSkill;                   /*!< The chosen entry of mSkills. */
    int mLastSkillMode;           /*!< The last mode the skill applies to. */
    int mDuelMode;                /*!< The entry of mModes for a duel, or -1 for none. */
    int mRemixMode;               /*!< The entry of mModes for a remix. */
    std::vector<String> mModes;   /*!< The names of the modes. */
    std::vector<String> mSkills;  /*!< The names of the skills. */
    UIPanel *mSongPanel;          /*!< The panel mSongPanelName identifies. */
    const char *mSongPanelName;   /*!< `song_panel_name`, the panel with the song list. */
    const char *mButtonPanelName; /*!< `button_panel_name`, the panel with the choices. */
    const char *mTriBackScreen;   /*!< `tri_back`, the screen the triangle button returns to. */
    int mChoiceCount;             /*!< The entries of the song list before the songs. */

protected:
    /**
     * Move a choice one step through a list, wrapping at both ends.
     *
     * @param nChoice The choice.
     * @param nCount The number of entries.
     * @param nButton kPadDLeft to step back, or any other button to step forward.
     * @return The new choice.
     */
    static int StepChoice(int nChoice, int nCount, int nButton) {
        if (nButton == kPadDLeft) {
            return nChoice - 1 > -1 ? nChoice - 1 : nCount - 1;
        }
        return nChoice + 1 < nCount ? nChoice + 1 : 0;
    }

    /**
     * Move to the song panel from `song`, or back to the button panel from `cursor`.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00180108
     * @ghidraAddress PAL: 0x00184348
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Change the mode or the skill with the left and right buttons, or move to the song panel
     * from `song` with the right button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x00180208
     * @ghidraAddress PAL: 0x00184448
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Return from the song panel to the button panel with the triangle or left button, or exit
     * the screen with the triangle button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True while a transition runs, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x00180410
     * @ghidraAddress PAL: 0x00184650
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};
