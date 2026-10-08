#pragma once

#include "game/gameoptions.h"
#include "memcard/mcgetinfotask.h"
#include "memcard/mcloadfiletask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that loads the GameOptions from the `settings` file of the directory at g_MemcardPath:
 * read the card information, then load the file.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x2c bytes.
 */
class MCLoadSettingsTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x00162260
     * @ghidraAddress PAL: 0x00164f70
     */
    MCLoadSettingsTask();

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @param pSettings Receives the settings.
     * @ghidraAddress NTSC-U/C: 0x001622f8
     * @ghidraAddress PAL: 0x00165008
     */
    void Set(int nPort, GameOptions *pSettings);

    /**
     * Decode the loaded file into mSettings.
     *
     * @ghidraAddress NTSC-U/C: 0x00162398
     */
    void OnFileLoaded() override;

    MCGetInfoTask *mGetInfo; /*!< The step that reads the card information. */
    MCLoadFileTask *mLoad;   /*!< The work that loads the file. */
    GameOptions *mSettings;  /*!< Receives the settings. */
};
