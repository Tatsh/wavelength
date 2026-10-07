#pragma once

#include "memcard/memcardserialtask.h"
#include "memcard/memcardtask.h"

/**
 * Task that reads the type, the free space, and the format of a card.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x24 bytes.
 */
class MCGetInfoTask : public MemcardTask {
public:
    /**
     * Prepare the task.
     *
     * @param pOwner The task that learns when this one ends through OnCardInfo(), or null.
     * @param nPort The memory card slot.
     * @param nAcceptNewCard Non-zero to count another card than before as success.
     * @ghidraAddress NTSC-U/C: 0x0015d8a0
     * @ghidraAddress PAL: 0x0015f0b8
     */
    void Set(MemcardSerialTask *pOwner, int nPort, int nAcceptNewCard);

    /**
     * Record the outcome, finish, and tell mOwner.
     *
     * An unformatted card counts as another card than before.
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x0015d8e8
     * @ghidraAddress PAL: 0x0015f100
     */
    void OnGetInfo(int nResult) override;

    int mFree;                 /*!< The free space in kilobytes. */
    int mType;                 /*!< The card type. */
    int mFormat;               /*!< Non-zero for a formatted card. */
    int mAcceptNewCard;        /*!< Non-zero when another card than before counts as success. */
    MemcardSerialTask *mOwner; /*!< The task OnGetInfo() tells, or null. */

protected:
    /**
     * Issue the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015d988
     * @ghidraAddress PAL: 0x0015f1a0
     */
    void OnStart() override;
};
