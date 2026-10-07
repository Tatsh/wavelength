#pragma once

#include "memcard/memcardtask.h"

/**
 * Task that closes the file MemcardTask::sFd identifies.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x10 bytes.
 */
class MCCloseTask : public MemcardTask {
public:
    /**
     * Record the outcome and finish.
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x0015dcb0
     * @ghidraAddress PAL: 0x0015f4f0
     */
    void OnClose(int nResult) override;

protected:
    /**
     * Issue the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015dd00
     * @ghidraAddress PAL: 0x0015f540
     */
    void OnStart() override;
};
