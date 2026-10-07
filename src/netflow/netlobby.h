#pragma once

#include "app/msgsink.h"

/**
 * The online lobby service: accounts, chat rooms, launchpads, and the remix repository.
 *
 * The RTTI includes the name. LobbyRT implements it, and TheNetLobby points at the one instance.
 * Only the members the metagame uses are declared.
 */
class NetLobby {
public:
    /**
     * Make a sink the receiver of the lobby's result messages. Vtable slot 2.
     *
     * @param pSink The sink.
     * @ghidraAddress NTSC-U/C: 0x0039afb0
     * @ghidraAddress PAL: 0x00409bf8
     */
    virtual void SetSink(MsgSink *pSink);

    MsgSink *mSink; /*!< The receiver of the result messages. +0x04 */
};

/**
 * The lobby service.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0aec
 */
extern NetLobby *TheNetLobby;
