#pragma once

#include "msg/packet.h"

/**
 * Packet that travels between consoles rather than inside one.
 *
 * The RTTI includes the class name and records Packet as the base. The class adds one byte
 * member, at `+0x0c`, that this header does not yet declare.
 */
class ExternalPacket : public Packet {};
