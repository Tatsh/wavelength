#pragma once

#include <deque>
#include <vector>

#include "app/msgsink.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"

/**
 * Listener of the controllers that recognises the cheat sequences of the `long-cheats` section.
 *
 * The RTTI records the class as deriving from MsgSink. The object is 0x6c bytes and its vtable is
 * at `0x003d6e48`. Init() creates the one instance.
 */
class CheatsManager : public MsgSink {
public:
    /** Presses mHistory retains. */
    static constexpr int kMaxHistory = 16;

    /** Buttons one long cheat may have. */
    static constexpr int kMaxCheatButtons = 16;

    /** Combinations that select a table of mComboCheats. */
    enum Combo {
        kComboLeft = 0,  /*!< L1, L2, and L3 held. */
        kComboRight = 1, /*!< R1, R2, and R3 held. */
        kComboCount = 2, /*!< Entries of mComboCheats. */
    };

    /** A sequence of presses that runs a script command. */
    struct LongCheat {
        std::vector<int> mButtons; /*!< The presses, one JoypadButton each. */
        DataArray *mAction;        /*!< The command the sequence runs. */
        int mMarksEntered;         /*!< Whether the sequence sets mCheatEntered. */
    };

    /** A press made while a combination is held that runs a script command. */
    struct ComboCheat {
        int mButton;        /*!< The press, one JoypadButton. */
        DataArray *mAction; /*!< The command the press runs. */
    };

    /**
     * Construct a manager with no cheat.
     *
     * @ghidraAddress NTSC-U/C: 0x00294678
     * @ghidraAddress PAL: 0x0029e2a0
     */
    CheatsManager();

    /**
     * Run a cheat's script command, first storing the controller in its `controller` entry.
     *
     * @param pAction The command.
     * @param nPad The controller that entered the cheat.
     * @ghidraAddress NTSC-U/C: 0x00294728
     * @ghidraAddress PAL: 0x0029e350
     */
    void RunAction(DataArray *pAction, int nPad);

    /**
     * Add a press to the history and run every cheat it completes.
     *
     * A press more than 750 milliseconds after the previous one starts a new history. A long cheat
     * matches when its presses open the history, and the history is then cleared.
     *
     * @param pMsg The press or release.
     * @return Always true.
     * @ghidraAddress NTSC-U/C: 0x00294788
     * @ghidraAddress PAL: 0x0029e3b0
     */
    bool HandleButton(JoypadInputMsg *pMsg);

    /**
     * Handle controller button messages.
     *
     * @param pMsg The message.
     * @return HandleButton() for a JoypadInputMsg, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x00294c20
     * @ghidraAddress PAL: 0x0029e848
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report whether a cheat that marks the game was entered.
     *
     * @return mCheatEntered of the one instance.
     * @ghidraAddress NTSC-U/C: 0x00295178
     * @ghidraAddress PAL: 0x0029eda0
     */
    static int IsCheatEntered();

    /**
     * Create the one instance, listen to the controllers, load the `long-cheats` section, and
     * register the `test` script command.
     *
     * @ghidraAddress NTSC-U/C: 0x00295188
     * @ghidraAddress PAL: 0x0029edb0
     */
    static void Init();

    /**
     * Stop listening to the controllers and delete the one instance.
     *
     * @ghidraAddress NTSC-U/C: 0x00295210
     * @ghidraAddress PAL: 0x0029ee38
     */
    static void Terminate();

private:
    /**
     * Print the arguments of the `test` script command.
     *
     * @param pCommand The command.
     * @param pUserData Not used.
     * @ghidraAddress NTSC-U/C: 0x00294c90
     * @ghidraAddress PAL: 0x0029e8b8
     */
    static void TestCmd(DataArray *pCommand, void *pUserData);

    /**
     * Add the long cheats of a `long-cheats` section to the one instance.
     *
     * Each entry after the tag lists the presses, the command, and whether the cheat marks the
     * game. An entry with more than kMaxCheatButtons presses or an invalid button is skipped.
     *
     * @param pCheats The section.
     * @ghidraAddress NTSC-U/C: 0x00294cf8
     * @ghidraAddress PAL: 0x0029e920
     */
    static void LoadLongCheats(DataArray *pCheats);

    std::vector<LongCheat> mLongCheats;                /*!< The sequences. */
    std::vector<ComboCheat> mComboCheats[kComboCount]; /*!< The presses of each combination. */
    std::deque<int> mHistory;                          /*!< The latest presses, oldest first. */
    int mReserved60;                                   // +0x60, cleared and never read.
    float mLastPressMs;                                /*!< SystemMs() at the latest press. */
    int mCheatEntered; /*!< Non-zero once a cheat that marks the game was entered. */
};
