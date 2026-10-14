#pragma once

#include "os/binstream.h"

/**
 * The results of the online games of a player that the next rating weighs.
 *
 * The record has no RTTI, and the name is inferred. LobbyPlayer stores one at `+0x78`.
 */
struct RatingRecord {
    float mWins;    /*!< The games won, with a draw as half a game. */
    int mGames;     /*!< The games counted. */
    int mPoints;    /*!< The sum of the opponents' ratings, each weighed by its games. */
    int mOpponents; /*!< The sum of the opponents' weights. */
};

/**
 * Write a record, with the wins in half games.
 *
 * The wins, the games, and the opponents take two bytes, and the points four.
 *
 * @param stream The stream.
 * @param record The record.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00267f58
 * @ghidraAddress PAL: 0x00271c30
 */
BinStream &operator<<(BinStream &stream, const RatingRecord &record);

/**
 * Read a record operator<<() wrote.
 *
 * @param stream The stream.
 * @param record Receives the record.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00268028
 * @ghidraAddress PAL: 0x00271d00
 */
BinStream &operator>>(BinStream &stream, RatingRecord &record);
