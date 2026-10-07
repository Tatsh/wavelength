#pragma once

#include "memcard/memcarddirentry.h"
#include "memcard/memcardserialtask.h"
#include "memcard/memcardtask.h"
#include "os/string.h"

/**
 * Task that lists the entries matching a name into MemcardTask::sDirEntries and
 * MemcardTask::sDirCount.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x30 bytes. A name that
 * has no match counts as success with no entries.
 */
class MCGetDirTask : public MemcardTask {
public:
    /**
     * Prepare the task.
     *
     * @param pOwner The task that learns when this one ends through OnDirListed(), or null.
     * @param nPort The memory card slot.
     * @param pszName The name. It may include wildcards.
     * @param nMaxEntries The number of entries requested.
     * @param nMode The library's listing mode.
     * @ghidraAddress NTSC-U/C: 0x0015deb8
     * @ghidraAddress PAL: 0x0015f6f8
     */
    void Set(MemcardSerialTask *pOwner, int nPort, const char *pszName, int nMaxEntries, int nMode);

    /**
     * Record the entries and the outcome, finish, and tell mOwner.
     *
     * @param nResult The number of entries, or a negative library error.
     * @param pEntries The entries.
     * @ghidraAddress NTSC-U/C: 0x0015df28
     * @ghidraAddress PAL: 0x0015f768
     */
    void OnGetDir(int nResult, MemcardDirEntry *pEntries) override;

    MemcardSerialTask *mOwner; /*!< The task OnGetDir() tells, or null. */
    String mName;              /*!< The name, wildcards included. */
    int mMode;                 /*!< The library's listing mode. */
    int mMaxEntries;           /*!< The entries requested, and the entries listed once done. */

protected:
    /**
     * Issue the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015dfe8
     * @ghidraAddress PAL: 0x0015f828
     */
    void OnStart() override;
};
