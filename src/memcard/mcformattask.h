#pragma once

#include "memcard/memcardtask.h"

/**
 * Task that formats a card.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x10 bytes.
 */
class MCFormatTask : public MemcardTask {
public:
    /**
     * Prepare the task.
     *
     * @param nPort The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015e018
     * @ghidraAddress PAL: 0x0015f858
     */
    void Set(int nPort);

    /**
     * Record the outcome and finish.
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x0015e038
     * @ghidraAddress PAL: 0x0015f878
     */
    void OnFormat(int nResult) override;

protected:
    /**
     * Issue the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015e088
     */
    void OnStart() override;
};
