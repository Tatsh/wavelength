#pragma once

#include <vector>

#include "game/streamtask.h"
#include "os/string.h"
#include "os/taskdonenotifier.h"

/**
 * Queue of narration streams that plays one stream at a time.
 *
 * The RTTI includes the class name and records TaskDoneNotifier as the base. The constructor has
 * no out-of-line copy.
 */
class StreamQueue : public TaskDoneNotifier {
public:
    /**
     * Release the queue and every stream in it.
     *
     * @ghidraAddress NTSC-U/C: 0x00141238
     * @ghidraAddress PAL: 0x00142bf8
     */
    ~StreamQueue() override;

    /**
     * Stop the stream at the head of the queue.
     *
     * @ghidraAddress NTSC-U/C: 0x00141318
     * @ghidraAddress PAL: 0x00142cd8
     */
    void Stop();

    /**
     * Append a stream to the queue.
     *
     * @param pszName The stream name.
     * @param bSkipIfBusy Drop the request when a stream is already queued.
     * @ghidraAddress NTSC-U/C: 0x00141348
     * @ghidraAddress PAL: 0x00142d08
     */
    void Enqueue(const char *pszName, bool bSkipIfBusy);

    /**
     * Poll the stream at the head, and start the next one when the head is done.
     *
     * @ghidraAddress NTSC-U/C: 0x00141510
     * @ghidraAddress PAL: 0x00142ed0
     */
    void Poll();

    /**
     * Pause or resume the stream at the head.
     *
     * @param bPaused Pause the stream.
     * @ghidraAddress NTSC-U/C: 0x001415e0
     * @ghidraAddress PAL: 0x00142fa0
     */
    void SetPaused(bool bPaused);

    /**
     * Arrange for a task to learn when the named stream is done.
     *
     * @param pTask The task waiting for the stream.
     * @param pszName The stream name.
     * @ghidraAddress NTSC-U/C: 0x00141618
     * @ghidraAddress PAL: 0x00142fd8
     */
    void NotifyWhenDone(Task *pTask, const char *pszName) override;

    std::vector<StreamTask *> mStreams; /*!< The queued streams, the playing one first. */
    String mDirectory;                  /*!< The directory the stream files are read from. */
};
