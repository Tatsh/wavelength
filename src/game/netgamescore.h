#pragma once

#include "os/binstream.h"
#include "os/prnstream.h"

/**
 * The final score of one player of an online session.
 *
 * The RTTI includes the class name. The structure is not polymorphic.
 */
struct NetGameScore {
    int mNetOrder; /*!< The player's position in the order of the session. */
    int mScore;    /*!< The player's score. */
};

/**
 * Write a score as `order:score`.
 *
 * @param stream The stream.
 * @param score The score.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002686b0
 * @ghidraAddress PAL: 0x00272308
 */
PrnStream &operator<<(PrnStream &stream, const NetGameScore &score);

/**
 * Write a score to the packets of a session.
 *
 * @param stream The stream.
 * @param score The score.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00268708
 * @ghidraAddress PAL: 0x00272360
 */
BinStream &operator<<(BinStream &stream, const NetGameScore &score);

/**
 * Read a score operator<<() wrote.
 *
 * @param stream The stream.
 * @param score Receives the score.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00268768
 * @ghidraAddress PAL: 0x002723c0
 */
BinStream &operator>>(BinStream &stream, NetGameScore &score);
