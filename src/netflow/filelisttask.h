#pragma once

#include <list>

#include <librtmedius.h>

#include "netflow/nettask.h"
#include "os/string.h"

/**
 * Step that lists the files of the server whose names begin with a prefix.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5068` and the
 * object is 0x14 bytes. The server sends one file per response. The step fails with
 * RepoRemixStatusMsg::kStatusFailed when the request cannot be sent.
 */
class FileListTask : public NetTask {
public:
    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pPrefix The prefix.
     * @param pFiles Receives the names of the files.
     */
    FileListTask(String *pPrefix, std::list<String> *pFiles) : mPrefix(pPrefix), mFiles(pFiles) {
    }

    /**
     * Add a file whose name begins with the prefix, and finish the step the response is tagged
     * with after the last file.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x002670d8
     * @ghidraAddress PAL: 0x00270ca0
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusFileResponse *pResponse);

    String *mPrefix;           /*!< The prefix. */
    std::list<String> *mFiles; /*!< Receives the names of the files. */

protected:
    /**
     * Clear the names and send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00267018
     */
    void OnStart() override;
};
