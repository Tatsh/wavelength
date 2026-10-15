#pragma once

#include "os/binstream.h"

/**
 * Best result of one player on one song at one skill level, as Campaign stores it.
 *
 * The class is not polymorphic. The name comes from the RTTI of the vectors that store it. The
 * object is 12 bytes.
 */
class SongStats {
public:
    /**
     * Construct a record with every byte cleared.
     *
     * @ghidraAddress NTSC-U/C: 0x00276970
     * @ghidraAddress PAL: 0x00280490
     */
    SongStats();

    /**
     * Raise each statistic to the larger of its own value and the value of another record.
     *
     * Nothing changes unless the other record has a higher score, more full mix bars, a higher
     * share blasted, or a longer best streak. The name is inferred.
     *
     * @param other The other record.
     * @return Whether the record changed.
     * @ghidraAddress NTSC-U/C: 0x00276990
     * @ghidraAddress PAL: 0x002804b0
     */
    bool Improve(const SongStats &other);

    /**
     * Report whether two records are of the same song at the same skill level.
     *
     * @param other The other record.
     * @return Whether the song and the skill level match.
     * @ghidraAddress NTSC-U/C: 0x00276a98
     * @ghidraAddress PAL: 0x002805b8
     */
    bool operator==(const SongStats &other) const;

    const char *mSong;          /*!< The song, a symbol. */
    unsigned short mScore;      /*!< The best score. */
    unsigned char mSkillLevel;  /*!< The skill level. */
    unsigned char mBestStreak;  /*!< The best streak. */
    unsigned char mBlasted;     /*!< The share of the gems blasted, in percent. */
    unsigned char mFullMixBars; /*!< The bars played with the full mix. */
    unsigned char mPercentDone; /*!< The part of the song played, 100 for a finished song. */
};

/**
 * Write a record, with the song as the `id` of its entry.
 *
 * @param stream The stream.
 * @param record The record.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00276ac8
 * @ghidraAddress PAL: 0x002805e8
 */
BinStream &operator<<(BinStream &stream, const SongStats &record);

/**
 * Read a record the writer wrote.
 *
 * The song is the song of any arena, the tutorial included, whose `id` matches. The song does not
 * change when none matches.
 *
 * @param stream The stream.
 * @param record Receives the record.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00276c00
 * @ghidraAddress PAL: 0x002806d8
 */
BinStream &operator>>(BinStream &stream, SongStats &record);
