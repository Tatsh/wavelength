#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that starts the network library and its lobby client.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d57e8`. The
 * step sets LobbyRT::mRtReady, and it fails with LobbyConnectResultMsg::kResultFailed.
 */
class InitRTTask : public NetTask {
public:
    /**
     * Allocate a block for the library from the `rtime` pool.
     *
     * The name is inferred.
     *
     * @param nSize The size of the block in bytes.
     * @return The block, aligned to 16 bytes.
     * @ghidraAddress NTSC-U/C: 0x00267358
     * @ghidraAddress PAL: 0x00271030
     */
    static void *AllocBlock(int nSize);

    /**
     * Release a block AllocBlock() returned.
     *
     * The name is inferred.
     *
     * @param pBlock The block.
     * @ghidraAddress NTSC-U/C: 0x00267380
     * @ghidraAddress PAL: 0x00271058
     */
    static void FreeBlock(void *pBlock);

    /**
     * Receive a notice of the lobby server. The body is empty.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of notice.
     * @param nSize The size of the notice.
     * @param pNotice The notice.
     * @ghidraAddress NTSC-U/C: 0x002673a0
     * @ghidraAddress PAL: 0x00271078
     */
    static void OnNotice(int nConnection, int nType, int nSize, const rtMediusResponse *pNotice);

    /**
     * A step that is never set. OnStop() clears it.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0af8
     */
    static InitRTTask *sTask;

protected:
    /**
     * Start the peer-to-peer layer, register the messages of the online session, and start the
     * lobby client.
     *
     * @ghidraAddress NTSC-U/C: 0x00262d78
     * @ghidraAddress PAL: 0x0026bed0
     */
    void OnStart() override;

    /**
     * Clear sTask.
     *
     * @ghidraAddress NTSC-U/C: 0x00262f48
     * @ghidraAddress PAL: 0x0026c0a0
     */
    void OnStop() override;
};
