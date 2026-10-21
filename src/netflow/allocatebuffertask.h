#pragma once

#include "netflow/nettask.h"

/**
 * Step that allocates the buffer of a download.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5158` and the
 * object is 0x14 bytes. The step always succeeds.
 */
class AllocateBufferTask : public NetTask {
public:
    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param ppBuffer Receives the buffer.
     * @param pnSize The size of the buffer in bytes.
     */
    AllocateBufferTask(char **ppBuffer, int *pnSize) : mBuffer(ppBuffer), mSize(pnSize) {
    }

protected:
    /**
     * Allocate the buffer from the `FileDownload` pool and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x002668b0
     * @ghidraAddress PAL: 0x002703d8
     */
    void OnStart() override;

private:
    char **mBuffer; /*!< Receives the buffer. */
    int *mSize;     /*!< The size of the buffer in bytes. */
};
