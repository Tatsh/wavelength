#pragma once

#include "os/command.h"
#include "os/ptr.h"

/**
 * Hints shown to a player who needs help with the controls.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The one instance is the
 * function-local static of shared(), and TheHelpText addresses it. A hint appears only in a game
 * with the hints option on, and never sooner than a second after the previous hint.
 */
class HelpText {
public:
    /**
     * Allow the hints.
     *
     * @ghidraAddress NTSC-U/C: 0x00117968
     * @ghidraAddress PAL: 0x00119100
     */
    HelpText();

    /**
     * Release the command that ends the attention cue.
     *
     * @ghidraAddress NTSC-U/C: 0x001179c0
     * @ghidraAddress PAL: 0x00119158
     */
    ~HelpText();

    /**
     * Report the one instance, constructing it on the first call.
     *
     * @return The instance.
     * @ghidraAddress NTSC-U/C: 0x00117a38
     * @ghidraAddress PAL: 0x001191d0
     */
    static HelpText *shared();

    /**
     * Allow or forbid the hints.
     *
     * @param bEnabled Whether hints are shown.
     * @ghidraAddress NTSC-U/C: 0x00117a90
     * @ghidraAddress PAL: 0x00119228
     */
    void SetEnabled(bool bEnabled);

    /**
     * Allow the hints and forget when the last one was shown.
     *
     * @ghidraAddress NTSC-U/C: 0x00117a98
     * @ghidraAddress PAL: 0x00119230
     */
    void Reset();

    /**
     * Warn that the energy is low.
     *
     * @return Whether the hint was shown.
     * @ghidraAddress NTSC-U/C: 0x00117aa8
     * @ghidraAddress PAL: 0x00119240
     */
    bool ShowEnergyLow();

    /**
     * Explain why the autocatcher power-up failed.
     *
     * @return Whether the hint was shown.
     * @ghidraAddress NTSC-U/C: 0x00117ae0
     * @ghidraAddress PAL: 0x00119278
     */
    bool ShowAutocatcherFailed();

    /**
     * Explain why the bumper power-up failed.
     *
     * @return Whether the hint was shown.
     * @ghidraAddress NTSC-U/C: 0x00117b18
     * @ghidraAddress PAL: 0x001192b0
     */
    bool ShowBumperFailed();

    /**
     * Explain why the crippler power-up failed.
     *
     * @return Whether the hint was shown.
     * @ghidraAddress NTSC-U/C: 0x00117b50
     * @ghidraAddress PAL: 0x001192e8
     */
    bool ShowCripplerFailed();

    /**
     * Explain that the notes ahead are energised, in the tutorial wording during the tutorial.
     *
     * @return Whether the hint was shown.
     * @ghidraAddress NTSC-U/C: 0x00117b88
     * @ghidraAddress PAL: 0x00119320
     */
    bool ShowNotesAreEnergized();

    /**
     * Explain that a player fell behind on a track.
     *
     * @param nPlayer The player, used only when the players share one console.
     * @return Whether the hint was shown.
     * @ghidraAddress NTSC-U/C: 0x00117c00
     * @ghidraAddress PAL: 0x00119398
     */
    bool ShowCatchBehind(int nPlayer);

    /**
     * Explain how to energise the notes, in a single-player game outside practice mode.
     *
     * @param bCatchable Whether the gems of the current bar can be caught, which picks the second
     *        line and adds the attention cue.
     * @ghidraAddress NTSC-U/C: 0x00117cb0
     * @ghidraAddress PAL: 0x00119448
     */
    void ShowEnergizeNotes(bool bCatchable);

    /**
     * Explain how to deploy the autocatcher power-up.
     *
     * @return Whether the hint was shown.
     * @ghidraAddress NTSC-U/C: 0x00117d58
     * @ghidraAddress PAL: 0x001194f0
     */
    bool ShowDeployAutocatcher();

    /**
     * Show a hint when the hints are allowed.
     *
     * @param pszFirst The first line, or its token.
     * @param pszSecond The second line, its token, or null.
     * @param bBeginnerOnly Whether to show the hint only at the two easiest skill levels.
     * @param bOnePadOnly Whether to show the hint only when one controller plays.
     * @param nPlayer The player the hint is for, or -1 for every player.
     * @param bLocalize Whether the lines are tokens to look up in the locale.
     * @return Whether the hint was shown.
     * @ghidraAddress NTSC-U/C: 0x00117d90
     * @ghidraAddress PAL: 0x00119528
     */
    bool Show(const char *pszFirst,
              const char *pszSecond,
              bool bBeginnerOnly,
              bool bOnePadOnly,
              int nPlayer,
              bool bLocalize);

    /**
     * Highlight the controller buttons for two seconds.
     *
     * @ghidraAddress NTSC-U/C: 0x00117f50
     * @ghidraAddress PAL: 0x001196e8
     */
    void PlayAttentionCue();

    /**
     * End the highlight of the controller buttons.
     *
     * @ghidraAddress NTSC-U/C: 0x00117fd8
     * @ghidraAddress PAL: 0x00119770
     */
    void StopAttentionCue();

private:
    bool mEnabled;            /*!< Whether hints are allowed. */
    float mLastShownMs;       /*!< The song time the last hint appeared at, in milliseconds. */
    Ptr<Command> mStopCueCmd; /*!< The command that runs StopAttentionCue(). */
};

/**
 * The hints.
 *
 * @ghidraAddress NTSC-U/C: 0x00435f28
 */
extern HelpText *TheHelpText;
