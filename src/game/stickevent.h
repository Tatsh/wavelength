#pragma once

#include "os/binstream.h"

/**
 * Input event of an analog stick position.
 *
 * The RTTI includes the template with the arguments 2 and 6. The argument is the identifier of
 * the serializable input command that records the event, and it makes each instance a distinct
 * type. Every member is inline.
 *
 * @tparam kId The command identifier.
 */
template <int kId>
class StickEvent {
public:
    /** Construct an event with unset members. */
    StickEvent() = default;

    /**
     * Construct an event.
     *
     * @param nPlayer The player index.
     * @param fX The horizontal position.
     * @param fY The vertical position.
     */
    StickEvent(unsigned char nPlayer, float fX, float fY) : mPlayer(nPlayer), mX(fX), mY(fY) {
    }

    /**
     * Construct the event from a recording.
     *
     * @param stream The stream to read from.
     */
    explicit StickEvent(BinStream &stream) {
        Load(stream);
    }

    /**
     * Record the player in one byte and each position in four bytes of the console's byte order.
     *
     * @param stream The stream to write to.
     */
    void Save(BinStream &stream) const {
        const unsigned char nPlayer = mPlayer;
        stream.Write(&nPlayer, sizeof(nPlayer));
        float fValue = mX;
        stream.WriteEndian(&fValue, sizeof(fValue));
        fValue = mY;
        stream.WriteEndian(&fValue, sizeof(fValue));
    }

    /**
     * Read the event Save() records.
     *
     * @param stream The stream to read from.
     */
    void Load(BinStream &stream) {
        stream.Read(&mPlayer, sizeof(mPlayer));
        stream.ReadEndian(&mX, sizeof(mX));
        stream.ReadEndian(&mY, sizeof(mY));
    }

    unsigned char mPlayer; /*!< The player index. */
    float mX;              /*!< The horizontal position. */
    float mY;              /*!< The vertical position. */
};
