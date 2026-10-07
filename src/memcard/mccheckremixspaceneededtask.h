#pragma once

#include <vector>

#include "memcard/mcgetdirtask.h"
#include "memcard/mcgetinfotask.h"
#include "memcard/memcardserialtask.h"
#include "memcard/savespaceuser.h"
#include "os/string.h"

/**
 * Work that chooses the remix directory a remix is saved to, measures the space the save needs,
 * and checks it against the free space of the card.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x64 bytes. The
 * directories are listed in order. A directory that includes the file is chosen; otherwise the
 * first directory with room is, and g_MemcardPath records it. When the work ends, OnStop() reports
 * the outcome to mUser. The outcome is MemcardTask::kStatusLimit when the card includes
 * g_nMaxRemixes remixes already, MemcardTask::kStatusExists when the file exists and may not be
 * replaced, and MemcardTask::kStatusFull when the card lacks the space.
 */
class MCCheckRemixSpaceNeededTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps, one listing for each remix directory.
     *
     * @ghidraAddress NTSC-U/C: 0x0015ef00
     */
    MCCheckRemixSpaceNeededTask();

    /**
     * Prepare the work, and clear g_MemcardPath.
     *
     * @param pUser The receiver of the outcome.
     * @param nPort The memory card slot.
     * @param pszName The name of the remix file, without its extension.
     * @param nFileSize The size of the file in bytes.
     * @param nOverwrite Non-zero when an existing file may be replaced.
     * @ghidraAddress NTSC-U/C: 0x0015f0e0
     */
    void Set(SaveSpaceUser *pUser, int nPort, const char *pszName, int nFileSize, int nOverwrite);

    /**
     * Search one listed directory for the file, and measure the space once every directory is
     * listed.
     *
     * @ghidraAddress NTSC-U/C: 0x0015f278
     */
    void OnDirListed() override;

    MCGetInfoTask *mGetInfo;              /*!< The step that reads the card information. */
    std::vector<MCGetDirTask *> mGetDirs; /*!< One listing step for each remix directory. */
    SaveSpaceUser *mUser;                 /*!< The receiver of the outcome. */
    String mFileName;                     /*!< The remix file, with its extension. */
    int mFileSize;                        /*!< The size of the file in bytes. */
    int mNeeded;                          /*!< The kilobytes the save needs. */
    unsigned char mRemixCount;            /*!< The remix files in the listed directories. */
    unsigned char mDirIndex;              /*!< The directory whose listing ends next. */
    int mFound;                           /*!< Non-zero once a directory includes the file. */
    int mFoundSize;                       /*!< The size of the file found, in bytes. */
    int mOverwrite;                       /*!< Non-zero when an existing file may be replaced. */

protected:
    /**
     * End with the outcome, and report it to mUser.
     *
     * @ghidraAddress NTSC-U/C: 0x0015f4a8
     */
    void OnStop() override;
};
