#pragma once

/**
 * Best result of one player on one song at one skill level, as PlayerProfile stores it.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object is 12 bytes.
 */
class SongRecord {
public:
    /**
     * Construct a record with every byte cleared.
     *
     * @ghidraAddress NTSC-U/C: 0x00276970
     * @ghidraAddress PAL: 0x00280490
     */
    SongRecord();

    const char *mSong;          /*!< The song, a symbol. */
    unsigned short mScore;      /*!< The best score. */
    unsigned char mSkillLevel;  /*!< The skill level. */
    unsigned char mReserved07;  // +0x07, not yet recovered.
    unsigned char mReserved08;  // +0x08, not yet recovered.
    unsigned char mReserved09;  // +0x09, not yet recovered.
    unsigned char mPercentDone; /*!< The part of the song played, 100 for a finished song. */
    unsigned char mReserved0B;  // +0x0b, not yet recovered.
};
