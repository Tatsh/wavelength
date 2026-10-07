#pragma once

#include <vector>

#include "game/netgamescore.h"
#include "msg/message.h"

/**
 * Identity that GameEndedMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b80
 */
extern int g_nGameEndedMsgType;

/**
 * Message that reports the end of an online session, with the final scores.
 *
 * The RTTI records the class as deriving from Message. Only the members its receivers here read
 * are declared.
 */
class GameEndedMsg : public Message {
public:
    /** Values of mResult. */
    enum Result {
        kResultFinished = 1,       /*!< The song ended normally. */
        kResultHostAborted = 2,    /*!< The host left the session. */
        kResultConnectionLost = 3, /*!< The connection to the session was lost. */
    };

    int mResult;                       /*!< One of Result. */
    std::vector<NetGameScore> mScores; /*!< The final score of each player. */
};
