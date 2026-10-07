#pragma once

#include "game/playerprofile.h"
#include "memcard/mcchecksavespaceneededtask.h"
#include "memcard/mccreatesavedirtask.h"
#include "memcard/mcdeletetask.h"
#include "memcard/mcgetinfotask.h"
#include "memcard/mcsavefiletask.h"
#include "memcard/memcardserialtask.h"
#include "memcard/savespaceuser.h"
#include "os/string.h"

/**
 * Work that saves the Freq of a profile to the Freq directory, under the profile's name.
 *
 * The RTTI records the class as deriving from MemcardSerialTask and from SaveSpaceUser at `+0x20`.
 * The object is 0x5c bytes. The steps read the card information, delete the file of the Freq's
 * previous name, measure the space, create the directory, and save the file. A save that does not
 * fit fails after the measurement, and GetNeeded() then reports the space it needed.
 */
class MCSaveFreqTask : public MemcardSerialTask, public SaveSpaceUser {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x001600b8
     * @ghidraAddress PAL: 0x00162918
     */
    MCSaveFreqTask();

    /**
     * Prepare the work, and serialise the profile into g_pMemcardBuffer.
     *
     * The lowered name of the profile is the file name. When the previous name differs from it,
     * ignoring case, the file of the previous name is deleted.
     *
     * @param nPort The memory card slot.
     * @param pProfile The profile.
     * @param pszOldName The name the Freq was saved under before.
     * @param nOverwrite Non-zero when a saved Freq of the name may be replaced.
     * @ghidraAddress NTSC-U/C: 0x00160220
     * @ghidraAddress PAL: 0x00162a80
     */
    void Set(int nPort, PlayerProfile *pProfile, const char *pszOldName, int nOverwrite);

    /**
     * Report the space a save that did not fit needed.
     *
     * @return The kilobytes, or zero.
     * @ghidraAddress NTSC-U/C: 0x00160428
     * @ghidraAddress PAL: 0x00162c88
     */
    int GetNeeded();

    /**
     * Prepare the directory and the file steps when the save fits, and otherwise fail the
     * measurement.
     *
     * @param nNeeded The kilobytes the save needs.
     * @ghidraAddress NTSC-U/C: 0x00160430
     * @ghidraAddress PAL: 0x00162c90
     */
    void OnSaveSpace(int nNeeded) override;

    MCGetInfoTask *mGetInfo;            /*!< The step that reads the card information. */
    MCCheckSaveSpaceNeededTask *mCheck; /*!< The work that measures the space. */
    MCCreateSaveDirTask *mCreateDir;    /*!< The step that creates the directory. */
    MCSaveFileTask *mSaveFile;          /*!< The work that saves the file. */
    MCDeleteTask *mDelete;              /*!< The step that deletes the previous file. */
    int mNeeded;                        /*!< The value GetNeeded() reports. */
    int mPort;                          /*!< The memory card slot. */
    String mName;                       /*!< The Freq's name, lowered. */
    int mReserved54;                    // +0x54, never written or read.
    int mFileSize;                      /*!< The size of a Freq file, `max_freq_size`. */
};
