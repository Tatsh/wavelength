#pragma once

#include "memcard/mcformattask.h"
#include "memcard/mcgetinfotask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that formats an unformatted card: read the card information, then format.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x28 bytes. A card
 * that is formatted already fails with MemcardTask::kStatusFormatted.
 */
class MCFormatCardTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x0015e138
     * @ghidraAddress PAL: 0x0015f978
     */
    MCFormatCardTask();

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015e220
     * @ghidraAddress PAL: 0x0015fa60
     */
    void Set(int nPort);

    /**
     * Fail the information step when the card is formatted already.
     *
     * @ghidraAddress NTSC-U/C: 0x0015e1e8
     * @ghidraAddress PAL: 0x0015fa28
     */
    void OnCardInfo() override;

    MCGetInfoTask *mGetInfo; /*!< The step that reads the card information. */
    MCFormatTask *mFormat;   /*!< The step that formats the card. */
};
