#pragma once

#include "app/msgsink.h"
#include "msg/message.h"
#include "netflow/nettask.h"
#include "os/serialtasks.h"

/**
 * A request to the online library, as the NetTask steps that run one after another.
 *
 * The RTTI includes the name and records SerialTasks as the base. The vtable is at `0x003d4820`.
 * The constructor has no out-of-line copy. It clears mSink and mResult.
 */
class NetMultiTask : public SerialTasks {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00398c30
     * @ghidraAddress PAL: 0x004074b8
     */
    ~NetMultiTask() override = default;

    /**
     * Construct a request with no steps.
     *
     * The constructor has no out-of-line copy.
     */
    NetMultiTask() : mSink(nullptr), mResult(0) {
    }

    /**
     * Append a step and make the request its parent.
     *
     * @param pTask The step.
     * @ghidraAddress NTSC-U/C: 0x0026a508
     * @ghidraAddress PAL: 0x00274098
     */
    void Add(NetTask *pTask);

    /**
     * Start the request.
     *
     * @param pSink The receiver of the messages of the request, or null.
     * @ghidraAddress NTSC-U/C: 0x0026a530
     * @ghidraAddress PAL: 0x002740c0
     */
    void Start(MsgSink *pSink);

    /**
     * Pass a message to mSink, when there is one.
     *
     * @param pMsg The message.
     * @ghidraAddress NTSC-U/C: 0x0026a550
     * @ghidraAddress PAL: 0x002740e0
     */
    void Notify(Message *pMsg);

    MsgSink *mSink; /*!< The receiver of the messages of the request, or null. */
    int mResult;    /*!< The result a failed step recorded, or 0. */
};
