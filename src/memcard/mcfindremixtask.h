#pragma once

#include "memcard/mcgetdirtask.h"
#include "memcard/memcardtask.h"
#include "os/string.h"

/**
 * Task that searches the remix directories for a remix file and records its path in g_RemixPath.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x6c bytes. A remix found
 * in no directory fails with MemcardTask::kStatusNotFound.
 */
class MCFindRemixTask : public MemcardTask {
public:
    /**
     * Prepare the task.
     *
     * @param nPort The memory card slot.
     * @param pszName The remix file, with its extension.
     * @ghidraAddress NTSC-U/C: 0x00160bf0
     * @ghidraAddress PAL: 0x00163580
     */
    void Set(int nPort, const char *pszName);

    String mReserved10;   // +0x10, never read.
    MCGetDirTask mGetDir; /*!< The step that searches one directory. */
    int mDirIndex;        /*!< The directory searched now. */
    String mName;         /*!< The remix file. */

protected:
    /**
     * Clear g_RemixPath and start searching the first directory.
     *
     * @ghidraAddress NTSC-U/C: 0x00160c30
     * @ghidraAddress PAL: 0x001635c0
     */
    void OnStart() override;

    /**
     * Record the path once a directory includes the file, or search the next directory.
     *
     * @ghidraAddress NTSC-U/C: 0x00160d50
     * @ghidraAddress PAL: 0x001636e0
     */
    void OnPoll() override;

private:
    /**
     * Start searching the directory at mDirIndex, or fail once every directory was searched.
     *
     * @ghidraAddress NTSC-U/C: 0x00160c70
     * @ghidraAddress PAL: 0x00163600
     */
    void SearchNextDir();
};
