#pragma once

#include <list>

#include "memcard/mcgetinfotask.h"
#include "memcard/mcnetconfigtask.h"
#include "memcard/memcardserialtask.h"
#include "netflow/inetconfig.h"

/**
 * Work that lists the network configurations: read the card information, then request the
 * configurations from the network layer.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x28 bytes. An
 * unformatted card fails with MemcardTask::kStatusUnformatted.
 */
class MCListNetConfigsTask : public MemcardSerialTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00353bb0
     * @ghidraAddress PAL: 0x003c0eb8
     */
    ~MCListNetConfigsTask() override = default;

    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x00162588
     * @ghidraAddress PAL: 0x001652a8
     */
    MCListNetConfigsTask();

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x00162630
     * @ghidraAddress PAL: 0x00165350
     */
    void Set(int nPort);

    /**
     * Fail the information step when the card read succeeded but the card is not formatted.
     *
     * @ghidraAddress NTSC-U/C: 0x00162688
     * @ghidraAddress PAL: 0x001653a8
     */
    void OnCardInfo() override;

    /**
     * Report the configurations found.
     *
     * @return The configurations.
     * @ghidraAddress NTSC-U/C: 0x001626c8
     * @ghidraAddress PAL: 0x001653e8
     */
    std::list<InetConfig> *GetConfigs();

private:
    // The steps belong to the SerialTasks list.
    MCGetInfoTask *mGetInfo;     /*!< The step that reads the card information. Borrowed. */
    MCNetConfigTask *mNetConfig; /*!< The step that requests the configurations. Borrowed. */
};
