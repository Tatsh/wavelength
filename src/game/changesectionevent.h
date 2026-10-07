#pragma once

#include "os/binstream.h"

/**
 * Input event that changes the section a player is on.
 *
 * The RTTI includes the name as the argument of the input command template.
 */
class ChangeSectionEvent {
public:
    /**
     * Construct an event with unset members.
     *
     * Inline.
     */
    ChangeSectionEvent() = default;

    /**
     * Construct the event from a recording.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00151560
     */
    explicit ChangeSectionEvent(BinStream &stream);

    /**
     * Record the player in one byte and the direction in four bytes of the console's byte order.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00151588
     */
    void Save(BinStream &stream) const;

    /**
     * Read the event Save() records.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x001515f8
     */
    void Load(BinStream &stream);

    unsigned char mPlayer; /*!< The player index. */
    int mDirection;        /*!< The direction of the change, 0 or 1. */
};
