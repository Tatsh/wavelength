#pragma once

#include "memcard/mcgetdirtask.h"
#include "memcard/mcgetinfotask.h"
#include "memcard/memcardserialtask.h"
#include "memcard/savespaceuser.h"
#include "os/string.h"

/** The listings MCCheckSaveSpaceNeededTask makes: the Freq directory, then its files. */
constexpr int kSaveSpaceListingCount = 2;

/**
 * Work that measures the space a save to the Freq directory needs, and checks it against the free
 * space of the card.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x58 bytes. When the
 * work ends, OnStop() reports the outcome to mUser. The outcome is MemcardTask::kStatusLimit when
 * the directory includes mMaxFiles files of the kind already, MemcardTask::kStatusExists when the
 * file exists and may not be replaced, and MemcardTask::kStatusFull when the card lacks the space.
 */
class MCCheckSaveSpaceNeededTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x0015ea48
     */
    MCCheckSaveSpaceNeededTask();

    /**
     * Prepare the work, and make g_MemcardPath the Freq directory.
     *
     * @param pUser The receiver of the outcome.
     * @param nPort The memory card slot.
     * @param pszName The name of the file, without its extension.
     * @param pszExt The extension of the file.
     * @param nMaxFiles The files of the extension the directory may include.
     * @param nFileSize The size of the file in bytes.
     * @param nOverwrite Non-zero when an existing file may be replaced.
     * @ghidraAddress NTSC-U/C: 0x0015eba0
     * @ghidraAddress PAL: 0x00160490
     */
    void Set(SaveSpaceUser *pUser,
             int nPort,
             const char *pszName,
             const char *pszExt,
             int nMaxFiles,
             int nFileSize,
             int nOverwrite);

    /**
     * Record whether the directory exists, then measure the space once the files are listed.
     *
     * @ghidraAddress NTSC-U/C: 0x0015ece8
     * @ghidraAddress PAL: 0x001605f0
     */
    void OnDirListed() override;

    MCGetInfoTask *mGetInfo; /*!< The step that reads the card information. */
    MCGetDirTask *mGetDirs[kSaveSpaceListingCount]; /*!< The directory, then its files. */
    SaveSpaceUser *mUser;                           /*!< The receiver of the outcome. */
    int mNeeded;                                    /*!< The kilobytes the save needs. */
    String mFileName;                               /*!< The file, with its extension. */
    int mFileSize;                                  /*!< The size of the file in bytes. */
    int mOverwrite;         /*!< Non-zero when an existing file may be replaced. */
    unsigned char mListing; /*!< The listing that ends next. */
    int mMaxFiles;          /*!< The files of the extension the directory may include. */

protected:
    /**
     * End with the outcome, and report it to mUser.
     *
     * @ghidraAddress NTSC-U/C: 0x0015ee98
     */
    void OnStop() override;
};
