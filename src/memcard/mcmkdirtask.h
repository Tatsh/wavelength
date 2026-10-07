#pragma once

#include "memcard/memcardtask.h"
#include "os/string.h"

/**
 * Task that creates a directory.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x24 bytes. A result of
 * MemcardTask::kStatusNotFound counts as success.
 */
class MCMkDirTask : public MemcardTask {
public:
    /** Construct an idle task. */
    MCMkDirTask() : mName(nullptr) {
    }

    /**
     * Prepare the task.
     *
     * @param nPort The memory card slot.
     * @param pszName The directory.
     * @ghidraAddress NTSC-U/C: 0x0015ddf0
     * @ghidraAddress PAL: 0x0015f630
     */
    void Set(int nPort, const char *pszName);

    /**
     * Record the outcome and finish.
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x0015de30
     * @ghidraAddress PAL: 0x0015f670
     */
    void OnMkdir(int nResult) override;

    String mName; /*!< The directory. */

protected:
    /**
     * Issue the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015de90
     * @ghidraAddress PAL: 0x0015f6d0
     */
    void OnStart() override;
};
