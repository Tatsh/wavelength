#pragma once

#include "app/msgsink.h"

/**
 * The online session of a console that joins it.
 *
 * The RTTI records the class as deriving from LaunchpadRT, and LaunchpadRT as deriving from MsgSink
 * and NetLaunchpad. The object is 0x278 bytes, and its constructor stores the instance in
 * TheNetLaunchpad. Only the factory its callers here use is declared.
 */
class JoinpadRT {
public:
    /**
     * Start joining a session.
     *
     * @param pScreen The screen that receives the result of the attempt.
     * @param pGame The sink that receives the messages of the session.
     * @param nLaunchpadId The first identifier of the session.
     * @param nLaunchpadWorld The second identifier of the session.
     * @ghidraAddress NTSC-U/C: 0x00254d98
     * @ghidraAddress PAL: 0x0025dcb0
     */
    static void Create(MsgSink *pScreen, MsgSink *pGame, int nLaunchpadId, int nLaunchpadWorld);
};
