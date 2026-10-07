#pragma once

#include "app/msgsink.h"

/**
 * Listener of the controllers that recognises the cheat sequences of the `long-cheats` section.
 *
 * The RTTI records the class as deriving from MsgSink. The object is 0x6c bytes and its vtable is
 * at `0x003d6e48`. Only the member the front end calls is declared, and the routines of the class
 * are not reconstructed.
 */
class CheatsManager : public MsgSink {
public:
    /**
     * Report the word at `+0x68` of the one instance.
     *
     * The word is non-zero once a cheat was entered. The name is inferred.
     *
     * @return The word.
     * @ghidraAddress NTSC-U/C: 0x00295178
     * @ghidraAddress PAL: 0x0029eda0
     */
    static int IsCheatEntered();
};
