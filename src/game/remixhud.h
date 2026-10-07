#pragma once

#include <vector>

#include "app/msgsink.h"
#include "gfx/gfxmanager.h"
#include "msg/joypadinputmsg.h"
#include "msg/message.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Remix menu of one player during a song, a list of panels that switch effects on and off.
 *
 * The RTTI records MsgSink as the one base and the nested RemixHUD::Panel. The session builds one
 * for each player. The menu takes the player's controller while it is both active and focused,
 * draws itself through the remix sub-panels of TheGfxManager, and reports what the player chose
 * through a callback.
 */
class RemixHUD : public MsgSink {
public:
    /** Kind of a panel. A kind also indexes mPanelOfType. */
    enum PanelType {
        kPanelLoop = 0,    /*!< `REMIX_LOOP`. */
        kPanelMute = 1,    /*!< `REMIX_MUTE` in kModeMute, blank otherwise, and always last. */
        kPanelChorus = 2,  /*!< `REMIX_CHORUS`. */
        kPanelStutter = 3, /*!< `REMIX_STUTTER`. */
        kPanelEffect = 4,  /*!< The effect whose label the constructor receives. */
        kPanelSolo = 5,    /*!< `REMIX_SOLO`, absent with several players on one console. */
        kPanelTempo = 6,   /*!< `REMIX_TEMPO`, for the first player of a console game only. */
        kPanelBoot = 7,    /*!< `REMIX_BOOT`, for the host of an online game only. */
        kNumPanelTypes = 8 /*!< The number of kinds. */
    };

    /** Value of SetMode() under which the mute panel can be chosen. */
    static constexpr int kModeMute = 4;

    /** What the player chose, as the callback receives it. */
    enum Event {
        kEventNone = 0,         /*!< A panel with nothing to switch. */
        kEventMuteOn = 1,       /*!< Mute, chosen while off. */
        kEventMuteOff = 2,      /*!< Mute, chosen while on. */
        kEventLoopOn = 3,       /*!< Loop, chosen while off. */
        kEventLoopOff = 4,      /*!< Loop, chosen while on. */
        kEventChorusOn = 5,     /*!< Chorus, chosen while off. */
        kEventChorusOff = 6,    /*!< Chorus, chosen while on. */
        kEventStutterOn = 7,    /*!< Stutter, chosen while off. */
        kEventStutterOff = 8,   /*!< Stutter, chosen while on. */
        kEventEffectOn = 9,     /*!< The constructor's effect, chosen while off. */
        kEventEffectOff = 10,   /*!< The constructor's effect, chosen while on. */
        kEventSoloOn = 11,      /*!< Solo, chosen while off. */
        kEventSoloOff = 12,     /*!< Solo, chosen while on. */
        kEventTempoUp = 13,     /*!< Up in the tempo control. */
        kEventTempoDown = 14,   /*!< Down in the tempo control. */
        kEventTempoOpened = 15, /*!< The tempo control opened. */
        kEventBootFirst = 16    /*!< Removal of player 0. Player n adds n. */
    };

    /**
     * Receiver of the player's choices.
     *
     * @param nPlayer The player's index.
     * @param nEvent One of Event.
     * @param pData The data SetCallback() received.
     */
    using Callback = void (*)(int nPlayer, int nEvent, void *pData);

    /**
     * Construct the menu of a player and add its panels.
     *
     * @param nPlayer The player's index.
     * @param pszEffectLabel The label of the kPanelEffect panel.
     * @ghidraAddress NTSC-U/C: 0x00151a80
     * @ghidraAddress PAL: 0x001532c8
     */
    RemixHUD(int nPlayer, const char *pszEffectLabel);

    /**
     * Release the controller when the menu has it.
     *
     * @ghidraAddress NTSC-U/C: 0x00151ce8
     * @ghidraAddress PAL: 0x00153530
     */
    ~RemixHUD() override;

    /**
     * Activate or deactivate the menu.
     *
     * Deactivation closes an open sub-panel and removes the cursor.
     *
     * @param nActive Non-zero to activate.
     * @ghidraAddress NTSC-U/C: 0x00151ea0
     * @ghidraAddress PAL: 0x001536e8
     */
    void SetActive(int nActive);

    /**
     * Show whether chorus is on.
     *
     * @param nOn Non-zero when on.
     * @ghidraAddress NTSC-U/C: 0x00151f78
     * @ghidraAddress PAL: 0x001537c0
     */
    void SetChorus(int nOn);

    /**
     * Show whether stutter is on.
     *
     * @param nOn Non-zero when on.
     * @ghidraAddress NTSC-U/C: 0x00151fd0
     * @ghidraAddress PAL: 0x00153818
     */
    void SetStutter(int nOn);

    /**
     * Show whether the constructor's effect is on.
     *
     * @param nOn Non-zero when on.
     * @ghidraAddress NTSC-U/C: 0x00152028
     * @ghidraAddress PAL: 0x00153870
     */
    void SetEffect(int nOn);

