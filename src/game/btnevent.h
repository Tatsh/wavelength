#pragma once

#include "os/binstream.h"

/**
 * Input event of a button press that names only the player.
 *
 * The RTTI includes the template with the arguments 3, 4, 5, 8, 9, and 10. The argument is the
 * identifier of the serializable input command that records the event, and it makes each
 * instance a distinct type. Every member is inline.
 *
 * @tparam kId The command identifier.
 */
template <int kId>
class BtnEvent {
public:
    /** Construct an event with an unset player. */
    BtnEvent() = default;

    /**
     * Construct the event from a recording.
     *
     * @param stream The stream to read from.
     */
    explicit BtnEvent(BinStream &stream) {
        Load(stream);
    }

    /**
     * Record the player in one byte.
     *
     * @param stream The stream to write to.
     */
    void Save(BinStream &stream) const {
        const unsigned char nPlayer = mPlayer;
        stream.Write(&nPlayer, sizeof(nPlayer));
    }

    /**
     * Read the event Save() records.
     *
     * @param stream The stream to read from.
     */
    void Load(BinStream &stream) {
        stream.Read(&mPlayer, sizeof(mPlayer));
    }

    unsigned char mPlayer; /*!< The player index. */
};
