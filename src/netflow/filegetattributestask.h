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
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039c720
     * @ghidraAddress PAL: 0x0040b3d8
     */
    ~FileGetAttributesTask() override = default;

    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pName The file.
     * @param pnSize Receives the size of the file in bytes.
     */
#ifdef VIDEO_STANDARD_PAL
    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pName The file.
     * @param pnSize Receives the size of the file in bytes.
     * @param pnPermissions Receives the access of the owner of the file.
     */
    FileGetAttributesTask(String *pName, int *pnSize, int *pnPermissions)
        : mName(pName), mSize(pnSize), mPermissions(pnPermissions) {
    }
#else
    FileGetAttributesTask(String *pName, int *pnSize) : mName(pName), mSize(pnSize) {
    }
#endif

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
#ifdef VIDEO_STANDARD_PAL
    int *mPermissions; /*!< Receives the access of the owner of the file. */
#endif

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00266780
     * @ghidraAddress PAL: 0x00270298
     */
    void OnStart() override;
};
