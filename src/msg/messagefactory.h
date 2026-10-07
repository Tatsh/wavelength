#pragma once

#include "msg/message.h"

/** Produces one default-constructed message of a registered class. */
typedef Message *(*MessageFactoryProc)();

/**
 * Registrar that adds one message class to the factory list a received message is built from.
 *
 * Each registration is a file-scope object whose constructor passes the identity Message::Type()
 * reports and the class's static New(). The class is not polymorphic, emits no RTTI descriptor,
 * and writes no member. The name is inferred.
 */
class MessageFactory {
public:
    /**
     * Insert a factory into the list, ordered by identity.
     *
     * @param nType The identity the class streams itself under.
     * @param pfnCreate The factory for the class.
     * @ghidraAddress NTSC-U/C: 0x0029ccd0
     * @ghidraAddress PAL: 0x002a6998
     */
    MessageFactory(int nType, MessageFactoryProc pfnCreate);
};
