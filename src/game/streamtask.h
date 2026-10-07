#pragma once

#include "os/task.h"
#include "synth/streamplayer.h"

/**
 * Task that plays one narration stream of a StreamQueue.
 *
 * The RTTI includes the class name and records Task as the base. The task is done once the stream
 * has played to its end, and it then reports to the task waiting for the stream, if any.
 */
class StreamTask : public Task {
public:
    /**
     * Construct a task for a stream file.
     *
     * The task records the pointers and does not copy the text.
     *
     * @param pszName The stream name, the file name within the directory.
     * @param pszDirectory The directory the stream file is read from.
     * @ghidraAddress NTSC-U/C: 0x001416a0
     * @ghidraAddress PAL: 0x00143060
     */
    StreamTask(const char *pszName, const char *pszDirectory);

    /**
     * Set the task that learns when the stream is done.
     *
     * @param pTask The waiting task.
     * @ghidraAddress NTSC-U/C: 0x001416c8
     * @ghidraAddress PAL: 0x00143088
     */
    void SetWaitingTask(Task *pTask);

    /**
     * Pause or resume the stream.
     *
     * @param bPaused Pause the stream.
     * @ghidraAddress NTSC-U/C: 0x001417f0
     * @ghidraAddress PAL: 0x00143190
     */
    void SetPaused(bool bPaused);

    Task *mWaitingTask;     /*!< The task that learns when the stream is done, or null. */
    StreamPlayer *mPlayer;  /*!< The playing stream, or null. */
    const char *mName;      /*!< The stream name. */
    const char *mDirectory; /*!< The directory of the stream file. */

protected:
    /**
     * Open the stream file and give it to the sound output to play.
     *
     * @ghidraAddress NTSC-U/C: 0x001416d0
     */
    void OnStart() override;

    /**
     * Stop the stream and release it.
     *
     * @ghidraAddress NTSC-U/C: 0x00141780
     * @ghidraAddress PAL: 0x00143120
     */
    void OnStop() override;

    /**
     * Once the stream has ended, release it, finish the task, and report to the waiting task.
     *
     * @ghidraAddress NTSC-U/C: 0x00141810
     * @ghidraAddress PAL: 0x001431b0
     */
    void OnPoll() override;
};
