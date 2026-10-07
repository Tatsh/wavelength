#pragma once

/**
 * Input event of an analog stick position.
 *
 * The RTTI includes the template with the arguments 2 and 6. The argument is the identifier of
 * the serializable input command that records the event, and it makes each instance a distinct
 * type.
 *
 * @tparam kId The command identifier.
 */
template <int kId>
struct StickEvent {
    unsigned char mPlayer; /*!< The player index. */
    float mX;              /*!< The horizontal position. */
    float mY;              /*!< The vertical position. */
};
