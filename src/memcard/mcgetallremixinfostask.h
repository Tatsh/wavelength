#pragma once

#include <list>
#include <vector>

#include "game/remixinfo.h"
#include "memcard/mcgetremixinfotask.h"
#include "memcard/memcardtask.h"
#include "os/string.h"

/**
 * Task that reads the RemixInfo of every remix file in g_RemixNames.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x50 bytes, and 0x4c in
 * the PAL build.
 */
class MCGetAllRemixInfosTask : public MemcardTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00353470
     * @ghidraAddress PAL: 0x003c0778
     */
    ~MCGetAllRemixInfosTask() override = default;

    /**
     * Prepare the task.
     *
     * @param nPort The memory card slot.
     * @param pInfos Receives one description for each file.
     * @ghidraAddress NTSC-U/C: 0x00161528
     * @ghidraAddress PAL: 0x00164068
     */
    void Set(int nPort, std::vector<RemixInfo> *pInfos);

    MCGetRemixInfoTask mInfo; /*!< The work that reads one description. */
    int mIndex;               /*!< The index into g_RemixNames of the file read now. */
#ifndef VIDEO_STANDARD_PAL
    std::list<String>::iterator mIter; /*!< The file of g_RemixNames read now. */
#endif
    std::vector<RemixInfo> *mInfos; /*!< Receives the descriptions. */

protected:
    /**
     * Size the descriptions to the files and start reading the first. The PAL build empties the
     * descriptions and only reserves room for them.
     *
     * @ghidraAddress NTSC-U/C: 0x00161578
     * @ghidraAddress PAL: 0x001640a0
     */
    void OnStart() override;

    /**
     * Decode a read description and start reading the next. The PAL build appends each
     * description, and deletes a file whose size is not that of a description instead.
     *
     * @ghidraAddress NTSC-U/C: 0x00161720
     * @ghidraAddress PAL: 0x001641c0
     */
    void OnPoll() override;

private:
    /**
     * Start reading the file at mIndex, or finish once every file was read.
     *
     * @ghidraAddress NTSC-U/C: 0x001616a0
     * @ghidraAddress PAL: 0x00164138
     */
    void ReadNext();
};