    /**
     * Show the tempo on the tempo panel and in the tempo control.
     *
     * A menu with no tempo panel ignores the call.
     *
     * @param fTempo The tempo, truncated for display.
     * @param nDimmed Non-zero to show the tempo panel unlit.
     * @ghidraAddress NTSC-U/C: 0x00152080
     * @ghidraAddress PAL: 0x001538c8
     */
    void SetTempo(float fTempo, int nDimmed);

    /**
     * Show whether loop is on.
     *
     * @param nOn Non-zero when on.
     * @ghidraAddress NTSC-U/C: 0x00152168
     * @ghidraAddress PAL: 0x001539b0
     */
    void SetLoop(int nOn);

    /**
     * Record whether mute is on, shown only in kModeMute.
     *
     * @param nOn Non-zero when on.
     * @ghidraAddress NTSC-U/C: 0x001521c0
     * @ghidraAddress PAL: 0x00153a08
     */
    void SetMute(int nOn);

    /**
     * Show whether solo is on.
     *
     * A menu with no solo panel ignores the call.
     *
     * @param nOn Non-zero when on.
     * @ghidraAddress NTSC-U/C: 0x00152228
     * @ghidraAddress PAL: 0x00153a70
     */
    void SetSolo(int nOn);

    /**
     * Set the mode of the player's track. The mode decides whether the mute panel can be chosen.
     *
     * @param nMode The mode. kModeMute labels and enables the mute panel.
     * @ghidraAddress NTSC-U/C: 0x00152288
     * @ghidraAddress PAL: 0x00153ad0
     */
    void SetMode(int nMode);

    /**
     * Remove a player from the players the host can remove.
     *
     * @param nPeer The player's index.
     * @ghidraAddress NTSC-U/C: 0x00152380
     * @ghidraAddress PAL: 0x00153bc8
     */
    void RemovePeer(int nPeer);

    /**
     * Set the receiver of the player's choices.
     *
     * @param pfnCallback The receiver.
     * @param pData The data the receiver receives.
     * @ghidraAddress NTSC-U/C: 0x00152428
     * @ghidraAddress PAL: 0x00153c70
     */
    void SetCallback(Callback pfnCallback, void *pData);

    /**
     * Open the panel at index 1, the tempo panel when the menu has one, unless the tempo control is
     * already open with an entry chosen.
     *
     * @ghidraAddress NTSC-U/C: 0x00153828
     * @ghidraAddress PAL: 0x00155090
     */
    void OpenTempo();

    /**
     * Close the tempo control when it is open.
     *
     * @ghidraAddress NTSC-U/C: 0x00153890
     * @ghidraAddress PAL: 0x001550f8
     */
    void CloseTempo();

    /**
     * Give the controller to the menu or take it away. The change matters only while active.
     *
     * @param nFocused Non-zero to give the controller.
     * @ghidraAddress NTSC-U/C: 0x001538d8
     * @ghidraAddress PAL: 0x00155140
     */
    void SetFocused(int nFocused);

    /**
     * Act on a press of the player's controller.
     *
     * @param pMsg The message.
     * @return False. The message is never reported as handled.
     * @ghidraAddress NTSC-U/C: 0x00153dc8
     * @ghidraAddress PAL: 0x00155630
     */
    bool DispatchPriv(Message *pMsg) override;

private:
    /**
     * Move the cursor to a panel of the main list.
     *
     * @param nIndex The panel.
     * @ghidraAddress NTSC-U/C: 0x00152438
     * @ghidraAddress PAL: 0x00153c80
     */
    void SelectPanel(int nIndex);

    /**
     * Move the cursor to an entry of the open sub-panel.
     *
     * @param nEntry The entry.
     * @ghidraAddress NTSC-U/C: 0x00152470
     * @ghidraAddress PAL: 0x00153cb8
     */
    void SelectEntry(int nEntry);

    /**
     * Open the sub-panel of a panel, or close the open one.
     *
     * @param nIndex The panel to open. Closing ignores it.
     * @param bOpen Whether to open rather than close.
     * @ghidraAddress NTSC-U/C: 0x001524b8
     * @ghidraAddress PAL: 0x00153d00
     */
    void OpenPanel(int nIndex, bool bOpen);

    /**
     * Choose the entry under the cursor of the open sub-panel.
     *
     * @return True. The choice is always taken.
     * @ghidraAddress NTSC-U/C: 0x00152590
     * @ghidraAddress PAL: 0x00153dd8
     */
    bool ConfirmEntry();

    /**
     * Show a player the host can remove, or `(CANCEL)`.
     *
     * @param nIndex The index into mPeers, or -1 for `(CANCEL)`.
     * @ghidraAddress NTSC-U/C: 0x00152630
     */
    void SetPeerIndex(int nIndex);

