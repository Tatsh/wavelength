#pragma once

#include <librtmgcl.h>

#include "netflow/nettask.h"

/**
 * Step that starts the game server library and its peer-to-peer layer.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4d28`.
 * The step fails with LaunchpadAbortedMsg::kReasonInit.
 */
class InitMGCLTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00399c58
     * @ghidraAddress PAL: 0x004084e0
     */
    ~InitMGCLTask() override = default;

    /**
     * Ignore the first kind of request of the game server.
     *
     * @param nConnection The connection.
     * @param nType The kind of request.
     * @param nSize The size of the request.
     * @param pRequest The request.
     * @return 0, which the library does not read.
     * @ghidraAddress NTSC-U/C: 0x0025b0f0
     * @ghidraAddress PAL: 0x00264140
     */
    static int
    OnServerRequest0(int nConnection, int nType, int nSize, const rtMgclResponse *pRequest);

    /**
     * Accept the second kind of request of the game server with an empty reply.
     *
     * @param nConnection The connection.
     * @param nType The kind of request.
     * @param nSize The size of the request.
     * @param pRequest The request, whose identifier the reply repeats.
     * @return The result of rtMgclSendServerReply().
     * @ghidraAddress NTSC-U/C: 0x0025b0f8
     * @ghidraAddress PAL: 0x00264148
     */
    static int
    OnServerRequest1(int nConnection, int nType, int nSize, const rtMgclResponse *pRequest);

    /**
     * Ignore the third kind of request of the game server.
     *
     * @param nConnection The connection.
     * @param nType The kind of request.
     * @param nSize The size of the request.
     * @param pRequest The request.
     * @return 0, which the library does not read.
     * @ghidraAddress NTSC-U/C: 0x0025b148
     * @ghidraAddress PAL: 0x00264198
     */
    static int
    OnServerRequest2(int nConnection, int nType, int nSize, const rtMgclResponse *pRequest);

    /**
     * Ignore the fourth kind of request of the game server.
     *
     * @param nConnection The connection.
     * @param nType The kind of request.
     * @param nSize The size of the request.
     * @param pRequest The request.
     * @return 0, which the library does not read.
     * @ghidraAddress NTSC-U/C: 0x0025b150
     * @ghidraAddress PAL: 0x002641a0
     */
    static int
    OnServerRequest3(int nConnection, int nType, int nSize, const rtMgclResponse *pRequest);

protected:
    /**
     * Start the library, note whether it is ready, and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x0025b020
     * @ghidraAddress PAL: 0x00264070
     */
    void OnStart() override;
};
