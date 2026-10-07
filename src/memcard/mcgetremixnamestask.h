#pragma once

#include "memcard/mcgetdirtask.h"
#include "memcard/memcardtask.h"

/**
 * Task that lists the remix files of every remix directory into g_RemixNames.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x44 bytes. A directory
 * that cannot be listed adds no files.
 */
class MCGetRemixNamesTask : public MemcardTask {
public:
    /**
     * Prepare the task.
     *
     * @param nPort The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x00160e08
     * @ghidraAddress PAL: 0x00163798
     */
    void Set(int nPort);

    MCGetDirTask mGetDir; /*!< The step that lists one directory. */
    int mDirIndex;        /*!< The directory listed now. */

protected:
    /**
     * Clear g_RemixNames and start listing the first directory.
     *
     * @ghidraAddress NTSC-U/C: 0x00160e28
     */
    void OnStart() override;

    /**
     * Add the files of a listed directory to g_RemixNames and list the next directory.
     *
     * @ghidraAddress NTSC-U/C: 0x00160f38
     */
    void OnPoll() override;

private:
    /**
     * Start listing the directory at mDirIndex, or finish once every directory was listed.
     *
     * @ghidraAddress NTSC-U/C: 0x00160e60
     * @ghidraAddress PAL: 0x00163848
     */
    void ListNextDir();
};
