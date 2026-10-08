#pragma once

#include <deque>
#include <vector>

#include "os/JoypadButtonMsg.h"
#include "utl/Data.h"
#include "utl/MsgSink.h"

/** A command run by one button while a modifier combination is held. */
struct QuickCheat {
    int mButton;         /*!< The button. */
    DataArray *mCommand; /*!< The script command. */
};

/** A command run by a sequence of buttons. */
struct LongCheat {
    std::vector<int> mButtons; /*!< The sequence, first button first. */
    DataArray *mCommand;       /*!< The script command. */
    bool mFlag;                /*!< Set into CheatsManager::mFlags when the cheat runs. */
};

/**
 * Runs script commands entered on a controller.
 *
 * The RTTI records the class as deriving from MsgSink. The object is 0x58 bytes. A quick cheat
 * runs when its button is pressed while the left or right modifier combination is held. A long
 * cheat runs when the last buttons pressed, each within 750 milliseconds of the one before, match
 * its sequence.
 */
class CheatsManager : public MsgSink {
public:
    /**
     * Create a manager with no cheats.
     *
     * @ghidraAddress 0x10013580
     */
    CheatsManager();

    /**
     * Destroy the cheats. Their commands are not released.
     *
     * @ghidraAddress 0x10013690
     */
    virtual ~CheatsManager();

    /**
     * Handle a button message.
     *
     * @param msg The message.
     * @return Whether the message was a button change.
     * @ghidraAddress 0x10013b80
     */
    virtual bool DispatchPriv(Message *msg);

    /**
     * Run a cheat's command, first setting its `controller` entry to the controller.
     *
     * @param command The command.
     * @param pad The controller.
     * @ghidraAddress 0x100137f0
     */
    void ExecuteCheat(DataArray *command, int pad);

    /**
     * Run the cheats a button change completes.
     *
     * @param msg The button change.
     * @return True.
     * @ghidraAddress 0x10013830
     */
    bool OnButtonDown(JoypadButtonMsg *msg);

    std::vector<LongCheat> mLongCheats; /*!< The long cheats. */
    std::vector<QuickCheat>
        mQuickCheats[2];      /*!< The quick cheats of the left and right modifiers. */
    std::deque<int> mButtons; /*!< The last buttons pressed, oldest first. */
    float mLastTime;          /*!< When the last button was pressed, in ms. */
    bool mFlags;              /*!< The flags of the long cheats that ran. */
};

/**
 * Create the cheats manager from the `quick-cheats` and `long-cheats` configuration, and register
 * the `test` script command.
 *
 * @ghidraAddress 0x10013f60
 */
void CheatsInit();

/**
 * Destroy the cheats manager.
 *
 * @ghidraAddress 0x10014080
 */
void CheatsTerminate();
