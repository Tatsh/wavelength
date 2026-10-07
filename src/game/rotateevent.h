#pragma once

#include "os/binstream.h"

/**
 * Input event that rotates a player's view of the tracks by one track.
 *
 * The RTTI includes the name as the argument of the input command template. The input router
 * builds one for each rotation and for each repeat of a held rotation.
 */
class RotateEvent {
public:
    /** Values of mDirection. */
    enum Direction {
        kDirectionPrevious = 0, /*!< The previous track. */
        kDirectionNext = 1,     /*!< The next track. */
    };

    /**
     * Construct an event with unset members.
     *
     * Inline.
     */
    RotateEvent() = default;

    /**
     * Construct the event from a recording.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00151460
     */
    explicit RotateEvent(BinStream &stream);

    /**
     * Record the player in one byte and the direction in four bytes of the console's byte order.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00151488
     */
    void Save(BinStream &stream) const;

    /**
     * Read the event Save() records.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x001514f8
     */
    void Load(BinStream &stream);

    unsigned char mPlayer; /*!< The player index. */
    int mDirection;        /*!< One of Direction. */
};
