#pragma once

/**
 * A message a MsgSource sends to its sinks.
 *
 * The control never constructs a message, so no vtable or RTTI of one is in it. Only the third
 * virtual function, which reports the type, is called; the first two are inferred.
 */
class Message {
public:
    virtual ~Message() {
    }

    /** Not called by the control. */
    virtual void Unused1() {
    }

    /** @return The type of the message. */
    virtual int Type() = 0;
};
