#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class String;

/**
 * Step that reads the size of a file of the server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5188` and the
 * object is 0x14 bytes. The step fails with RepoRemixStatusMsg::kStatusFailed.
 */
class FileGetAttributesTask : public NetTask {
public:
    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pName The file.
     * @param pnSize Receives the size of the file in bytes.
     */
    FileGetAttributesTask(String *pName, int *pnSize) : mName(pName), mSize(pnSize) {
    }

    /**
     * Take the size and finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00266818
     * @ghidraAddress PAL: 0x00270330
     */
    static void OnResponse(int nConnection,
                           int nType,
                           int nSize,
                           const rtMediusFileAttributesResponse *pResponse);

    String *mName; /*!< The file. */
    int *mSize;    /*!< Receives the size of the file in bytes. */

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00266780
     */
    void OnStart() override;
};
