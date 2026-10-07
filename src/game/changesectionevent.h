#pragma once

/**
 * Input event that changes the section a player is on.
 *
 * The RTTI includes the name as the argument of the input command template.
 */
struct ChangeSectionEvent {
    unsigned char mPlayer; /*!< The player index. */
    int mDirection;        /*!< The direction of the change, 0 or 1. */
};
