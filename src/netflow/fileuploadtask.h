#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class String;

/**
 * Step that uploads a buffer to a file of the server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d50c8` and the
 * object is 0x28 bytes. The step reports its progress through LobbyRT::ReportProgress(), from an
 * estimate of the bytes each poll sends, and fails with RepoRemixStatusMsg::kStatusFailed.
 */
class FileUploadTask : public NetTask {
public:
    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pName The file.
     * @param ppBuffer The buffer.
     * @param pnSize The size of the buffer in bytes.
     */
    FileUploadTask(String *pName, char **ppBuffer, int *pnSize)
        : mName(pName), mBuffer(ppBuffer), mSize(pnSize) {
    }

    /**
     * Take the progress of a part, and mark the step done after the last part.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00266e58
     * @ghidraAddress PAL: 0x00270980
     */
    static void OnResponse(int nConnection,
                           int nType,
                           int nSize,
                           const rtMediusFileTransferResponse *pResponse);

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00266cf0
     * @ghidraAddress PAL: 0x00270818
     */
    void OnStart() override;

    /**
     * Report the estimated progress, and finish once the last part was sent.
     *
     * @ghidraAddress NTSC-U/C: 0x00266db8
     * @ghidraAddress PAL: 0x002708e0
     */
    void OnPoll() override;

private:
    String *mName;   /*!< The file. */
    char **mBuffer;  /*!< The buffer. */
    int *mSize;      /*!< The size of the buffer in bytes. */
    int mDone;       /*!< Non-zero once the last part was sent. */
    float mPolls;    /*!< The polls since the start. */
    float mRate;     /*!< The bytes sent per poll. */
    float mProgress; /*!< The part reported, from 0 to 1. */
};
