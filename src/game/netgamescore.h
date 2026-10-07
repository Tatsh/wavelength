#pragma once

/**
 * The final score of one player of an online session.
 *
 * The RTTI includes the class name. The structure is not polymorphic.
 */
struct NetGameScore {
    int mNetOrder; /*!< The player's position in the order of the session. */
    int mScore;    /*!< The player's score. */
};
