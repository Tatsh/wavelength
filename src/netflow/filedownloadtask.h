#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class String;

/**
 * Step that downloads a file of the server into a buffer.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5128` and the
 * object is 0x2c bytes. The step reports its progress through LobbyRT::ReportProgress(), from an
 * estimate of the bytes each poll receives, and fails with RepoRemixStatusMsg::kStatusFailed.
 */
class FileDownloadTask : public NetTask {
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
    FileDownloadTask(String *pName, char **ppBuffer, int *pnSize)
        : mName(pName), mBuffer(ppBuffer), mSize(pnSize), mReceived(0) {
    }

    /**
     * Copy a part of the file, and mark the step done after the last part.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00266a70
     * @ghidraAddress PAL: 0x00270598
     */
    static void OnResponse(int nConnection,
                           int nType,
                           int nSize,
                           const rtMediusFileTransferResponse *pResponse);

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00266900
     * @ghidraAddress PAL: 0x00270428
     */
    void OnStart() override;

    /**
     * Report the estimated progress, and finish once the last part arrived and the estimate passes
     * 99 percent.
     *
     * @ghidraAddress NTSC-U/C: 0x002669b0
     * @ghidraAddress PAL: 0x002704d8
     */
    void OnPoll() override;

private:
    /**
     * Copy a part of the file to the buffer, or fail when it overflows the buffer.
     *
     * @param pResponse The part.
     * @param nBytes The bytes of the part.
     * @ghidraAddress NTSC-U/C: 0x00266b28
     * @ghidraAddress PAL: 0x00270650
     */
    void Append(const rtMediusFileTransferResponse *pResponse, int nBytes);

    String *mName;   /*!< The file. */
    char **mBuffer;  /*!< The buffer. */
    int *mSize;      /*!< The size of the buffer in bytes. */
    int mReceived;   /*!< The bytes received. */
    int mDone;       /*!< Non-zero once the last part arrived. */
    float mPolls;    /*!< The polls since the start. */
    float mRate;     /*!< The bytes received per poll. */
    float mProgress; /*!< The part reported, from 0 to 1. */
};
