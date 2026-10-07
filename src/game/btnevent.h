#pragma once

/**
 * Input event of a button press that names only the player.
 *
 * The RTTI includes the template with the arguments 3, 4, 5, 8, 9, and 10. The argument is the
 * identifier of the serializable input command that records the event, and it makes each
 * instance a distinct type.
 *
 * @tparam kId The command identifier.
 */
template <int kId>
struct BtnEvent {
    unsigned char mPlayer; /*!< The player index. */
};
