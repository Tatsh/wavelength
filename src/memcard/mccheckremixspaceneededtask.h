#pragma once

#include <vector>

#ifdef VIDEO_STANDARD_PAL
#include "memcard/mccheckdirexiststask.h"
#endif
#include "memcard/mcgetdirtask.h"
#include "memcard/mcgetinfotask.h"
#include "memcard/memcardserialtask.h"
#include "memcard/savespaceuser.h"
#include "os/string.h"

/**
 * Work that chooses the remix directory a remix is saved to, measures the space the save needs,
 * and checks it against the free space of the card.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x64 bytes, and 0x84
 * in the PAL build. The PAL build first checks every directory with an MCCheckDirExistsTask. The
 * directories are listed in order. A directory that includes the file is chosen; otherwise the
 * first directory with room is, and g_MemcardPath records it. When the work ends, OnStop() reports
 * the outcome to mUser. The outcome is MemcardTask::kStatusLimit when the card includes
 * g_nMaxRemixes remixes already, MemcardTask::kStatusExists when the file exists and may not be
 * replaced, and MemcardTask::kStatusFull when the card lacks the space.
 */
class MCCheckRemixSpaceNeededTask : public MemcardSerialTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00352548
     * @ghidraAddress PAL: 0x003bf7c8
     */
    ~MCCheckRemixSpaceNeededTask() override = default;

    /**
     * Construct the steps, one listing for each remix directory.
     *
     * @ghidraAddress NTSC-U/C: 0x0015ef00
     * @ghidraAddress PAL: 0x00160818
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
     * @ghidraAddress PAL: 0x00160ba0
     */
    void Set(SaveSpaceUser *pUser, int nPort, const char *pszName, int nFileSize, int nOverwrite);

    /**
     * Search one listed directory for the file, and measure the space once every directory is
     * listed.
     *
     * @ghidraAddress NTSC-U/C: 0x0015f278
     * @ghidraAddress PAL: 0x00160df8
     */
    void OnDirListed() override;

#ifdef VIDEO_STANDARD_PAL
    /**
     * Learn whether the next checked remix directory is whole, and mark it as existing when it is.
     *
     * @param bWhole Non-zero when the directory has every file a save directory needs.
     * @ghidraAddress PAL: 0x00160da8
     */
    void OnDirChecked(int bWhole) override;
#endif

protected:
    /**
     * End with the outcome, and report it to mUser.
     *
     * @ghidraAddress NTSC-U/C: 0x0015f4a8
     * @ghidraAddress PAL: 0x00161138
     */
    void OnStop() override;

private:
#ifdef VIDEO_STANDARD_PAL
    /**
     * Choose the directory, measure the space the save needs, and record the outcome.
     *
     * The name is inferred. The first existing directory with room is chosen, else the first
     * directory that does not exist.
     *
     * @ghidraAddress PAL: 0x00160f40
     */
    void Evaluate();
#endif

    // The steps belong to the SerialTasks list.
    MCGetInfoTask *mGetInfo; /*!< The step that reads the card information. Borrowed. */
#ifdef VIDEO_STANDARD_PAL
    /** One step for each remix directory that deletes the directory when damaged. */
    std::vector<MCCheckDirExistsTask *> mCheckDirs;
#endif
    std::vector<MCGetDirTask *> mGetDirs; /*!< Listing steps. Borrowed. */
#ifdef VIDEO_STANDARD_PAL
    /** The remix files of each remix directory, or -1 for a directory that does not exist. */
    std::vector<int> mDirRemixCounts;
#endif
    SaveSpaceUser *mUser; /*!< The receiver of the outcome. Borrowed. */
    String mFileName;     /*!< The remix file, with its extension. */
    int mFileSize;        /*!< The size of the file in bytes. */
    int mNeeded;          /*!< The kilobytes the save needs. */
#ifndef VIDEO_STANDARD_PAL
    unsigned char mRemixCount; /*!< The remix files in the listed directories. */
#endif
    unsigned char mDirIndex; /*!< The directory whose check or listing ends next. */
    int mFound;              /*!< Non-zero once a directory includes the file. */
    int mFoundSize;          /*!< The size of the file found, in bytes. */
    int mOverwrite;          /*!< Non-zero when an existing file may be replaced. */
};
