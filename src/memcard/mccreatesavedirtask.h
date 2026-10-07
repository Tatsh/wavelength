#pragma once

#include <vector>

#include "memcard/mcmkdirtask.h"
#include "memcard/mcsavefiletask.h"
#include "memcard/memcardtask.h"
#include "os/string.h"

/**
 * Task that creates the save directory at g_MemcardPath, with its icon, its `icon.sys`, and the
 * file named after it.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x74 bytes. When
 * g_bMemcardDirExists is set the task finishes at once. A failure to create the directory is
 * ignored, and the files are saved regardless.
 */
class MCCreateSaveDirTask : public MemcardTask {
public:
    /** One file the task saves. */
    struct FileData {
        String mName;      /*!< The path of the file. */
        const void *mData; /*!< The bytes. The task never releases them. */
        int mSize;         /*!< The number of bytes. */
    };

    /**
     * Construct an idle task.
     *
     * @ghidraAddress NTSC-U/C: 0x0015f510
     * @ghidraAddress PAL: 0x00161d90
     */
    MCCreateSaveDirTask();

    /**
     * Prepare the task, and write the title the card's browser shows into g_MemcardIconSys.
     *
     * @param nPort The memory card slot.
     * @param pszTitle The title, in ASCII.
     * @ghidraAddress NTSC-U/C: 0x0015f5b0
     * @ghidraAddress PAL: 0x00161e30
     */
    void Set(int nPort, const char *pszTitle);

    MCMkDirTask mMkDir;           /*!< The step that creates the directory. */
    MCSaveFileTask mSaveFile;     /*!< The step that saves one file. */
    std::vector<FileData> mFiles; /*!< The files to save. */
    int mFileIndex;               /*!< The index into mFiles of the file being saved. */

protected:
    /**
     * List the files and start creating the directory.
     *
     * @ghidraAddress NTSC-U/C: 0x0015f5f0
     * @ghidraAddress PAL: 0x00161e70
     */
    void OnStart() override;

    /**
     * Advance the step running now.
     *
     * @ghidraAddress NTSC-U/C: 0x0015fe00
     */
    void OnPoll() override;

private:
    /**
     * Start saving the file at mFileIndex, or finish once every file is saved.
     *
     * @ghidraAddress NTSC-U/C: 0x0015fd60
     * @ghidraAddress PAL: 0x001625e0
     */
    void SaveNextFile();
};
