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
 * The RTTI records the class as deriving from MemcardTask. The object is 0x50 bytes.
 */
class MCGetAllRemixInfosTask : public MemcardTask {
public:
    /**
     * Prepare the task.
     *
     * @param nPort The memory card slot.
     * @param pInfos Receives one description for each file.
     * @ghidraAddress NTSC-U/C: 0x00161528
     */
    void Set(int nPort, std::vector<RemixInfo> *pInfos);

    MCGetRemixInfoTask mInfo;          /*!< The work that reads one description. */
    int mIndex;                        /*!< The index into mInfos of the file read now. */
    std::list<String>::iterator mIter; /*!< The file of g_RemixNames read now. */
    std::vector<RemixInfo> *mInfos;    /*!< Receives the descriptions. */

protected:
    /**
     * Size the descriptions to the files and start reading the first.
     *
     * @ghidraAddress NTSC-U/C: 0x00161578
     */
    void OnStart() override;

    /**
     * Decode a read description and start reading the next.
     *
     * @ghidraAddress NTSC-U/C: 0x00161720
     */
    void OnPoll() override;

private:
    /**
     * Start reading the file at mIter, or finish once every file was read.
     *
     * @ghidraAddress NTSC-U/C: 0x001616a0
     */
    void ReadNext();
};
