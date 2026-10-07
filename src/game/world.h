#pragma once

#include "app/msgsink.h"

/**
 * One game in progress under one rule set.
 *
 * The RTTI records the class as deriving from MsgSink. Game, Remix, and Duel derive from it, one
 * for each rule set WorldMgr::Load() can build.
 */
class World : public MsgSink {
public:
    /**
     * Construct a world with no game state.
     *
     * @ghidraAddress NTSC-U/C: 0x00144330
     * @ghidraAddress PAL: 0x00145cc0
     */
    World();

    /**
     * Release the world.
     *
     * @ghidraAddress NTSC-U/C: 0x001443f0
     * @ghidraAddress PAL: 0x00145d80
     */
    ~World() override;
};
