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
     * @ghidraAddress PAL: 0x00152dd8
     */
    explicit ChangeSectionEvent(BinStream &stream);

    /**
     * Record the player in one byte and the direction in four bytes of the console's byte order.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00151588
     * @ghidraAddress PAL: 0x00152e00
     */
    void Save(BinStream &stream) const;

    /**
     * Read the event Save() records.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x001515f8
     * @ghidraAddress PAL: 0x00152e58
     */
    void Load(BinStream &stream);

    /** @return The player index. */
    unsigned char GetPlayer() const {
        return mPlayer;
    }

    /** @param nPlayer The player index. */
    void SetPlayer(unsigned char nPlayer) {
        mPlayer = nPlayer;
    }

    /** @return The direction of the change, 0 or 1. */
    int GetDirection() const {
        return mDirection;
    }

    /** @param nDirection The direction of the change, 0 or 1. */
    void SetDirection(int nDirection) {
        mDirection = nDirection;
    }

private:
    unsigned char mPlayer; /*!< The player index. */
    int mDirection;        /*!< The direction of the change, 0 or 1. */
};
