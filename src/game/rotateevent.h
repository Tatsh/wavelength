#pragma once

/**
 * Input event that rotates a player's view of the tracks by one track.
 *
 * The RTTI includes the name as the argument of the input command template. The input router
 * builds one for each rotation and for each repeat of a held rotation.
 */
struct RotateEvent {
    /** Values of mDirection. */
    enum Direction {
        kDirectionPrevious = 0, /*!< The previous track. */
        kDirectionNext = 1,     /*!< The next track. */
    };

    unsigned char mPlayer; /*!< The player index. */
    int mDirection;        /*!< One of Direction. */
};
