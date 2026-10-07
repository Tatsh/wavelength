#pragma once

#include "game/globalsettings.h"
#include "memcard/mcchecksavespaceneededtask.h"
#include "memcard/mccreatesavedirtask.h"
#include "memcard/mcsavefiletask.h"
#include "memcard/memcardserialtask.h"
#include "memcard/savespaceuser.h"
#include "os/string.h"

/**
 * Work that saves the GlobalSettings to the `settings` file of the Freq directory.
 *
 * The RTTI records the class as deriving from MemcardSerialTask and from SaveSpaceUser at `+0x20`.
 * The object is 0x50 bytes. The steps measure the space, create the directory, and save the file.
 * A save that does not fit fails after the measurement, and GetNeeded() then reports the space it
 * needed.
 */
class MCSaveSettingsTask : public MemcardSerialTask, public SaveSpaceUser {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x00162000
     * @ghidraAddress PAL: 0x00164d10
     */
    MCSaveSettingsTask();

    /**
     * Prepare the work, and serialise the settings into g_pMemcardBuffer.
     *
     * @param nPort The memory card slot.
     * @param pSettings The settings.
     * @param nOverwrite Non-zero when saved settings may be replaced.
     * @ghidraAddress NTSC-U/C: 0x001620c0
     * @ghidraAddress PAL: 0x00164dd0
     */
    void Set(int nPort, GlobalSettings *pSettings, int nOverwrite);

    /**
     * Report the space a save that did not fit needed.
     *
     * @return The kilobytes, or zero.
     * @ghidraAddress NTSC-U/C: 0x00162188
     * @ghidraAddress PAL: 0x00164e98
     */
    int GetNeeded();

    /**
     * Prepare the directory and the file steps when the save fits, and otherwise fail the
     * measurement.
     *
     * @param nNeeded The kilobytes the save needs.
     * @ghidraAddress NTSC-U/C: 0x00162190
     * @ghidraAddress PAL: 0x00164ea0
     */
    void OnSaveSpace(int nNeeded) override;

    MCCheckSaveSpaceNeededTask *mCheck; /*!< The work that measures the space. */
    MCCreateSaveDirTask *mCreateDir;    /*!< The step that creates the directory. */
    MCSaveFileTask *mSaveFile;          /*!< The work that saves the file. */
    int mNeeded;                        /*!< The value GetNeeded() reports. */
    int mPort;                          /*!< The memory card slot. */
    int mSize;                          /*!< The bytes serialised into g_pMemcardBuffer. */
    String mName;                       /*!< The name of the file, `settings`. */
};
