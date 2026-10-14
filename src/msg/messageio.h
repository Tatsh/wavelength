#pragma once

#include "msg/message.h"
#include "os/binstream.h"

/**
 * Write a message as its two-byte identity followed by its payload.
 *
 * Type() is dispatched twice with the first result discarded.
 *
 * @param stream The stream to write to.
 * @param pMsg The message.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0029d030
 * @ghidraAddress PAL: 0x002a6cf8
 */
BinStream &operator<<(BinStream &stream, Message *pMsg);

/**
 * Read a message the writer wrote, constructing it from the factory list.
 *
 * An unregistered identity is reported through DebugPrint() and leaves the result null.
 *
 * @param stream The stream to read from.
 * @param pMsg Receives the new message, or null.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0029d0b8
 * @ghidraAddress PAL: 0x002a6d80
 */
BinStream &operator>>(BinStream &stream, Message *&pMsg);
