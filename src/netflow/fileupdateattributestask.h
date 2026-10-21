#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class String;

/**
 * Step that clears the permissions of a file of the server, which hides the file.
 *
 * The European release has the step, and LobbyRT::DeleteFile() runs it instead of FileDeleteTask.
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x0046ba40` and
 * the object is 0x14 bytes. The step fails with RepoRemixStatusMsg::kStatusFailed when the request
 * cannot be sent, and otherwise succeeds whatever the server reports.
 */
class FileUpdateAttributesTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress PAL: 0x0040b708
     */
    ~FileUpdateAttributesTask() override = default;

    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pName The file.
     */
    explicit FileUpdateAttributesTask(String *pName) : mName(pName) {
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
     * @ghidraAddress PAL: 0x00270ea8
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusFileResponse *pResponse);

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress PAL: 0x00270e00
     */
    void OnStart() override;

private:
    String *mName;       /*!< The file. */
    int mReserved10 = 0; // +0x10, the constructor clears it and the step does not read it.
};
