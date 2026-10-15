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
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00351f88
     * @ghidraAddress PAL: 0x003bf208
     */
    ~MCUnformatTask() override = default;

    /**
     * Record the outcome and finish.
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x0015e0b0
     * @ghidraAddress PAL: 0x0015f8f0
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
