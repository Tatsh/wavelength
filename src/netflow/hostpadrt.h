#pragma once

#include "app/msgsink.h"

/**
 * The online session of a console that hosts it.
 *
 * The RTTI records the class as deriving from LaunchpadRT, and LaunchpadRT as deriving from MsgSink
 * and NetLaunchpad. The object is 0x328 bytes, and its constructor stores the instance in
 * TheNetLaunchpad. Only the factory its callers here use is declared.
 */
class HostpadRT {
public:
    /**
     * Start hosting a session.
     *
     * @param pScreen The screen that receives the result of the attempt.
     * @param pGame The sink that receives the messages of the session.
     * @ghidraAddress NTSC-U/C: 0x00254d48
     * @ghidraAddress PAL: 0x0025dc60
     */
    static void Create(MsgSink *pScreen, MsgSink *pGame);
};
