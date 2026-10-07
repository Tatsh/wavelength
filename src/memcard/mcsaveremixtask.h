#pragma once

#include "game/remixinfo.h"
#include "memcard/mccheckremixspaceneededtask.h"
#include "memcard/mccreatesavedirtask.h"
#include "memcard/mcsavefiletask.h"
#include "memcard/memcardserialtask.h"
#include "memcard/savespaceuser.h"
#include "os/string.h"

/**
 * Work that saves a remix with its RemixInfo to a remix directory.
 *
 * The RTTI records the class as deriving from MemcardSerialTask and from SaveSpaceUser at `+0x20`.
 * The object is 0x50 bytes. The steps choose the directory and measure the space, create the
 * directory, and save the file. A save that does not fit fails after the measurement, and
 * GetNeeded() then reports the space it needed.
 */
class MCSaveRemixTask : public MemcardSerialTask, public SaveSpaceUser {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x00161810
     * @ghidraAddress PAL: 0x00164520
     */
    MCSaveRemixTask();

    /**
     * Prepare the work, and serialise the description and the remix into g_pMemcardBuffer.
     *
     * @param nPort The memory card slot.
     * @param pszName The name of the remix file, without its extension.
     * @param pInfo The description.
     * @param pData The remix.
     * @param nSize The size of the remix in bytes.
     * @param nOverwrite Non-zero when a saved remix of the name may be replaced.
     * @ghidraAddress NTSC-U/C: 0x001618f0
     * @ghidraAddress PAL: 0x00164600
     */
    void Set(int nPort,
             const char *pszName,
             RemixInfo *pInfo,
             const void *pData,
             int nSize,
             int nOverwrite);

    /**
     * Report the space a save that did not fit needed.
     *
     * @return The kilobytes, or zero.
     * @ghidraAddress NTSC-U/C: 0x001619d8
     * @ghidraAddress PAL: 0x001646e8
     */
    int GetNeeded();

    /**
     * Prepare the directory and the file steps when the save fits, and otherwise fail the
     * measurement.
     *
     * The title of the directory is the `remix_label` and the last character of the directory's
     * name.
     *
     * @param nNeeded The kilobytes the save needs.
     * @ghidraAddress NTSC-U/C: 0x001619e0
     * @ghidraAddress PAL: 0x001646f0
     */
    void OnSaveSpace(int nNeeded) override;

    MCCheckRemixSpaceNeededTask *mCheck; /*!< The work that chooses the directory. */
    MCCreateSaveDirTask *mCreateDir;     /*!< The step that creates the directory. */
    MCSaveFileTask *mSaveFile;           /*!< The work that saves the file. */
    int mNeeded;                         /*!< The value GetNeeded() reports. */
    int mPort;                           /*!< The memory card slot. */
    int mSize;                           /*!< The bytes serialised into g_pMemcardBuffer. */
    String mName;                        /*!< The name of the remix file. */
};
