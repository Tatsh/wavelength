#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class String;

/**
 * Step that deletes a file of the server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5038` and the
 * object is 0x14 bytes. The step fails with RepoRemixStatusMsg::kStatusFailed when the request
 * cannot be sent, and otherwise succeeds whatever the server reports.
 */
class FileDeleteTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039ca58
     * @ghidraAddress PAL: 0x0040b780
     */
    ~FileDeleteTask() override = default;

    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pName The file.
     * @param pnEnabled The word that enables the deletion when non-zero.
     */
    FileDeleteTask(String *pName, int *pnEnabled) : mName(pName), mEnabled(pnEnabled) {
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
     * @ghidraAddress NTSC-U/C: 0x002672f0
     * @ghidraAddress PAL: 0x00270fc8
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusFileDeleteResponse *pResponse);

protected:
    /**
     * Send the request, or finish when the deletion is not enabled.
     *
     * @ghidraAddress NTSC-U/C: 0x00267238
     * @ghidraAddress PAL: 0x00270f10
     */
    void OnStart() override;

private:
    String *mName; /*!< The file. */
    int *mEnabled; /*!< The word that enables the deletion when non-zero. */
};
