#pragma once

#include "os/serialtasks.h"

/**
 * Memory card work made of MemcardTask steps that run one after another.
 *
 * The RTTI records the class as deriving from SerialTasks. The object is 0x20 bytes. When the work
 * ends, OnStop() records MemcardTask::sStatus as the result GetResult() reports. The three
 * notification methods, four in the PAL build, do nothing here. A step calls one of them on the
 * task it was given, and a subclass overrides the one it needs to check the step's outcome.
 */
class MemcardSerialTask : public SerialTasks {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00351508
     * @ghidraAddress PAL: 0x003be780
     */
    ~MemcardSerialTask() override = default;

    /**
     * Report the outcome of the work.
     *
     * @return One of MemcardTask::Status.
     * @ghidraAddress NTSC-U/C: 0x0015e130
     * @ghidraAddress PAL: 0x0015f970
     */
    int GetResult();

    /**
     * Learn that a directory listing step ended.
     *
     * @ghidraAddress NTSC-U/C: 0x00351530
     * @ghidraAddress PAL: 0x003be7a8
     */
    virtual void OnDirListed() {
    }

    /**
     * Learn that a card information step ended.
     *
     * @ghidraAddress NTSC-U/C: 0x00351538
     * @ghidraAddress PAL: 0x003be7b0
     */
    virtual void OnCardInfo() {
    }

    /**
     * Learn that a file load step ended.
     *
     * @ghidraAddress NTSC-U/C: 0x00351540
     * @ghidraAddress PAL: 0x003be7b8
     */
    virtual void OnFileLoaded() {
    }

#ifdef VIDEO_STANDARD_PAL
    /**
     * Learn that an MCCheckDirExistsTask step ended.
     *
     * @param bWhole Non-zero when the directory has every file a save directory needs.
     * @ghidraAddress PAL: 0x003be7c0
     */
    virtual void OnDirChecked([[maybe_unused]] int bWhole) {
    }
#endif

    int mResult; /*!< The outcome, one of MemcardTask::Status. */

protected:
    /**
     * Record MemcardTask::sStatus in mResult.
     *
     * @ghidraAddress NTSC-U/C: 0x0015e120
     * @ghidraAddress PAL: 0x0015f960
     */
    void OnStop() override;
};
