#pragma once

#include "memcard/mcdeletetask.h"
#include "memcard/mcgetinfotask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that deletes a saved Freq: read the card information, then delete the file.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x28 bytes.
 */
class MCDeleteFreqTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x00160a38
     * @ghidraAddress PAL: 0x001633c8
     */
    MCDeleteFreqTask();

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @param pszName The Freq's name. Its lowered form is the file name.
     * @ghidraAddress NTSC-U/C: 0x00160b00
     * @ghidraAddress PAL: 0x00163490
     */
    void Set(int nPort, const char *pszName);

    MCGetInfoTask *mGetInfo; /*!< The step that reads the card information. */
    MCDeleteTask *mDelete;   /*!< The step that deletes the file. */
};
