#pragma once

#include <list>

#include "utl/MsgSink.h"

/**
 * Sender of messages to a list of sinks.
 *
 * The RTTI records the class. The object is 0xc bytes. The name of the flag at `+0x04` follows the
 * game's MsgSource; its meaning here is inferred from RemoveSink().
 */
class MsgSource {
public:
    /**
     * Create a source with no sinks.
     *
     * @ghidraAddress 0x100199e0
     */
    MsgSource();

    /**
     * Free the list of sinks. The sinks are not destroyed.
     *
     * @ghidraAddress 0x1000eb50
     */
    virtual ~MsgSource();

    /**
     * Add a sink at the end of the list, unless it is already there.
     *
     * @param sink The sink.
     * @ghidraAddress 0x10019a60
     */
    virtual void AddSink(MsgSink *sink);

    /**
     * Remove a sink, which must be in the list. While mGraphBuilt is set, its entry is cleared
     * instead.
     *
     * @param sink The sink.
     * @ghidraAddress 0x10019ae0
     */
    virtual void RemoveSink(MsgSink *sink);

    int mGraphBuilt; /*!< While set, RemoveSink() clears entries instead of erasing them. */

private:
    std::list<MsgSink *> mSinks; /*!< The sinks, in the order they were added. */
};
