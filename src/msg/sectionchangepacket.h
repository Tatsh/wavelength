#pragma once

#include "msg/externalpacket.h"
#include "stream/ibstream.h"
#include "stream/obstream.h"

/**
 * Packet that tells the other consoles of a remix session about a section change.
 *
 * The RTTI includes the class name and records ExternalPacket as the base. The payload travels as
 * one four-byte word, the section in the top four bits and the tick in the other 28.
 */
class SectionChangePacket : public ExternalPacket {
public:
    /**
     * Write the payload.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00139058
     * @ghidraAddress PAL: 0x0013a8b8
     */
    void saveGuts(OBStream &stream) const override;

    /**
     * Read the payload.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x001390d0
     * @ghidraAddress PAL: 0x0013a930
     */
    void restoreGuts(IBStream &stream) override;

    unsigned int mSection; /*!< The section. Four bits travel. */
    unsigned int mTick;    /*!< The tick of the change. 28 bits travel. */
};
