#pragma once

#include "memcard/mcgetinfotask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that reads the card information to learn whether a usable card is in a slot.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x24 bytes.
 */
class MCCardStatusTask : public MemcardSerialTask {
public:
    /**
     * Construct the step.
     *
     * @ghidraAddress NTSC-U/C: 0x0015e9a8
     * @ghidraAddress PAL: 0x001601e8
     */
    MCCardStatusTask();

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015ea20
     * @ghidraAddress PAL: 0x00160260
     */
    void Set(int nPort);

    MCGetInfoTask *mGetInfo; /*!< The step that reads the card information. */
};
