#pragma once

#include "memcard/mcgetinfotask.h"
#include "memcard/mcunformattask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that unformats a card: read the card information, then unformat.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x28 bytes. The
 * game never prepares this work, and MCManager only constructs it and reports its end.
 */
class MCUnformatCardTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x0015e278
     * @ghidraAddress PAL: 0x0015fab8
     */
    MCUnformatCardTask();

    MCGetInfoTask *mGetInfo;   /*!< The step that reads the card information. */
    MCUnformatTask *mUnformat; /*!< The step that unformats the card. */
};
