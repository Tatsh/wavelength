#pragma once

#include "app/msgsink.h"
#include "game/world.h"

/**
 * Router of controller input to a running world.
 *
 * The RTTI records the class as deriving from MsgSink. The object is 0x34 bytes, and WorldMgr
 * allocates it under the tag "InputMgr".
 */
class InputMgr : public MsgSink {
public:
    /**
     * Construct a router for one world.
     *
     * @param pWorld The world input goes to.
     * @ghidraAddress NTSC-U/C: 0x00118068
     * @ghidraAddress PAL: 0x00119800
     */
    explicit InputMgr(World *pWorld);

    /**
     * Release the router.
     *
     * @ghidraAddress NTSC-U/C: 0x001185a8
     * @ghidraAddress PAL: 0x00119d40
     */
    ~InputMgr() override;
};
