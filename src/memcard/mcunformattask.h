#pragma once

#include "memcard/memcardtask.h"

/**
 * Task that unformats a card.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x10 bytes.
 */
class MCUnformatTask : public MemcardTask {
public:
    /**
     * Record the outcome and finish.
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x0015e0b0
     * @ghidraAddress PAL: 0x0015f8c8
     */
    void OnUnformat(int nResult) override;

protected:
    /**
     * Issue the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015e100
     * @ghidraAddress PAL: 0x0015f940
     */
    void OnStart() override;
};
