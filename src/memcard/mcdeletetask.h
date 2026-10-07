#pragma once

#include "memcard/memcardtask.h"
#include "os/string.h"

/**
 * Task that deletes a file or an empty directory.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x24 bytes. A file that
 * does not exist counts as deleted.
 */
class MCDeleteTask : public MemcardTask {
public:
    /** Construct an idle task. */
    MCDeleteTask() : mName(nullptr) {
    }

    /**
     * Prepare the task.
     *
     * @param nPort The memory card slot.
     * @param pszName The file or directory.
     * @ghidraAddress NTSC-U/C: 0x0015dd28
     * @ghidraAddress PAL: 0x0015f568
     */
    void Set(int nPort, const char *pszName);

    /**
     * Record the outcome and finish.
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x0015dd68
     * @ghidraAddress PAL: 0x0015f5a8
     */
    void OnDelete(int nResult) override;

    String mName; /*!< The file or directory. */

protected:
    /**
     * Issue the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015ddc8
     * @ghidraAddress PAL: 0x0015f608
     */
    void OnStart() override;
};
