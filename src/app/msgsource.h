#pragma once

#include <list>

class Message;
class MsgSink;

/**
 * Sender of engine messages, which stores the list of sinks that receive them.
 *
 * The RTTI includes the class name and records no base. The data members precede the vptr, which
 * sits at `+0x0c`. A sink removed while a message is being delivered is first cleared, and the
 * delivery loop removes the cleared entry.
 */
class MsgSource {
public:
    /**
     * Construct a source with no sinks.
     *
     * @ghidraAddress NTSC-U/C: 0x0029d150
     * @ghidraAddress PAL: 0x002a6e18
     */
    MsgSource();

    /**
     * Release the list of sinks.
     *
     * @ghidraAddress NTSC-U/C: 0x003ab3d0
     * @ghidraAddress PAL: 0x0041a130
     */
    virtual ~MsgSource();

    /**
     * Register a sink, ignoring a sink that is already registered.
     *
     * @param pSink The sink to register.
     * @ghidraAddress NTSC-U/C: 0x0029d1d8
     * @ghidraAddress PAL: 0x002a6ea0
     */
    virtual void AddSink(MsgSink *pSink);

    /**
     * Unregister the first occurrence of a sink.
     *
     * During a delivery the entry is cleared instead. A sink that is not registered is ignored.
     *
     * @param pSink The sink to unregister.
     * @ghidraAddress NTSC-U/C: 0x0029d2a8
     * @ghidraAddress PAL: 0x002a6f70
     */
    virtual void RemoveSink(MsgSink *pSink);

    /**
     * Deliver a message to every registered sink, in registration order.
     *
     * @param pMsg The message to deliver.
     * @ghidraAddress NTSC-U/C: 0x0029d338
     * @ghidraAddress PAL: 0x002a7000
     */
    void Send(Message *pMsg);

    /**
     * Deliver a message to the registered sinks in registration order until one handles it.
     *
     * @param pMsg The message to deliver.
     * @return Whether a sink handled the message.
     * @ghidraAddress NTSC-U/C: 0x0029d420
     * @ghidraAddress PAL: 0x002a70e8
     */
    bool SendUntilHandled(Message *pMsg);

private:
    int mDispatchDepth;          // The deliveries in progress.
    std::list<MsgSink *> mSinks; // The sinks, with null entries removed by the next delivery.
};
