#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class String;

/**
 * Step that creates a file of the server for an upload.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d50f8` and the
 * object is 0x14 bytes. The step fails with RepoRemixStatusMsg::kStatusFailed.
 */
class FileCreateTask : public NetTask {
public:
    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pName The file.
     * @param pnSize The size of the file in bytes.
     */
    FileCreateTask(String *pName, int *pnSize) : mName(pName), mSize(pnSize) {
    }

    /**
     * Finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00266c78
     * @ghidraAddress PAL: 0x002707a0
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusFileResponse *pResponse);

protected:
    /**
     * Send the request, with every access for every account.
     *
     * @ghidraAddress NTSC-U/C: 0x00266bc0
     * @ghidraAddress PAL: 0x002706e8
     */
    void OnStart() override;

private:
    String *mName; /*!< The file. */
    int *mSize;    /*!< The size of the file in bytes. */
};
