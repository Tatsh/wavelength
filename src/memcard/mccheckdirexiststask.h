#pragma once

#ifdef VIDEO_STANDARD_PAL

#include "memcard/mcgetdirtask.h"
#include "memcard/memcardserialtask.h"
#include "memcard/memcardtask.h"
#include "os/string.h"

/**
 * Task that lists a save directory and deletes it when a file it needs is missing or damaged.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x58 bytes. A directory
 * is whole when it has the icon file, `icon.sys`, and the file named after the directory, each of
 * its expected size.
 */
class MCCheckDirExistsTask : public MemcardTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress PAL: 0x003bf938
     */
    ~MCCheckDirExistsTask() override = default;

    /**
     * Prepare the task.
     *
     * @param nPort The memory card slot.
     * @param pszDir The directory, or null for g_MemcardPath.
     * @param pUser The task MemcardSerialTask::OnDirChecked() tells, or null.
     * @ghidraAddress PAL: 0x001611a0
     */
    void Set(int nPort, const char *pszDir, MemcardSerialTask *pUser);

    String mDir;              /*!< The directory. */
    MCGetDirTask mGetDir;     /*!< The step that lists the directory. */
    MemcardSerialTask *mUser; /*!< The task told whether the directory is whole, or null. */

protected:
    /**
     * Start listing the directory.
     *
     * @ghidraAddress PAL: 0x00161268
     */
    void OnStart() override;

    /**
     * Once the listing ends, delete a damaged directory, finish, and tell mUser whether the
     * directory is whole.
     *
     * @ghidraAddress PAL: 0x00161cf8
     */
    void OnPoll() override;

private:
    /**
     * Compare the listed files with the files a save directory needs.
     *
     * An empty listing is neither damaged nor whole.
     *
     * @param pbDamaged Receives non-zero when the directory has files but lacks a needed one.
     * @param pbWhole Receives non-zero when the directory has every needed file.
     * @ghidraAddress PAL: 0x00161288
     */
    void CheckFiles(int *pbDamaged, int *pbWhole);
};

#endif