    /**
     * Append a panel to the main list.
     *
     * @param nType The kind, one of PanelType.
     * @param pszLabel The label.
     * @param nPostLabel Non-zero to show the label through PostPanelText().
     * @ghidraAddress NTSC-U/C: 0x001526a0
     * @ghidraAddress PAL: 0x00153f08
     */
    void AddPanel(int nType, const char *pszLabel, int nPostLabel);

    /**
     * Append the chorus panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00152b70
     * @ghidraAddress PAL: 0x001543d8
     */
    void AddChorusPanel();

    /**
     * Append the solo panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00152bc0
     * @ghidraAddress PAL: 0x00154428
     */
    void AddSoloPanel();

    /**
     * Append the loop panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00152c10
     * @ghidraAddress PAL: 0x00154478
     */
    void AddLoopPanel();

    /**
     * Append the stutter panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00152c60
     * @ghidraAddress PAL: 0x001544c8
     */
    void AddStutterPanel();

    /**
     * Append the panel of the constructor's effect.
     *
     * @param pszLabel The label.
     * @ghidraAddress NTSC-U/C: 0x00152cb0
     * @ghidraAddress PAL: 0x00154518
     */
    void AddEffectPanel(const char *pszLabel);

    /**
     * Append the mute panel, blank until kModeMute.
     *
     * @ghidraAddress NTSC-U/C: 0x00152cd8
     * @ghidraAddress PAL: 0x00154540
     */
    void AddMutePanel();

    /**
     * Append the tempo panel and show a tempo of 0.
     *
     * @ghidraAddress NTSC-U/C: 0x00152d00
     * @ghidraAddress PAL: 0x00154568
     */
    void AddTempoPanel();

    /**
     * Append the boot panel and list every other player.
     *
     * @ghidraAddress NTSC-U/C: 0x00153188
     * @ghidraAddress PAL: 0x001549f0
     */
    void AddBootPanel();

    /**
     * Schedule the text of an entry, later for later entries and sub-panels.
     *
     * @param nIndex The entry.
     * @param pszText The text.
     * @param ePanel The sub-panel.
     * @ghidraAddress NTSC-U/C: 0x001537a8
     * @ghidraAddress PAL: 0x00155010
     */
    void PostPanelText(int nIndex, const char *pszText, GfxManager::RemixSubPanel ePanel);

    /**
     * Move the cursor one step and schedule the repeat of a held direction.
     *
     * @param bUp Whether to move up rather than down.
     * @param bRepeat Whether the move repeats a held direction. A repeat shortens the next delay.
     * @ghidraAddress NTSC-U/C: 0x00153978
     * @ghidraAddress PAL: 0x001551e0
     */
    void Navigate(bool bUp, bool bRepeat);

    /**
     * Choose the panel or the entry under the cursor.
     *
     * @return Whether the choice was taken.
     * @ghidraAddress NTSC-U/C: 0x00153b78
     * @ghidraAddress PAL: 0x001553e0
     */
    bool Confirm();

    /**
     * Act on a button of the player's controller.
     *
     * @param pMsg The button message.
     * @return False. The message is never reported as handled.
     * @ghidraAddress NTSC-U/C: 0x00153cc0
     * @ghidraAddress PAL: 0x00155528
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mPlayer;                   /*!< The player's index and controller. */
    int mActive;                   /*!< Whether the menu is active. */
    int mFocused;                  /*!< Whether the menu may take the controller. */
    int mLoopOn;                   /*!< Whether loop is on. */
    int mMuteOn;                   /*!< Whether mute is on. */
    int mSoloOn;                   /*!< Whether solo is on. */
    int mChorusOn;                 /*!< Whether chorus is on. */
    int mStutterOn;                /*!< Whether stutter is on. */
    int mEffectOn;                 /*!< Whether the constructor's effect is on. */
    int mMode;                     /*!< The mode SetMode() set. */
    std::vector<int> mPeers;       /*!< The players the host can remove. */
    int mPeerIndex;                /*!< The index into mPeers shown, or -1. */
    int mPanel;                    /*!< The panel under the cursor. */
    int mEntry;                    /*!< The entry of the open sub-panel, or -1. */
    std::vector<int> mPanelTypes;  /*!< The kind of each panel. */
    std::vector<int> mPanelOfType; /*!< The panel of each kind, or -1. */
    std::vector<GfxManager::RemixSubPanel> mSubPanels; /*!< The sub-panel each panel opens. */
    std::vector<int> mEntryCounts; /*!< The entries of the sub-panel of each panel. */
    Ptr<Command> mRepeatUpCmd;     /*!< The command that repeats a held up. */
    Ptr<Command> mRepeatDownCmd;   /*!< The command that repeats a held down. */
    Callback mCallback;            /*!< The receiver of the choices, or null. */
    void *mCallbackData;           /*!< The data the receiver receives. */
};
