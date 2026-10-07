#pragma once

#include "memcard/memcardtask.h"
#include "os/string.h"

/**
 * Task that opens a file and records its descriptor in MemcardTask::sFd.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x28 bytes.
 */
class MCOpenTask : public MemcardTask {
public:
    /** Construct an idle task. */
    MCOpenTask() : mName(nullptr) {
    }

    /**
     * Prepare the task.
     *
     * @param nPort The memory card slot.
     * @param pszName The file.
     * @param nMode The library's open mode bits.
     * @ghidraAddress NTSC-U/C: 0x0015d9b8
     * @ghidraAddress PAL: 0x0015f1d0
     */
    void Set(int nPort, const char *pszName, int nMode);

    /**
     * Record the descriptor and the outcome, and finish.
     *
     * @param nResult The file descriptor, or a negative library error.
     * @ghidraAddress NTSC-U/C: 0x0015da08
     * @ghidraAddress PAL: 0x0015f220
     */
    void OnOpen(int nResult) override;

    String mName; /*!< The file. */
    int mMode;    /*!< The library's open mode bits. */

protected:
    /**
     * Issue the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015da60
     */
    void OnStart() override;
};
