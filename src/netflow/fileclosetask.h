#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class String;

/**
 * Step that closes a file of the server after an upload.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5098` and the
 * object is 0x10 bytes. The step fails with RepoRemixStatusMsg::kStatusFailed.
 */
class FileCloseTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039c970
     * @ghidraAddress PAL: 0x0040b620
     */
    ~FileCloseTask() override = default;

    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pName The file.
     */
    explicit FileCloseTask(String *pName) : mName(pName) {
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
     * @ghidraAddress NTSC-U/C: 0x00266fa0
     * @ghidraAddress PAL: 0x00270ac8
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusFileResponse *pResponse);

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00266f08
     * @ghidraAddress PAL: 0x00270a30
     */
    void OnStart() override;

private:
    String *mName; /*!< The file. */
};
